/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include <openssl/evp.h>
#include "eventlogMain.h"
#include "eventlogargs.h"


#include <CDataSourceFactory.h>
#include <CDataSource.h>
#include <CRingDataSource.h>

#include <V12/CRingItem.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingStateChangeItem.h>
#include <V12/DataFormat.h>
#include <V12/CDataFormatItem.h>
#include <V12/format_cast.h>

#include <RingIOV12.h>
#include <ByteBuffer.h>


#include <CAllButPredicate.h>
#include <CPortManager.h>
#include <CStateClientApi.h>
#include <io.h>

#include <iostream>
#include <array>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <cstdlib>

#include <system_error>

using std::string;
using std::cerr;
using std::endl;
using std::cout;

using namespace DAQ;
using namespace DAQ::V12;

// constant defintitions.

static const uint64_t K(1024);
static const uint64_t M(K*K);
static const uint64_t G(K*M);

static const int RING_TIMEOUT(5);	// seconds in timeout for end of run segments...need no data in that time.

static const int SpaceCheckInterval(1*M);  

///////////////////////////////////////////////////////////////////////////////////
// Local classes:
//

class noData :  public CRingBuffer::CRingBufferPredicate
 {
 public:
   virtual bool operator()(CRingBuffer& ring) {
     return (ring.availableData() == 0);
   }
 };

 ///////////////////////////////////////////////////////////////////////////////////
 //
 // Constructor and destructor  are basically no-ops:

 EventLogMain::EventLogMain() :
   m_pRing(0),
   m_eventDirectory(string(".")),
   m_segmentSize((static_cast<uint64_t>(1.9*G))),
   m_exitOnEndRun(false),
   m_nSourceCount(1),
   m_fRunNumberOverride(false),
   m_pChecksumContext(0),
   m_nBeginsSeen(0),
   m_fChangeRunOk(false),
   m_prefix("run"),
   m_haveWarned(false),
   m_haveSevere(false),
   m_pLogSocket(0),
   m_pLogger(0),
   m_pStateApi(0)
 {
 }

 EventLogMain::~EventLogMain()
 {
    delete m_pLogger;
    delete m_pLogSocket;
    delete m_pStateApi;
 }
 //////////////////////////////////////////////////////////////////////////////////
 //
 // Object member functions:
 //


 /*!
    Entry point is pretty simple, parse the arguments, 
    Record the data
 */
 int
 EventLogMain::operator()(int argc, char**argv)
 {
  parseArguments(argc, argv);
  log("EventlogMain::operator()", CStatusDefinitions::SeverityLevels::DEBUG);
  // If run under the state manager instantiate the client api to it.

  const char* programName = std::getenv("PROGRAM");
  const char* subUri      = std::getenv("SUB_URI");
  const char* reqUri      = std::getenv("REQ_URI");
  
  // all must be defined to instantiate the client.
  
  if (
    (programName != nullptr) && (subUri != nullptr) && (reqUri != nullptr))
  {
    log("Creating state API", CStatusDefinitions::SeverityLevels::DEBUG);
    m_pStateApi = new CStateClientApi(reqUri, subUri, programName);
   
   // at startup, we should have gotten a 'Readying' request.. we'll pull that
   // and echo it.  If we don't have such a request then we'll set our state
   // and the entire state to NotReady and suicide.
   
   std::string message;
    if(0) {
     log("Waiting for readying (1 second).", CStatusDefinitions::SeverityLevels::DEBUG);
     if (!expectStateRequest(message, "Readying", 10000)) {
         message += " initializing waiting for the 'Readying' transition request'";
         stateManagerDie(message.c_str());
     }
     
    
    log("Setting event log state to Readyng", CStatusDefinitions::SeverityLevels::DEBUG);
    m_pStateApi->setState("Readying");
    }
  }
   
   // Initialize the event logger:
   
   log("Event logger starting", CStatusDefinitions::SeverityLevels::INFO);
   
   
   // If the state API is active, we need to wait for the 'Ready' transition
   // and echo that too:
   
  if (m_pStateApi) {
    std::string message;
    std::string newState;
    if (0) {
    
    if (!expectStateRequest(message, "Ready", 10000)) {
      message += " expecting transition to Ready";
      stateManagerDie(message.c_str());
    } else {
      m_pStateApi->setState("Ready");
    }
    }
    m_pStateApi->setState("Ready");    // We've initialized.
    while(!m_pStateApi->waitTransition(newState, -1)) {
      log("Waiting for Ready...", CStatusDefinitions::SeverityLevels::DEBUG);
    }
    if (newState != "Ready") {
      message = "Expecting state transition to Ready got: ";
      message += newState;
      stateManagerDie(message.c_str());
    }
   
  }
   
   // Record data until we're supposed to exit.
   
   log("Event Logger entering recordData()", CStatusDefinitions::SeverityLevels::DEBUG);
   recordData();
   log("Event logger exiting normally", CStatusDefinitions::SeverityLevels::INFO);
   

    
    // Once everybody is readying, we shou
    
    
}

 ///////////////////////////////////////////////////////////////////////////////////
 //
 // Utility functions...well really this is where all the action is.
 //

 /*
 ** Open an event segment.  Event segment filenames are of the form
 **   run-runnumber-segment.evt
 **
 ** runnumber - the run number. in %04d
 ** segment   - The run ssegment in %02d
 **
 ** Note that all files are stored in the directory pointed to by
 ** m_eventDirectory.
 **
 ** Parameters:
 **     runNumber   - The run number.
 **     segment     - The segment number.
 **
 ** Returns:
 **     The file descriptor or exits with an error if the file could not be opened.
 **
 */
 int
 EventLogMain::openEventSegment(uint32_t runNumber, unsigned int segment)
 {
   // Create the filename:

   string fullPath  = m_eventDirectory;

   char nameString[1000];
   sprintf(nameString, "/%s-%04d-%02d.evt", m_prefix.c_str(), runNumber, segment);
   fullPath += nameString;

   int fd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 
		 S_IWUSR | S_IRUSR | S_IRGRP);
   if (fd == -1) {
      std::string msg = "Open failed for event file segment ";
      msg += fullPath;
      msg += " ";
      perror(msg.c_str());
      log(msg.c_str(), errno, CStatusDefinitions::SeverityLevels::SEVERE);
      log("Event logger exiting in error", CStatusDefinitions::SeverityLevels::SEVERE);
      exit(EXIT_FAILURE);
   }
   return fd;

 } 


 /*
 **
 ** Record the data.  We're ready to roll having verified that
 ** we can open an event file and write data into it, the ring is also 
 ** open.
 */
 void
 EventLogMain::recordData()
 {

   // if we are in one shot mode, indicate to whoever started us that we are ready to
   // roll.  That file goes in the event directory so that we don' thave to keep hunting
   // for it like we did in ye olde version of NSCLDAQ:

   if (m_exitOnEndRun) {
     string startedFile = m_eventDirectory;
     startedFile += "/.started";
     int fd = open(startedFile.c_str(), O_WRONLY | O_CREAT,
		   S_IRWXU );
     if (fd == -1) {
       perror("Could not open the .started file");
       log(
	   "Event logger could not open the .started file", errno,
	   CStatusDefinitions::SeverityLevels::SEVERE
	);
       log(
	  "Event Logger exiting in error",
	  CStatusDefinitions::SeverityLevels::SEVERE
	);
	exit(EXIT_FAILURE);
     }

     close(fd);
   }

   // Now we need to hunt for the BEGIN_RUN item...however if there's a run
   // number override we just use that run number unconditionally.

   bool warned = false;
   CRawRingItem rawItem;
   CRawRingItem formatItem;

   // Loop over all runs.

   while(1) {
    // If we are cooperating with the state manager, before the run
    // starts, we should expect to see a "Beginning" and then an "Active"
    // state transition as the readouts get rolling:
    
     if(m_pStateApi) {
       std::string newState;
       log("Waiting for beginning transition" , CStatusDefinitions::SeverityLevels::DEBUG);
       while (!m_pStateApi->waitTransition(newState, -1)) {
         ;
       }
       if (newState != "Beginning") {
         std::string msg = "Expected state transition to 'Beginning' instead got: ";
         msg += newState;
         stateManagerDie(msg.c_str());
       }

       log("Beginning received", CStatusDefinitions::SeverityLevels::DEBUG);
       m_pStateApi->setState("Beginning");

       while(!m_pStateApi->waitTransition(newState, -1)) {}
       if (newState != "Active") {
         std::string msg = "Expected state transition to 'Active' got: ";
         msg += newState;
         log(msg.c_str(), CStatusDefinitions::SeverityLevels::DEBUG);
       }
       // At this point readout programs can start shooting data through rings.

       m_pStateApi->setState("Active");
       log("Active", CStatusDefinitions::SeverityLevels::DEBUG);
     }
       // If necessary, hunt for the begin run.

       if (!m_fRunNumberOverride) {
           while (1) {
               readItem(*m_pRing, rawItem);
               /*
                As of NSCLDAQ-11 it is possible for the item just before a begin run
                to be one or more ring format items.
               */

               if (rawItem.type() == RING_FORMAT) {
                   formatItem = rawItem;
               } else if (rawItem.type() == BEGIN_RUN) {
                   m_nBeginsSeen = 1;
                   break;
               }

               if (!warned && formatItem.type() == UNDEFINED) {
                   warned = true;
                   cerr << "**Warning - first item received was not a begin run. Skipping until we get one\n";
               }
           }

           // Now we have the begin run item; and potentially the ring format item
           // too. Alternatively we have been told the run number on the command line.

           recordRun(rawItem, formatItem);

       } else {
           //
           // Run number is overidden we don't need a valid state change item.  Could,
           // for example, be a non NSCLDAQ system or a system without
           // State change items.
           //
           recordRun(rawItem, formatItem);
       }
       // Return/exit after making our .exited file if this is a one-shot.

       if (m_exitOnEndRun) {
           string exitedFile = m_eventDirectory;
           exitedFile       += "/.exited";
           int fd = open(exitedFile.c_str(), O_WRONLY | O_CREAT,
                         S_IRWXU);
           if (fd == -1) {
               perror("Could not open .exited file");
               exit(EXIT_FAILURE);
               return;
           }
	        log(
              "Event logger could not open .exited file ", errno,
              CStatusDefinitions::SeverityLevels::SEVERE
             );
          log(
              "Event logger exiting with error",
              CStatusDefinitions::SeverityLevels::SEVERE
             );
           close(fd);
           exit(EXIT_FAILURE);
           return;
       }


   }



 }

 /*
 ** Record a run to disk.  This must
 ** - open the initial event file segment
 ** - Write items to the segment keeping track of the segment size,
 **   opening new segments as needed until:
 ** - The end run item is gotten at which point the run ends.
 **
 ** @param item - The state change item.
 ** @param pFormatitem - possibly null pointer, if not null this points to the
 **                      ring format item that just precedes the begin run.
 **
 */
 void
 EventLogMain::recordRun(const CRawRingItem& rawStateItem, const CRawRingItem& formatItem)
 {
   std::string newState;
   std::string stateMessage;
   unsigned int segment        = 0;
   uint32_t     runNumber;
   uint64_t     bytesInSegment = 0;
   int          fd;
   unsigned     endsRemaining  = m_nSourceCount;

   CRawRingItem rawItem;
   uint16_t     itemType;
   m_lastCheckedSize = 0;    // Last checked free space.
   bool recording(true);
   bool processStateTransitions(true);

   // If using the state manager we need to use the global recording flag
   // to determine if we are recording data.

   if (m_pStateApi) {
     recording = m_pStateApi->recording();
   }

   // Figure out what file to open and how to set the pItem:

   if (m_fRunNumberOverride) {
     if (recording) {
       runNumber  = m_nOverrideRunNumber;
       fd         = openEventSegment(runNumber, segment);
     }
     readItem(*m_pRing, rawItem);
   } else {
     if (recording) {
       auto stateItem = format_cast<CRingStateChangeItem>(rawStateItem);

       runNumber  = stateItem.getRunNumber();
       fd         = openEventSegment(runNumber, segment);
     }

     rawItem = rawStateItem;
   }

   // if there is a format item (i.e. type != UNDEFINED), write it out to file
   // Note there won't be if the run number has been overridden
   if (formatItem.type() == RING_FORMAT && recording) {
     bytesInSegment += formatItem.size();
     writeItem(fd, formatItem);
   }

   while(1) {

     /* If the state manager is present and we still need to process
      * state transitions check for one and:
      * Whith the following exceptions just echo the transition:
      *  - NotReady - do an stateManagerExit as the system is doing an emergency
      *               shutdown.
      *  - Ending - Stop processing state transitions;  Once the run file is
      *             closed, we'll expect to see a transition to Ready to complete
      *             our part of ending the run.
      */
     if (m_pStateApi && processStateTransitions) {
       std::string nextState;
       if (m_pStateApi->waitTransition(newState, 0)) {
         if (newState == "NotReady") {
           stateManagerDie("Being asked to exit by transition to NotReady while recording");
         } else if (newState == "Ending") {
           m_pStateApi->setState("Ending");
           processStateTransitions = false;
         } else {
           m_pStateApi->setState(newState);    // All else just echo.
         }
       }
     }



     if (rawItem.type() != UNDEFINED) {

       size_t size    = rawItem.size();
       itemType       = rawItem.type();

       // If necessary, close this segment and open a new one:

       if ( (bytesInSegment + size) > m_segmentSize) {
         close(fd);
         segment++;
         bytesInSegment = 0;

         fd = openEventSegment(runNumber, segment);
         // put out a format item:

         if (formatItem.type() != UNDEFINED) {

           if (recording) {
             writeItem(fd, formatItem);
             bytesInSegment += formatItem.size();
           }
         }
       }

       if (recording) {
         writeItem(fd, rawItem);
         bytesInSegment  += size;
       }



       // Check free disk space and log Warning, SEVERE or Info if so.

       if ((bytesInSegment - m_lastCheckedSize) >= SpaceCheckInterval) {
         m_lastCheckedSize = bytesInSegment;
         std::stringstream logMessage;
         logMessage << "Segment size: ";
         logMessage << bytesInSegment/(1024*1024);
         logMessage << " Mbytes";
         log(logMessage.str().c_str(), CStatusDefinitions::SeverityLevels::DEBUG);
         try {
           double pctFree = io::freeSpacePercent(fd);
           if (shouldLogWarning(pctFree)) {
             log(
                 "Disk space is getting a bit low percent left: ", pctFree,
                 CStatusDefinitions::SeverityLevels::WARNING
                );
             m_haveWarned = true;
           }
           if (shouldLogSevere(pctFree)) {
             log(
                 "Disk space is getting very low percent left: ", pctFree,
                 CStatusDefinitions::SeverityLevels::SEVERE
                );
             m_haveSevere = true;
           }
           if (shouldLogSevereClear(pctFree)) {
             log(
                 "Disk space is somewhat better but still a bit percent left: ", pctFree,
                 CStatusDefinitions::SeverityLevels::INFO
                );
             m_haveSevere = false;
           }
           if (shouldLogWarnClear(pctFree)) {
             log(
                 "Disk space is ok now percent left:", pctFree,
                 CStatusDefinitions::SeverityLevels::INFO
                );
             m_haveWarned = false;
           }
         }
         catch (int errno) {
           log("Unable to get disk free space", CStatusDefinitions::SeverityLevels::WARNING);
         }
       }
     }

     if(itemType == END_RUN) {
       log("Got an end run item", CStatusDefinitions::SeverityLevels::DEBUG);
       endsRemaining--;
       // If we're participating in the state manager and our state is not
       // already 'Ready', we need to participate in the End of run transition.


       if (endsRemaining == 0) {
         log("All end runs received", CStatusDefinitions::SeverityLevels::DEBUG);
         break;
       }
     }
     if (itemType == ABNORMAL_ENDRUN) {
       endsRemaining = 0;             // In case we're not --one-shot
       break;                         // unconditionally ends the run.
     }

     // If we've seen an end of run, need to support timing out
     // if we dont see them all.

     if ((endsRemaining != m_nSourceCount) && dataTimeout()) {
       cerr << "Timed out waiting for end of runs. Need " << endsRemaining 
         << " out of " << m_nSourceCount << " sources still\n";
       cerr << "Closing the run\n";

       break;
     }

     // TODO:  In order to catch NotReady state transition in the middle of
     //        a run it's probably going to be necessary to get ring items
     //        with a timeout.

     readItem(*m_pRing, rawItem, CTimeout(std::chrono::seconds(1));
         if(rawItem.type() != UNDEFINED && isBadItem(*pItem, runNumber)) {
         std::cerr << "Eventlog: Data indicates probably the run ended in error exiting\n";
         log(
             "Event log exiting - got a bad data item.  run may have ended in error",
             CStatusDefinitions::SeverityLevels::SEVERE
            );
         exit(EXIT_FAILURE);
         }
         }
         log("Exited main recording loop", CStatusDefinitions::SeverityLevels::DEBUG);
         if (recording) {
         writeChecksumFile(runNumber);
         close(fd);
         }
         // If necessary participate in the final transition to "Ready".

         if (m_pStateApi) {
         std::string newState;
         log("Expecting transition to Ready", CStatusDefinitions::SeverityLevels::DEBUG);
         while (!m_pStateApi->waitTransition(newState, -1) ) {
           ;
         }
         if (newState != "Ready") {
           stateMessage  = "Was expecting a state transition to Ready but got: ";
           stateMessage += newState;
           stateManagerDie(stateMessage.c_str());
         }
         log("Setting state to Ready", CStatusDefinitions::SeverityLevels::DEBUG);
         m_pStateApi->setState("Ready");
         }


 }

/*
 ** Parse the command line arguments, stuff them where they need to be
 ** and check them for validity:
 ** - The ring must exist and be open-able.
 ** - The Event segment size, if supplied must decode properly.
 ** - The Event directory must be writable.
 **
 ** Parameters:
 **   argc  - Count of command words.
 **   argv  - Array of pointers to the command words.
 */
 void
 EventLogMain::parseArguments(int argc, char** argv)
 {
     gengetopt_args_info parsed;
     cmdline_parser(argc, argv, &parsed);

     // Figure out where we're getting data from:

     string ringUrl = defaultRingUrl();
     if(parsed.source_given) {
         ringUrl = parsed.source_arg;
     }
     // Figure out the event directory:

     if (parsed.path_given) {
         m_eventDirectory = string(parsed.path_arg);
     }

     if (parsed.oneshot_given) {
         m_exitOnEndRun = true;
     }
     if (parsed.run_given && !parsed.oneshot_given) {
         std::cerr << "--oneshot is required to specify --run\n";
         log(
             "Event log startup failed --oneshot is required to specify --run",
             CStatusDefinitions::SeverityLevels::SEVERE
            );
         exit(EXIT_FAILURE);
     }
     if (parsed.run_given) {
         m_fRunNumberOverride = true;
         m_nOverrideRunNumber = parsed.run_arg;
     }

     // And the segment size:
     if (parsed.segmentsize_given) {
         m_segmentSize = segmentSize(parsed.segmentsize_arg);
     }

     m_nSourceCount = parsed.number_of_sources_arg;

     // Get logging thresholds and service name:

     m_freeWarnThreshold   = parsed.freewarn_arg;
     m_freeSevereThreshold = parsed.freesevere_arg;
     m_appname             = parsed.appname_arg;
     m_logService          = parsed.service_arg;

     // The directory must be writable:
     if (!dirOk(m_eventDirectory)) {
       std::ostringstream nosuchdirmsg;
       nosuchdirmsg << "Event logger exiting: "
         << m_eventDirectory 
         << " must be an existing directory and writable so event files can be created";

       cerr << nosuchdirmsg.str()
         << endl;
       log(nosuchdirmsg.str().c_str(), CStatusDefinitions::SeverityLevels::SEVERE);
       exit(EXIT_FAILURE);
     }

     if (parsed.prefix_given) {
         m_prefix = parsed.prefix_arg;
     }

     // And the ring must open:

     try {
         m_pRing = CDataSourceFactory().makeSource(ringUrl, {}, {});
     }
     catch (...) {
       std::ostringstream msg;
       msg << "Event log exiting: Could not open the data source: " << ringUrl;
       cerr << msg.str() << endl;
       log(msg.str().c_str(), CStatusDefinitions::SeverityLevels::SEVERE);
       exit(EXIT_FAILURE);
     }
     // Checksum flag:

     m_fChecksum = (parsed.checksum_flag != 0);
     m_fChangeRunOk = (parsed.combine_runs_flag != 0);

 }

 /*
 ** Return the default URL for the ring.
 ** this is tcp://localhost/username  where
 ** username is the name of the account running the program.
 */
 string
 EventLogMain::defaultRingUrl() const
 {
     return CRingBuffer::defaultRingUrl();

 }

 /*
 ** Given a segement size string either returns the size of the segment in bytes
 ** or exits with an error message.
 **
 ** The form of the string can be one of:
 **    number   - The number of bytes.
 **    numberK  -  The number of kilobytes.
 **    numberM  - The number of megabytes.
 **    numberG  - The number o gigabytes.
 */
 uint64_t
 EventLogMain::segmentSize(const char* pValue)
 {
   char* end;
   uint64_t size = strtoull(pValue, &end, 0);

   // The remaning string must be 0 or 1 chars long:

   if (strlen(end) < 2) {

     // If there's a multiplier:

     if(strlen(end) == 1) {
       if    (*end == 'g') {
         size *= G;
       }
       else if (*end == 'm') {
         size *= M;
       }
       else if (*end == 'k') {
         size *= K;
       }
       else {
         std::ostringstream msg;
         msg << "Event logger startup failure: " << "Segment size multipliers must be one of g, m, or k";
         cerr << msg.str() << endl;
         log(msg.str().c_str(), CStatusDefinitions::SeverityLevels::SEVERE); 
         exit(EXIT_FAILURE);
       }

     }
     // Size must not be zero:

     if (size == (uint64_t)0) {
       std::ostringstream msg;
       msg << "Event logger startup failure: " << "Segment size must not be zero!!";
       cerr << msg.str() << endl;
       log(msg.str().c_str(), CStatusDefinitions::SeverityLevels::SEVERE);
       exit(EXIT_FAILURE);
     }
     return size;
   }
   // Some conversion problem:

   std::string msg1 = "Event logger startup Failure: ";
   msg1 +=  "Segment sizes must be an integer, or an integer followed by g, m, or k";
   cerr << msg1 << std::endl;
   log(msg1.c_str(), CStatusDefinitions::SeverityLevels::SEVERE);
   exit(EXIT_FAILURE);
 }

 /*
 ** Determine if a path is suitable for use as an event directory.
 ** - The path must be to a directory.
 ** - The directory must be writable by us.
 **
 ** Parameters:
 **  dirname - name of the proposed directory.
 ** Returns:
 **  true    - Path ok.
 **  false   - Path is bad.
 */
 bool
 EventLogMain::dirOk(string dirname) const
 {
     struct stat statinfo;
     int  s = stat(dirname.c_str(), &statinfo);
     if (s) return false;		// If we can't even stat it that's bad.

     mode_t mode = statinfo.st_mode;
     if (!S_ISDIR(mode)) return false; // Must be a directory.

     return !access(dirname.c_str(), W_OK | X_OK);
 }
 /**
  * dataTimeout
  *
  *  Determine if there's a data timeout.  A data timeout occurs when no data comes
  * from the ring for RING_TIMEOUT seconds.  This is used to detect missing
  * end segments in the ring (e.g. run ending because a source died).
  */
 bool
 EventLogMain::dataTimeout()
 {
     noData predicate;

     auto pRing = dynamic_cast<CRingDataSource*>(m_pRing);
     if (pRing == nullptr) {
         throw std::runtime_error("Only ring data sources are supported in eventlog currently.");
     }
     pRing->getRing().blockWhile(predicate, RING_TIMEOUT);
     return (pRing->getRing().availableData() == 0);
 }
 /**
 * writeItem
 *   Write a ring item.
 *
 *   @param fd - File descriptor open on the output file.
 *   @param item - Reference to the ring item.
 *
 * @throw  uses io::writeData which throws errs.
 *         The errors are caught described and we exit :-(
 */
 void
 EventLogMain::writeItem(int fd, const CRawRingItem& item)
 {
     try {
         uint32_t nBytes= item.size();
         std::array<char,20> header;

         serializeHeader(item, header.begin());
         auto& body = item.getBody();

         // If necessary create the checksum context
         // If checksumming add the ring item to the sum.

         if (m_fChecksum) {
             if (!m_pChecksumContext) {
                 m_pChecksumContext = EVP_MD_CTX_create();
                 if (!m_pChecksumContext) throw errno;
                 if(EVP_DigestInit_ex(
                             reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext), EVP_sha512(), NULL) != 1) {
                     EVP_MD_CTX_destroy(reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext));
                     m_pChecksumContext = 0;
                     throw std::string("Unable to initialize the checksum digest");
                 }

             }
             // hash the header
             EVP_DigestUpdate(
                         reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext), header.data(), 20);

             // hash the body
             EVP_DigestUpdate(
                         reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext), body.data(), body.size());
         }

         io::writeData(fd, header.data(), header.size());
         io::writeData(fd, body.data(), body.size());
     }
     catch(int err) {
       std::ostringstream msg;
       msg << "Event logger exiting in error: ";
       if(err) {
             msg << "Unable to output a ringbuffer item : "  << strerror(err) << endl;
         }  else {
             msg << "Output file closed out from underneath us\n";
         }
        cerr << msg << std::endl;
        log(msg.str().c_str(), CStatusDefinitions::SeverityLevels::SEVERE);
        exit(EXIT_FAILURE);
     }
     catch (std::string e) {
      std::ostringstream msg;
      msg << "Event logger exiting in error: " << e;
      std::cerr << msg.str() << std::endl;
      log(msg.str().c_str(), CStatusDefinitions::SeverityLevels::SEVERE);
      exit(EXIT_FAILURE);
     }
 }

 /**
 * shaFile
 *    Compute the filename for the checksum for a run.
 *
 * @param run - Run number
 *
 * @return std::string - the filename.
 */
 std::string
 EventLogMain::shaFile(int run) const
 {
     char runNumber[100];
     sprintf(runNumber, "%04d", run);

     std::string fileName = m_eventDirectory;
     fileName+= ("/" + m_prefix + "-");
     fileName+= runNumber;
     fileName += ".sha512";

     return fileName;
 }
 /**
 * isBadItem
 *     This method is called to determine if we've gotten a ring item that
 *      might indicate we need to exit in --one-shot mode:
 *      -  If --combine-runs is set, or --one-shot not set, return false.
 *      -  If the run number changed from the one we are recording,
 *         true.
 *      -  If we had more begin runs than m_nSourceCount, true
 *      -  None of these, return false.
 *
 * @param item      - Reference to the ring item to check.
 * @param runNumber - the current run number.
 *
 * @return bool
 * @retval true  - there's something fishy about this -- probably we should exit.
 * @retval false - as near as we can tell everything is ok.
 */
 bool
 EventLogMain::isBadItem(const CRawRingItem& item, int runNumber)
 {
     // For some states of program options we just don't care about the
     // data

     if (m_fChangeRunOk || (!m_exitOnEndRun)) {
         return false;
     }
     // For the rest we only care about state changes -- begins in fact

     if (item.type() == BEGIN_RUN) {
         m_nBeginsSeen++;
         if (m_nBeginsSeen > m_nSourceCount) {
             return true;
         }
         CRingStateChangeItem begin(item);
         if (begin.getRunNumber() != runNumber) {
             return true;
         }
     }
     return false;

 }
/**
 * shouldLogWarning
 *    Warnings should be logged if the free space pct is lower than the threshold
 *    and no warning has yet been given:
 *
 *  @param[in] pct  - Percent free space.
 *  @return bool
 */
bool
EventLogMain::shouldLogWarning(double pct){
  return (pct < m_freeWarnThreshold) && (!m_haveWarned);
}
/**
 * shouldLogSEVERE
 *    SEVERE log messages should be sent if the free space is lower than the
 *    threshold and no message has been issued:
 *
 *  @param[in] pct  - Percent free space.
 *  @return bool
 */
bool
EventLogMain::shouldLogSevere(double pct)
{
  return (pct < m_freeSevereThreshold) && (!m_haveSevere);
}
/**
 * shouldLogSEVEREClear
 *    If the free space goes back above the threshold for SEVERE warnings and
 *    we have not already done we need to send a log message to that effect:
 *
 *  @param pct  - Percentage of free disk space.
 *  @return bool
 */
bool
EventLogMain::shouldLogSevereClear(double pct)
{
  return (pct > m_freeSevereThreshold) && m_haveSevere;
}
/**
 * shouldLogWarnClear
 *    If the free space goes back above the threshold for a warning,
 *    and we have not already done so, we need to send a log emssage to that
 *    effect.
 *
 *  @param pct
 *  @return bool
 */
bool
EventLogMain::shouldLogWarnClear(double pct)
{
  return (pct > m_freeWarnThreshold) && m_haveWarned;
}
/**
 * getAggregatorURI
 *    Given that the service name for the aggregator is in m_logService,
 *    locate the port on which the service is listening.  If it does not exist,
 *    we throw an exception.
 *
 *  @return std::string
 */
std::string
EventLogMain::getAggregatorURI()
{
  CPortManager mgr;
  std::vector<CPortManager::portInfo> services = mgr.getPortUsage();
  
  std::string result = "tcp://localhost:";
  for (size_t i = 0; i < services.size(); i++) {
    if (services[i].s_Application == m_logService) {
      std::ostringstream portString;
      portString << services[i].s_Port;
      result += portString.str();
      return result;
    }
  }
  throw std::runtime_error("Unable to find the status aggreagion port");
}
/**
 * getLogger
 *    This is invoked to get the logger object.  The logger object
 *    is instantiated on demand rather than initially.   Once instantiated it
 *    is retained in m_pLogger.
 *    - Instantiating a logger is a matter using the port mnager to locate our
 *      local aggregator.
 *    - Creating a push socket that attaches to the URI tcp://localhost:aggregator-port
 *    - Using that and our application name to construct a logger object.
 *    - If that log object is successfully constructed, return it and save it in
 *    - m_pLogger.
 */
CStatusDefinitions::LogMessage*
EventLogMain::getLogger()
{
    CStatusDefinitions::LogMessage* pResult = m_pLogger;
    
    // If needed construct one.
    
    if (!pResult) {
      // In case of error we're just going to return null.
      try {
	std::string uri = getAggregatorURI();
	m_pLogSocket    = new zmq::socket_t(
	  CStatusDefinitions::ZmqContext::getInstance(), ZMQ_PUSH
	);
	m_pLogSocket->connect(uri.c_str());
	pResult = new CStatusDefinitions::LogMessage(*m_pLogSocket, m_appname);
	m_pLogger = pResult;
      }
      catch (...) {
	delete m_pLogger;             // In case any of these were made
	delete m_pLogSocket;
	m_pLogger = nullptr;
	m_pLogSocket = nullptr;
      }
    }
    
    return pResult;
}
/**
 * log
 *    Creates and sends a generic log message.
 *
 *  @param[in] message - message to send.
 *  @param[in] severity - message severity level.
 */
void
EventLogMain::log(const char* message, int severity)
{
#ifndef LOG_DEBUG
  if (severity != CStatusDefinitions::SeverityLevels::DEBUG)
#endif
  {
    CStatusDefinitions::LogMessage* pLogger = getLogger();
    if (pLogger) {
      pLogger->Log(severity, message);
    }
  }
}
/**
 * log
 *    Creates a disk low log message and sends it off.
 *
 *   @param baseMessage - the first part of the message.
 *   @param free        - Percentage of free space.
 *   @param severity    - Severity level for the log message.
 */
void
EventLogMain::log(const char* baseMessage, double free, int severity)
{


  std::ostringstream message;
  message << baseMessage << free;
  log(message.str().c_str(), severity);

}
/**
 * log
 *     Log a message text from an errno
 *
 *   @param[in] message -the base message.
 *   @param[in] errno   - System error number involved.
 *   @param[in] severity - Severity of the log message.
 */
void
EventLogMain::log(const char* message, int errno, int severity)
{
  std::ostringstream msg;
  msg << message << strerror(errno);
  
  log(msg.str().c_str(), severity);
}


/**
 * notReadyClose
 *    This is called on a transition to not ready when an event file is open.
 *    -  The file is closd and
 *    -  The associated checksum file is written.
 *    -  A log message is created indicating a premature close.
 *  
 * @param[in] fd - file descriptor of the open event file.
 * @param[in] run - Number of the run.
 */
void
EventLogMain::notReadyClose(int fd, int run)
{
  close(fd);
  writeChecksumFile(run);
  
  std::ostringstream msg;
  msg << "Eventlog: Premature close of event file for run " << run;
  log(msg.str().c_str(), CStatusDefinitions::SeverityLevels::SEVERE);
}
/**
 * writeChecksumFile
 *    Common code to write a checksum file for the just closed run file.l
 *
 * @param[in] runNumber - number of the run just closed - used to generate the
 *                    filename.
 */
void
EventLogMain::writeChecksumFile(int runNumber)
{
   //
   //  If checksumming, finalize the checksum and write out the checksum file as well.
   //  by  now m_pChecksumContext is only set if m_fChecksum was true when the run
   //  files were opened.
   //
   if (m_pChecksumContext) {
     EVP_MD_CTX* pCtx = reinterpret_cast<EVP_MD_CTX*>(m_pChecksumContext);
     unsigned char* pDigest = reinterpret_cast<unsigned char*>(OPENSSL_malloc(EVP_MD_size(EVP_sha512())));
     unsigned int   len;
       
     // Not quite sure what to do if pDigest failed to malloc...for now
     // silently ignore...

     if (pDigest) {
       EVP_DigestFinal_ex(pCtx, pDigest, &len);
       std::string digestFilename = shaFile(runNumber);
       FILE* shafp = fopen(digestFilename.c_str(), "w");

     
       // Again not quite sure what to do if the open failed.
       if (shafp) {
	 unsigned char* p = pDigest;
	 for (int i =0; i < len;i++) {
	   fprintf(shafp, "%02x", *p++);
	 }
	 fprintf(shafp, "\n");
	 fclose(shafp);
       }
       // Release the digest storage and the context.
       OPENSSL_free(pDigest);

     }
     EVP_MD_CTX_destroy(pCtx);
     m_pChecksumContext = 0;
       
   }  
}
/**
 * expectStateRequest
 *    Called when we expect a specific state transition request.
 *
 * @param[out] msg       - If an error occurs, this message is filled in.
 * @param[in] stateName - Expected state name in transition.
 * @param[in] timeout   - Maximum ms to wait for the transition:
 *                    - -1 no timeout.
 *                    -  0 nonblocking poll.
 * @return bool  - True if the expected transition occured, false otherwise.
 *                 If false; msg is overwritten with an error description.
 */
bool
EventLogMain::expectStateRequest(std::string& msg, const char* stateName, int timeout)
{
  // We assume the api is instantiated.
  
  std::string requestedState;
  if (!m_pStateApi->waitTransition(requestedState, timeout)) {
    // Timed out:
    
    msg = "Wait for transition request to state: ";
    msg += stateName;
    msg += " timed out without a transition request.";
    return false;
  }
  // We have a transition.  Is it the one we wanted?
  
  if (requestedState != stateName) {
    msg = "Expected a state transition to ";
    msg += stateName;
    msg + " but got one to ";
    msg += requestedState;
    return false;
  }
  // Yes it's the one we wanted.
  
  return true;
}
/**
 * stateManagerDie
 *   Called when the state manager is active and we need to die.
 *   - Set our state to NotReady
 *   - Request a global state transition to NotReady.
 *   - Make a log message with severity SEVERE indicating we're about to
 *     exit
 *   - Actually exit.
 *
 *  @param[in] msg - message to log on exit.
 */
void
EventLogMain::stateManagerDie(const char* msg)
{
  
  // these try's are because maybe both states are already not ready.
  
  try {
    m_pStateApi->setState("NotReady");       // We're Failing
  } catch (...) {}
  try {
    m_pStateApi->setGlobalState("NotReady"); // Force the system to die.
  } catch(...) {}
  log(msg, CStatusDefinitions::SeverityLevels::SEVERE);
  
  exit(EXIT_FAILURE);
}
>>>>>>> origin/nscldaq-12.0-dev
