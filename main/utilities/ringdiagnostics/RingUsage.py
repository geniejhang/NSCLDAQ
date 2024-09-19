'''
This module provides the ability to track ringbuffer usage.
What we do:

  We start with the ringbuffers in localhost enumerate them 
  and assign commands to the producer (if there is one) and all
  consumers.
  
  Since, due to pipelines we don't know the actual data flow
  within the system and since this could be run by a troubleshooter
  rather than experimenter, we assume all rings are involved.
  
  Once we've assigned the producer/consumer names, we can look for
  consumers of the form:
  
  ringtostdout some-hbostname
  
  or
  
  ring2stdout ... --comment Hoisting to host
  
  The first of these can come from a native NSCLDAQ ringmaster, the
  second from the Rust ringmaster.
  
  We then enumerate the unique set of hosts we are hoisting data to.
  The assumption, normally true, but not necessrily true, is that
  nobody will hoist from a proxy ring.
  
  Having enumerated the hosts data are being hoisted to, 
  we can enumerate the ringbuffers in those remote systems.  
  Ring buffers with names of the form hostname.ringname
  are proxy rings for our local ringname ringbuffer.
  
  Assigning producer/consumer names, requires using the
  ssh package to run the program (not package)
  pidtocommand.phy in the remote systems.
  
  This is possibily the most time consuming part of this process.
  
  It is possible that some non proxy rings may be hoisted in turn
  to a remote system so we keep going, discarding hosts we've already
  seen, until we've mapped the entire system.
  
  The structure we try to create is a list of dicts. Each top level
  dict represents a host and has the keys;
  
  'host' - the hostname.
  'rings' - enumeration of non-proxy rings in that host.  This enumeration
  is, itself, a list of ring definitions.  These are dicts that are
  ringbuffer dicts that come from nscldaqutils.RingMaster.list_rings with the addeed keys:
  'producer_command' with the producer (if any) command string.
  and for each consumer 'consumer_command" which is the command the consumer runs.
  'proxyies' - which are the list of dicts that describe the proxy rings in remote system.
     These are ringbuffer definition dicts with the ring name replaced by 
     ring@host where host is the host the proxy lives in.
     

    As you can see, the logic of describing the data flow given an startting point is:
    1. Complex.
    2. Incomplete as we only can chart the dataflows that have ultimate origin in this system.
      (consider starting with an spdaq system in an event built case, we only see the branch
       of the dataflow that originates in that system not, e.g. the dataflow asssociated with other
       source ids).
       
    The incompleteness is not normally a problem because we are typically troubleshooting dataflow
    stalls that we know, or think we know, are due to the failure of a specific data sourcde or
    the stalling of some consumer in that source's data flow.l
'''