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
  
  ring2stdout ... --comment "Hoisting to host"
  
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

import nscldaqutils
import pidtocommand
import socket
import os.path


#------------------------ Internal utiltities

# _getLocalCommand
#   Return a string that represents the 
#   command associated with a pid.
#   If pidtocommand.pidToCommand() raises,
#   '<unavailable>' is returned.

def _getLocalCommand(pid):
    try:
        return ' '.join(pidtocommand.pidToCommand(pid))
    except:
        return '<unavailable>'
    
    
# _getRemoteCommand
#   Get the command associated with a pid in an external host
#  (well really this will work with local commands if host is e.g. 'localhost' but will
#  be much slower than _getLocalCommand)
#
# Prerequisistes:
#   * The DAQ must have been setup (e.g. we relay on DAQBIN)
#   * Passwordless ssh must have been set up for the account.
#
# Return value is, as for _getLocalCommand.
#
def _getRemoteCommand(host, pid):
    result = nscldaqutils.ssh(host, f'$DAQBIN/pidtocommand {pid}')
    output = nscldaqutils.getSshOutput(result)
    error  = nscldaqutils.getSshError(result)
    
    # If there are no lines in ouptput, then we got nothing:
    
    
    if len(output) >0:
        return output[0]
    else:
        return "<unavailable>"
        
## Get target from specific hoisters:

# For the native hoister, the second command word is the host:
# Note that it's possible to use ringtostdout in a non
# hoisting capacity when (hopefully) it won't have a  parameter:
# and will just liook like "ringtostdout ringname" rather than
# "ringtostdout ringname hostname"
def _native_hoister(command):
    if len(command) == 3:
        return command[2]
    else :
        return None

# For the rust hoister, we have to find the text after --comment.
#  The host will be the 3'd word of its parameter.
def _rust_hoister(command):
    for idx, param in enumerate(command):
        if param == "--comment":
            return  command[idx+3]
    return None

# Table of hoister target getters by command word.

hoister_target = {
    'ringtostdout' : _native_hoister,
    'ring2stdout'  : _rust_hoister
}

##
# _getHoistTarget
#   Givnen a consumer command, determine the host to which
#   that command is hoisted or return None if the command
#   is not a hoisting command.
#
def _getHoistTarget(cmd):
    global hoister_target
    
    #
    #  Get the command word untangled from path stuff.
    #
    
    command_words = cmd.split()
    command = command_words[0]
    command_name = os.path.split(command)[1]
    
    if command_name in hoister_target.keys():
        return hoister_target[command_name](command_words)
    
    return None
#------------------------ Public entries

def makeLocalRingInfo():
    ''' 
        Get the ringbuffer information for the local rings.
        For those we can directly ask the system what the 
        commands are for the producer and consumers
    
        The return will be an element of the list of dicts that define the 
        rings in a host.    
    '''
    host = socket.gethostname()
    rm = nscldaqutils.RingMaster(host)
    usage = rm.list_rings()
    
    result = {'host': host, 'rings': []}
    for ring in usage:
        
        # We dont' distentangle proxies but we _do_ make an empty list for them.
        
        ring['proxies'] = []
        
        #  Add the producer and consuer commands.
        if ring['producer_pid'] != -1 :
            ring['producer_command'] = _getLocalCommand(ring['producer_pid'])
            
        else :
            ring['producer_command'] = 'None'
        
        for consumer in ring['consumers']:
            consumer['consumer_command'] = _getLocalCommand(consumer['consumer_pid'])
        
        result['rings'].append(ring)

    return result
    

def makeRemoteRingInfo(host):
    '''
        Get the ring buffer information for rings in a remote system.
        See makeLocalRingInfor for what is returned.  Note that we need
        to use ssh to get the process name information in this case.
    '''
    rm = nscldaqutils.RingMaster(host)
    usage = rm.list_rings()
    
    result = {'host': host, 'rings': []}
    for ring in usage:
        
        # We dont' distentangle proxies but we _do_ make an empty list for them.
        
        ring['proxies'] = []
        
        #  Add the producer and consuer commands.
        if ring['producer_pid'] != -1 :
            ring['producer_command'] = _getRemoteCommand(host, ring['producer_pid'])
            
        else :
            ring['producer_command'] = 'None'
        
        for consumer in ring['consumers']:
            consumer['consumer_command'] = _getRemoteCommand(host, consumer['consumer_pid'])
        
        result['rings'].append(ring)

    return result
    
    
def getHoistedHosts(info):
    '''
        Given information about a ringbuffer, return the set of hosts that ring is
        being hoisted to.  This is done by examining the consumers as described in the module
        comments.
        
        Returns, an array of the names or IP addresss of hosts to which the ring is being hoisted.
    '''
    result = []
    for consumer in info['consumers']:
        cmd = consumer['consumer_command']
        host = _getHoistTarget(cmd)
        if host is not None:
            result.append(host)
    
    return result

def getUniqueNames(listOfLists):
    '''
    Given an  iterable of iterables (probably containing strings),
    returns an iterable containing the unique names.
    '''
    result = {}
    for outer in listOfLists:
        for inner in outer:
            result[inner] = ''
    return [x for x in result.keys()]