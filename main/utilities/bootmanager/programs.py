#!/usr/bin/env python



#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   programs.py
# @brief  Class that manages programs.
# @author <fox@nscl.msu.edu>

from nscldaq.programs import ssh
import os
import Tkinter                 # Used to parse Tcl lists.

##
# Class to manage a set of programs that are run on other platforms.
class Programs:
    def __init__(self, requri, suburi, client, servicesApi):
        self._reqUri = requri
        self._subUri = suburi
        self._client = client
        self._services = servicesApi
        self._filesToProgram = dict()
        self._namesToServices = dict()
    
    #----------------- Private interfaces --------------------


    ##
    # __getFds
    #   Return a list of the two fds for a program.
    # @param prog - program object.
    # @return list [2] stdin, stderr fds, no order gaurantees.
    #
    def __getFds(self, prog):
        return  [fd for fd in self._filesToProgram.keys()
            if self._filesToProgram[fd] == prog
        ]
    
    ##
    # __getProgramDef
    #     Given a program name returns a dict with:
    #     'host' - host the program runs in.
    #     'path' - Path to the program.
    #     'name' - program name again.
    # @param name - name of the program.
    # @return dict - see above.
    #
    def __getProgramDef(self, name):
        fullDef = self._client.getProgramDefinition(name)
        result  = {
            'host': fullDef['host'], 'path': fullDef['path'], 'name': name,
            'outring': fullDef['outring'], 'inring' : fullDef['inring']
        }
        return result
    
    ##
    # __makeProgeramEnv
    #
    #   Creates the environment definition for a program.  This is a map with:
    #   'REQ_URI' - the request URI.
    #   'SUB_URI' - The subscription URI.
    #   'PROGRAM' - The program name.
    #   'DAQROOT' - Current value
    #   'DAQBIN'  - Current value
    #   'DAQLIB'  - Current Value
    #   'PYTHONPATH' - Current value.
    #   'OUTRING' - Output ring.
    #   'INRING'  - Input ring.
    #
    #  @param progDef - program definition.
    #  @return dict as described above.
    def __makeProgramEnv(self, progDef):
        return {
            'PROGRAM': progDef['name'], 'REQ_URI': self._reqUri,
            'SUB_URI': self._subUri,
            'OUTRING' : progDef['outring'], 'INRING' : progDef['inring'],
            'DAQROOT' : os.getenv('DAQROOT'), 'DAQBIN' : os.getenv('DAQBIN'),
            'DAQLIB' : os.getenv('DAQLIB'), 'PYTHONPATH' : os.getenv('PYTHONPATH')
        }
    ##
    # __tclListToString
    #   Process a Tcl List to a string.
    # @param tclList the list.
    # @return string - the resulting parsed list.
    #
    def __tclListToString(self, tclList):
        result = ' '
        if tclList != '':
            pyList = []
            tclInterp = Tkinter.Tcl().tk.eval
            tclInterp('set list "%s"' % tclList)
            size = int(tclInterp('llength $list'))
            for i in range(size):
                element = tclInterp('lindex $list %d' % i)
                pyList.append(element)
            result += ' '.join(pyList)
    
        return result
    ##
    # __getProgramParameters
    #   Decodes the Tcl list (might be empty) in the 'Program Parameters'
    #   property of a program and returns it.
    #
    # @param name - name of the program.
    # @return string - Tcl list of parameters turned into a string.
    #
    def __getProgramParameters(self, name):
        result = ' '
        tclList = self._client.getProgramProperty(name, 'Program Parameters').strip()
        return self.__tclListToString(tclList)
    
    ##
    # __getServiceParameters
    #   Returns the optional program parameters for a service.
    # @param name - name of the servicwe
    #
    def __getServiceParameters(self, name):
        tclList = self._services.getProperty(name, 'args').strip()
        return self.__tclListToString(tclList)
    
    ##
    # __propertiesToOptions
    #    Given a map whose keys are properties and whose values are corresponding
    #    option names, computes program options based on the properties that
    #    are present (non empty).
    #
    # @param name - program name
    # @param map  - Property map described above.
    #
    def __propertiesToOptions(self, name, map):
        optionList = []
        for propName in map.keys():
            try :
                propValue = self._client.getProgramProperty(name, propName).strip()
                if propValue != '':
                    optname = map[propName]
                    optionList.append(optname + '=' + propValue)
            except:
                pass             # Just assume a property is not defined.
        return ' ' + ' '.join(optionList)
    ##
    # __propertiesToFlags
    #
    #   Processes properties that have 'boolean' values that, if 'true' result
    #   in the presence of a flag parameter.
    #
    # @param name - the program name.
    # @param map  - map of property names to flag names.
    # @return string - command line options
    #
    def __propertiesToFlags(self, name, map):
        flags = []
        for propName in map.keys():
            try :
                propValue = self._client.getProgramProperty(name, propName).strip()
                if (propValue  == 'true') :
                    flags.append(map[propName])
            except:
                pass                     # Assume no property defined.
        
        return ' ' + ' '.join(flags)
    #
    ##
    # __getReadoutParameters
    #    Get the properties associated with readout prgorams and translate them
    #    into program command line options.
    #
    # @param name - program name.
    # @return string - command line parameters/options
    #
    def __getReadoutParameters(self, name):
        readoutPropertyMap = {
            'init script' : '--init-script',
            'outring'     : '--ring',
            'port'        : '--port',
            'sourceid'    : '--sourceid',
            'status service' : '--status-service','appname'     : '--appname'
        }
        
        result = self.__propertiesToOptions(name, readoutPropertyMap)
        return result
    ##
    # __getEventLogParameters
    #    Returns the event log command line options described by the program's
    #    properties.
    #
    # @param name - Name of the program.
    # @return string - command line options.
    #
    def __getEventLogParameters(self, name):
        eventlogPropertyMap = {
            'appname'      : '--appname',
            'freesevere'   : '--freesevere',
            'freewarn'     : '--freewarn',
            'inring'       : '--source',
            'prefix'       : '--prefix',
            'segmentsize'  : '--segmentsize',
            'status server' : '--service'
        }
        result = self.__propertiesToOptions(name, eventlogPropertyMap)
        
        eventlogFlagMap = {
            'checksum'     : '--checksum',
            'combineruns'  : '--combine-runs'
        }
        result += ' ' + self.__propertiesToFlags(name, eventlogFlagMap)
        
        return result
    
    ##
    # __programArguments
    #   The actual parameters passed to a program depends on the program type
    #   and the property settings it has.  This method dispatches to known program
    #   type handlers and just returns the Tcl decoded string for
    #   the property 'Program Parameters' for types not recognized.
    #
    # @param name  - Name of the program being started.
    # @param pgmType  - program's 'type' property.
    # @return string - Trailing program command line parameters.
    #
    def __programArguments(self, name, pgmType):
        result = ''
        if pgmType == 'Readout':
            result += self.__getReadoutParameters(name)
        elif pgmType == 'EventLog':
            result += self.__getEventLogParameters(name)
            
            
        # All types glom in the 'Program Parameters' list.  Unrecognized types
        # fall through here and that's all they get.
        
        result += ' ' + self.__getProgramParameters(name)
        return result
    
    ##
    # Start a program
    #  @param program - name of the program to start.
    #
    def __startProgram(self, program):
        programDef = self.__getProgramDef(program)
        programEnv = self.__makeProgramEnv(programDef)
        host       = programDef['host']
        path       = programDef['path']
        
        # Set the program's state to readying:
        
        self._client.setProgramState(program, 'Readying')
        
        # Construct the command line depending on the program type:
        
        programType = self._client.getProgramProperty(program, 'type')
        args        = self.__programArguments(program, programType)
        command = path + args
        
        # Create the program and make map entries from its stdout/stderr for it.
        
        programObj = ssh.program(host, command, programEnv, program)
        
        stdout = programObj.stdout()
        stderr = programObj.stderr()

        self._filesToProgram[stdout] = programObj
        self._filesToProgram[stderr] = programObj
        
    
    ##
    # __startDataFlowService
    #    Starts a dataflow service
    #
    # @param programDef - the program definition dict.
    #
    def __startDataFlowService(self, programDef):
        programEnv = self.__makeProgramEnv(programDef)
        
        host = programDef['host']
        path = programDef['path']
        
        args = self.__getServiceParameters(programDef['name'])
        command = path + ' ' + args
        
        print('starting' + command + '@', 'host')
        programObj = ssh.program(host, command, programEnv, programDef['name'])
        stdout     = programObj.stdout()
        stderr      = programObj.stderr()
        self._filesToProgram[stdout] = programObj
        self._filesToProgram[stderr] = programObj
        self._namesToServices[programDef['name']] = programObj
        
        
    ##
    # _setAllNotReady
    #    Set all active programs to notready state:
    #
    def _setAllNotReady(self)  :
        for name in self._client.listActivePrograms() :
            if self._client.getProgramState(name) != 'NotReady':
                self._client.setProgramState(name, 'NotReady')
            
    ##
    # __listDataFlowServices
    #
    #    returns a list of data flow definition services.  These are services
    #    that have the type property value 'dataflow' and therefore have an
    #    inring/outring.k
    #
    # The result produced is a list of dicts that are much like what's
    #  produced by getProgramDef.
    #
    def __listDataFlowServices(self):
        result = list()
        serviceListDict = self._services.list()
        
        for servicename in serviceListDict.keys():
            serviceType = self._services.getProperty(servicename, 'type')
            if serviceType == 'dataflow':
                
                # Dataflow object type make the dict.
                
                programDef = {
                    'name': servicename, 'path': serviceListDict[servicename][0],
                    'host' : serviceListDict[servicename][1],
                    'outring': self._services.getProperty(servicename, 'outring'),
                    'inring' : self._services.getProperty(servicename, 'inring')
                }
                result.append(programDef)
        return result
     
    ##
    # __stopDataFlowServices
    #    Destroys service programs.  This should result in all of them exiting.
    #
    def __stopDataFlowServices(self):
        for name in self._namesToServices:
            program = self._namesToServices[name]
            program.intr()
            self._namesToServices[name] = None
            del program
        
    #----------------- Public interfaces ---------------------
        
    ##
    # start
    #   start all of the programs that are defined.
    #
    def start(self):
        self._filesToProgram = dict()
        programList = self._client.listPrograms()
        for program in programList:
            self.__startProgram(program)
            
        serviceList = self.__listDataFlowServices()
        for service in serviceList:
            self.__startDataFlowService(service)
    ##
    # All programs were externally stopped due to system shutdown.
    #
    def stop(self):
        self._setAllNotReady()
        self._filesToProgram = dict()
        self.__stopDataFlowServices()
    
    ##
    # getFiles:
    #
    #  Return the file descriptors for the stdout/stderr for all programs:
    #
    # @return list
    #
    def getFiles(self):
        return self._filesToProgram.keys()
    ##
    # getProgram
    #   Given a file descriptor return the program it belongs to:
    #
    # @param file - the file descriptor.
    # @return ssh.Program object.
    #
    def getProgram(self, file):
        return self._filesToProgram[file]
    ##
    # getPrograms
    #   Returns all of the program objects associated with the program:
    #
    # @return list - list of program objects.
    #
    def getPrograms():
        return list(set(self._filesToProgram.values()))  # set -> unique objects.

    def programExited(self, item):
        programName = item.name()
        print("programs.programExited called")
        fds = self.__getFds(item)
        self._filesToProgram.pop(fds[0])
        self._filesToProgram.pop(fds[1])
        try:
            self._client.setProgramState(name, 'NotReady')  # May already be.
        except:
            pass
        self.stop()
        
