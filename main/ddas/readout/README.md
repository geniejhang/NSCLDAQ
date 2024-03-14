# For updating ASC (3/14/24)

While the DDAS Readout has been simplified greatly, it's still a complex beast
and worthy of some documentation.  The goals of the rewrite were as follows:

-   Make the logic clearer.
-   Eliminate data copying for bulk data.
-   Eliminate, where possible, dynamic memory management.

The first of these goals promotes maintainability while the last promotes
performance, as profiling of other NSCLDAQ code (specifically eventlog)
suggested that performance can be drastically improved by minimizing those
actions.


Logic clarification was done by dividing the actual acquisition code into three
classes:

ModuleReader - Responsible for getting data from a digitizer and parsing it
               into 'hits'
CHitManager  - Responsible for maintaining time ordered hits, indicating
               when a hit can be emitted and providing that hit.

Zero-copy and reduction of dynamic memory allocation were improved by
the following classes:

ReferenceCountedBuffer - is storage that can keep track of the references
                         to it by external objects.
BufferArena            - Is a class that supports re-use of
                         ReferenceCountedBufers.
ZeroCopyHit            - Is a DDAS Hit whose data are located in a
                         ReferenceCountedBuffer that came from a BufferArena.

Finally note that ZeroCopyHit is derived from RawHit which understands the
structure of a  hit but can have either locally allocated or client-provided
storage.

Finally CExperiment, the caller of the CMyEventSegment instance has been added
to allow the read code to indicate it has more events to provide prior to
entering the trigger loop again.

Let's take a high level look at how CMyEventSegment::read operates and then
drill down into how the pieces it uses function:

Read can be called for two reasons:
1.  It asked to be called because, after emitting a hit, it has more hits to
    give.
2.  CMyTrigger indicated that at least one module had data in the fifo that
    exceeded the FiFo threshold.

The first case has priority.  We want to emit as many events as possible before
reading more data;

The event segment has a CHitManager (m_sorter).
*  It asks the sorter if it has any hits that can be emitted.
*  If there are emittable hits, it calls its own emitHit method to emit the
   least recently acquired hit to the event buffer.
*  If there are still hits that can be emitted, it tells the experiment that it
   has more events it can without awaiting a new trigger.
*  Regardless, if a hit was emitted, its work is done and it returns the size
   of that hit.

If the hit manager says there were no hits to emit, we must have been called in
response to a trigger by CMyEventTrigger.  We reset the trigger (a hold over
from prior code).

The trigger maintains an array of the number of words it saw in each module
FIFO.  This makes it unecessary to ask each module again how many words it has
(there's a sanity check that can do that however by defining the preprocessor
symbol SANITY_CHECKING).

Each module with data (non zero FIFO words) is read by its module reader
(m_readers array).  The hits are then added to the hit manager which maintains
the time ordered hist read so far.  Note that each hit is a pair consisting of
a pointer to the module it came from and a pointer to  zero copy hit.  This
allows this stuff to be passed around without bulkdata copy and for the non
malloc/free storage management to be done at a module level for both the hit
objects and the buffers they came from.

The hits are handed off to the hit manager which sorts them into the set of
hits already accumulated.  If this results in emittable hits, logic identical
to the code at entry is invoked (emit the hit and ask to be called again if
there are still more hits).

Finally if the hit manager still says there are no hits to emit, we invoke the
base class reject() method which results in the event not producing a ring item.

A bit about the data structures used.  The hits are accumluated into a
std::vector<std::deque<DDASReadout::ModuleReader::HitInfo>>

Each element of the vector is a deque of hits that were read from one module.
Why not put them in a single deque?  Keeping the data from each module
separated makes the sort faster.  Good sort algorithms (e.g. quicksort) run on
order O(n*log2(n)) where log2 is the base to logarithm.  Suppose we have 5 modules
each with n hits.  Sorting them all at once gives performance O((5n)log2(5n)) =
O(5n(log2(n) + log2(5)).
).
Sorting them individually gives performance O(5nlog2(n)), where that extra term
is no longer present.  Since we do a _lot_ of sorting and merging it's worth
it.

We'll say more about the sorting of hits when we describe the hit manager.

Another point:  There's some code that might appear to be a bit strange:

	    std::deque<DDASReadout::ModuleReader::HitInfo> tmp;
            moduleHits.push_back(tmp);
            auto& hits(moduleHits.back());
...
            m_readers[i]->read(hits, words[i]);

where an empty deque is pushed in to the vector, a reference to it gotten, then
read into.  If, instead, the code looked liek:

	    std::deque<DDASReadout::ModuleReader::HitInfo> tmp;
            auto& hits(moduleHits.back());
...
            m_readers[i]->read(tmp, words[i]);
            moduleHits.push_back(tmp);

The contents of the dequeue would have to be copy constructed into the new
vector element.  The code as written allows the data to be read into the
element in place.


The ModuleReader code is realtively straight forward;
-  It has a BufferArena from which it gets buffers into which to read data.
-  It has a hit pool from which, if possible, it gets zerocopy hits and to
   which those hits are returned.

The read will allocate a buffer big enough to hold the data.   Since this comes
from its buffer arena, eventually, the buffer arena will have enough
pre-allocated data to hold all of the in-flight hits from this module.  Once
the data are read, they are parsed into a deque of
std::pair<ModuleReader*,ZeroCopyHit*>  objects (this is called HitInfo object).  The ZeroCopyHits refer to storage
in the buffer, so the hits were created without any data copy operations.
Including a pointer to the module that parsed those hits allows them to be
freed back to the right hit pool and right  buffer arena when the underlying
buffers have no more references.

(the static method ModuleReader::freeHit does this).

The most complicated bits of code are in the hit manager (CHitManager).
Specifically in trying to find efficient ways to maintain the hits sorted by
time.  This code is all triggered by calls to the addHits method.  Here's the
sequence of operations:

*  Each of the deques of hits from digitizer modules is sorted.
*  The sorted hit deques are merged using a minheap algorithm.
*  The sorted hit deque is merged into the existing sorted hit queue.

The individual deques are sorted using the std::sort algorithm which uses an
O(nLog(n)) algorithm.  The deques are merged using the minheap algorithm,which
is O(nLog(m)) where m is the number queues.  This leaves two fully sorted
deques to merge.   Merging two sorted lists is an O(n+m) problem where n and m
are the number of elements in each list.  The time windows typically ensure
that the number of existing hits is much larger than the number of new hits.

This final merge considers three cases:

- There are no existing hits.  The sorted new hits become the existing hits.
- The last existing hit (largest timestamp) has a timestamp before the first
- (smallest timestamp) new hit timstamp.   The new hits are appended to the
 existing hits.
-  The  last existing hit's timestamp is smaller than the first new hit's
   timestamp.  In that case a reduced merge/append approach is taken:
   Hits are pulled off the back of the existing hit list onto the front of a
   temporary deque until the timestamp at the back of the existing hit deque is
   less than the timestamp at the front of the new hits deque, or the existing
   hit deque is empty.  At that point, the  new hit deque and temporary deque
   formed above are merged to the back of the existing hit deque.


The algorithm for the last case takes advantage of the fact that while the hits
from the digitizers are not time ordered, the timestamp overall is
increasing.

These merge operations are implemented in the two overloaded CHitManager::merge
methods.

The end of run is one final complication.  At the end of a run, in general, the hit
manager will have a set of unflushed hits.  The DDASReadout program replaces
the "end" command.  The end replacement stops data taking in the Pixie16
modules, and flushes their FIFOs to file.  It then calls
CMyEventSegment::onEnd.   That method puts the hit manager into flush mode.  In
flush mode, haveHit will return true if there are any hits in the hit queue.
If there are hits, CExperiment::ReadEvent is called which will, in turn, call
CMyEventSegment::read as many times as needed to empty that queue.  On return,
the hit manager is taken out of flush mode and each module reader's most recent
timestamp array is zeroed out in preparation for the next run.


One last comment on container choices: std::deque vs std::list.
Both of these containers have suitable access patterns, however the
implementation of std::deque results in fewer dynamic memory allocations.

An std::list is a doubly linked list of nodes.  Each node has a payload
containing the data at that point in the list.  Each list element, therefore
requires that the node be allocated and each list element removal requires that
node be deleted.

An std::deque is implemented as a set of fixed length arrays and a pointer
array to the beginning and end of each array.  Each array contains several
deque nodes.  Therefore memory allocation/free is substantially less
granually.  Memory for a deque is only freed when the deque is destroyed and
only allocated when pushing a new item on the front or back overflows the array
of nodes at the front or back of the deque.  Therefore, in general, deques are
used rather than lists for the 'lists' of hits.


/*!  

\page readout
\author Ron Fox, Sean Liddick
\date   March 3, 2016 (last modification).

\section introduction Introduction

This page provides step by step instructions for the use of the
DDAS readout program.  Throughout this page we will use $prefix
to mean the top level directory of the DDAS software installation
directory.  Thus if the installation directory is in

> /usr/opt/ddas/1.0

Then

> $prefix/share/spectcl

refers to the directory

> /usr/opt/ddas/1.0/share/spectcl

\section getting_started  Getting Started

To use the Readout program requires that you:

*  Obtain and modify the template crate directory for each
   DDAS PXI Crate you will use.
*  Set up the Readout(s) you need as data sources for the ReadoutGUI.
*  Set up the NSCL Event builder to build event data from the
   Readout program(s) you use.  


\note In order to maintain a consistent event structure, the event builder is used
even when data only comes from a single PXI crate.


\section cratedir Obtaining and modifying the crate files.

For each PXI Crate you will use create a directory for its configuration files.  
By convention, the files are located in your home directory tree under the
<tt>readout</tt> directory.  The name of each crate directory is, again by convention,
<tt>crate_</tt>n  where n is a crate number you have chosen.

Once you have created a crate directory, you can copy a set of sample files from 
<tt>$prefix/share/readout/crate_1</tt> into your crate directory.

This will create the files:

<table>
<tr><th>Filename</th><th>Contents</th></tr>
<tr><td> pxisys.ini </td><td>Crate slot to PCI resource map </td></tr>
<tr><td>cfgPixie16.txt</td><td>Defines what is in the crate</td></tr>
<tr><td>crate_1.set</td><td>Parameter setting files for the DSP algorithms.</td></tr>
<tr><td>modevtlen.txt</td><td>Event length for triggers from each module in cfgPixie16.txt</td></tr>

</table>



Note that Readout for the crate must be run with the current working directory set to 
the crate directory that has the configuration files it needs.

\subsection cfgPixie16 The crate configuration file.


This is a file that just contains the following information:
\verbatim
Crate Id.
Number of modules in the crate
Slot of first module
Slot of second module
       ...
Slot number of last module.
path to set file.

Example:
1
4
2
4
5
6
/user/0400x/readout/crate_1/crate_1.set
\endverbatim

Crate id is one with four moudules in slots 2,4,5,6.
settings file is /user/0400x/readout/crate_1/crate_1.set

It is possible to load a crate with a heterogeneous selection of Pixie-16 digitizers.
You need not fill a crate with only 100 MSPS digitizers for example. You could fill it
with 100 MSPS and 250 MSPS modules.

A full description of the cfgPixie16.txt file can be found at \ref cfgPixie_format.

\subsection setfile The module settings file.

This file is generated by `nscope` and contains he DPP settings 
that will be loaded into each module in the crate.  At this time only 
nscope should be used to modify this binary file.


\subsection    modevtlen The Module Event Length Configuration File - modevtlen.txt

For each module described in cfgPixie16.txt provides the size of the event
expected from the channels in that module. Be careful to compute the correct
length of each event. If this number is incorrect, the data output from the
DDAS Readout will be gibberish because the Readout program uses the modevtlen
to chunk up the data it reads from the FIFO into events. If trace acquisition,
energy summing, and qdc modes are not enabled in the module, then only the
header will be outputted. This header is four 32-bit words long. Assuming that
this is the case for the cfgPixie16.txt file above, your modevtlen.txt would
look like this.

For example:

\verbatim
4
4
4
4
...
\endverbatim

The four modules defined in the cfgPixie16.txt file above all provide event
lengths of 4 longwords (i.e. 32-bit).

This gets a little more interesting if you have trace acquisition, energy
summing, or qdc mode enabled. In those situation you have to compute the number
of 32-bit words that will be outputted by each module. Here is how to compute
it based on whether each feature is enabled (On) or disabled (Off) :

| QDC    | Energy Sums    | Trace Acquisition | Event length (32-bit words) |
|--------|----------------|-------------------|-----------------------------|
| Off    |  Off           |  Off              | 4                           |
| Off    |  On            |  Off              | 8                           |
| On     |  Off           |  Off              | 12                          |
| On     |  On            |  Off              | 16                          |

If trace acquisition is enabled, then the the number of data
words required to store the complete trace must be added to these event lengths. 
In general, the number of longwords required to store a trace is: 

\verbatim
N_longwords = ceil( 0.5 * trace_length * frequency )
\endverbatim

The factor of 1/2 arises because the ADCs in all DDAS supported modules have
less than 16-bit resolution. That means that for every 32-bits, there are two
samples.  Here is an example calculation for a 100 MSPS digitizer. Assume that
trace capture is enabled and that 2.3 us traces are going to be recorded.  That
means that there are 165 longwords to store for this trace. We then add to this
the number of words in the rightmost column of the above table according to the
features that were enabled. Let's assume that energy summing is enabled in
addition to our trace capture. The value we would write in our modevtlen.txt
would be 173. Now if I had the same scenario, but was using a 250 MSPS
digitizer, then the corresponding modevtlen.txt entry would be 296. Note that
we had to round up to account for the odd number of samples in the
trace.

\note At the moment, there is no support for setting up channels within a 
          single module to output different length events. If one channel in
          a module is set to acquire traces of length 1 us, then all other channels in the
          module must do the same. If you do not, then Readout will fail to create
          events with boundaries that are meaningful and the data will become gibberish
          very quickly.

\subsubsection pxisysini The pxisys.ini

The `pxisys.ini' file contains a mapping between crate slot numbers and 
cPCI resoiurces.  The sample file is suitable for use with the Wiener PXI crates
that are used at the NSCL.

Other laboratories may need to modify this file to correctly assign a correspondence
between PCI resources and PXI slots.



\section rdogui Setting up readoutgui event sources.

The NSCL ReadoutGUI can manage several _event sources_.   Use the <tt>Data Source</tt> 
menu to set up data sources.  Data sources are remembered from run to run of
the GUI.

The DDAS Readout programs get run as <tt>SSHPipe</tt> programs.  In the parameter prompter
for SSH Pipe data sources fill in the fields as follows:

<table>
<tr><th>Field</th><th>Value</th></tr>
<tr><td>Host name</td><td> DNS name of the computer connected to the DDAS crate</td></tr>
<tr><td>Readout Program></td><td>$prefix/bin/Readout</td></tr>
<tr><td>Working Directory</td><td>The appropriate crate directory e.g. /user/0400x/readout/crate_1</td></tr>
<tr><td>Command line options</td><td>See below set --ring, and --sourceid options</td></tr>
</table>


Each Readout is required to put data in a unique ring name and to have a unique source id. 
The source id identifies data from that crate within fragment of events put together by 
the event builder.

A reasonable set of conventions are:

*  Ring names like username_DDAS_crateno e.g. e0400x_DDAS_1 for crate number 1 for the user e0400x.
*  Source id that matches the crate number.

\section evbuilder Setting up the NSCL Event builder.


The NSCL Event builder set up is documented in detail the NSCLDAQ online documentation.  
This section will summarize the setup and provide a sample file for a single crate.
The assumption is that we are following the conventions for ring names and ids described
in the previous section.  

Setting up the event builder involves creating or adding to a `ReadoutCallouts.tcl` 
Readout GUI extension file.  

Here is an annotated example:

\verbatim

package require evbcallouts                                       ; #1

proc OnStart {} {
   EVBC::initialize -restart false -glombuild true -glomdt 10     ; #2
}

EVBC::registerRingSource \
	tcp://spdaq21/0400x_DDAS_1 \
	$prefix/lib/libddastimestampextractor.so \
	1 {Crate 1};                                                      #3

\endverbatim

-#  Includes the package that provides the API for the NSCL event builder.
-#   When the system becomes ready, starts the event builder.  The options ensure that
    the event builder is persistent, builds events from fragments and combines fragments
    within 10 timstamp ticks of an initial fragment into events.
-#  Registers and event builder data source for a Readout running in spdaq21 following
    the ring name and source ids for crate 1.

For multi-crate systems the setup simply requires an additional `EVBC::registerRingSource` 
for each crate's readout program.

\section specialfeatures DDAS Readout Special features

\subsection infinity_clock Infinity clock synchronization mode

Synchronizing clocks can result in different card to card or even channel to
channel time offsets.  This is a problem for experiments that require precise
timing.  Those experiments need to synchronize only infrequently and only at
known times.  This form of synchronization was dubbed "infinity clock" because
the clocks are allowed to continue to increment without reset.

The DDAS readout supports the infinity clock in two ways:

*  If the environment variable INFINITY_CLOCK is defined and has the value
   "YES" clock synchronization is only performed automatically
   as DDAS Readout initializes, otherwise clock synchronization/zeroing is
   performed at the beginning of each run.
*  If DDAS Readout is given the command "ddas_sync"  clocks will synchronize
   in response to that command.

\subsection fifo_threshold Controlling the FIFO threshold for triggering readout

The DDAS Readout program is designed to poll the Pixie-16 digitizers until 
the data available in the FIFO is greater than a threshold. The threshold 
default is set in the XIA Pixie-16 API, but users can
configure it themselves by setting the FIFO_THRESHOLD environment variable. When
this variable is defined, its value is used in place of the default threshold.
The value of the variable represents the number of 32-bit words required
to be in the FIFO.

\section rdo_dataformat The Output Data Format

The format out of the DDAS Readout program can be read in more detail at
\ref rdo_dataformat_readout.
*/