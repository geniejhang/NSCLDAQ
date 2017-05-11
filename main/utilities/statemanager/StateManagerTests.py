#
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
# @file   StateManagerTests.py
# @brief  Tests for the statemachine data type bindings
# @author <fox@nscl.msu.edu>

import unittest
import tempfile
import os
import signal
import subprocess
import time


import nscldaq.vardb.vardb
import nscldaq.vardb.varmgr

import nscldaq.vardb.statemanager
import testBase

import sys
import gc

class StateManagerTests(testBase.TestBase):
    
    def dbUri(self):
        uri = 'file://' + self._dbName
        return uri
    
    def stockDatabase(self):
        nscldaq.vardb.vardb.create(self._dbName)
        api = nscldaq.vardb.varmgr.Api(self.dbUri())
        
        # Run state:
        
        
        api.defineStateMachine(
            'RunStateMachine',
            {
                '0Initial' : ('NotReady',),
                'NotReady' : ('Readying', '0Initial'),
                'Readying' : ('Ready', 'NotReady'),
                'Ready'    : ('NotReady',)
            }
        )                # Don't really need the full state machine for tests.
        
        api.mkdir("/RunState")
        api.declare("/RunState/RunNumber", "integer")
        api.declare("/RunState/Title", "string")
        api.declare("/RunState/Recording", "bool", "false")
        api.declare('/RunState/Timeout', 'integer', '60')
        api.declare('/RunState/ReadoutParentDir', 'string')
        api.declare('/RunState/State', 'RunStateMachine')
        api.declare('/RunState/SystemStatus', 'string', 'Consistent')
        
        # Make the test program:
        
        api.mkdir("/RunState/test")
        api.declare('/RunState/test/standalone', 'bool', 'false')
        api.declare('/RunState/test/enable', 'bool')
        api.declare('/RunState/test/path', 'string')
        api.declare('/RunState/test/host', 'string')
        api.declare('/RunState/test/outring', 'string')
        api.declare('/RunState/test/inring', 'string')
        api.declare('/RunState/test/State', 'RunStateMachine', '0Initial')
        api.declare('/RunState/test/editorx', 'integer', '0')
        api.declare('/RunState/test/editory', 'integer', '0')
        
        
    def setUp(self):

        # Get a temp file name..
    
        myVarDb = tempfile.NamedTemporaryFile()
        self._dbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        
        # Create the stuff in the database:
        
        self.stockDatabase()
        
        #  Locate the server (it's in $BINDIR/vardbServer)
        #  This is normally a shell script that runs the server
        #  passing parameters to it.
        
        bindir = os.getenv('DAQBIN')
        self._server = os.path.join(bindir, 'vardbServer')
        #
        #  Now initialize variables that startServer might create
        #  to None
        #
        self._pid    = None
        self._stdout = None
        #
        #  Start the server unconditionally
        #
        self.startServer(['--database', self._dbName, '--create-ok', 'yes'])
        p = self.getport('vardb-request')       # Wait for server to publish services.
        self._api = nscldaq.vardb.varmgr.Api('tcp://localhost:%d' % p)
        self._sm =nscldaq.vardb.statemanager.Api(
            'tcp://localhost', 'tcp://localhost'
        )


    def tearDown(self):
        
        if self._pid is not None:
            os.kill(self._pid, signal.SIGKILL)
            os.waitpid(self._pid,0)
            self.waitPortGone('vardb-request')
            self._pid = None
        if self._stdout is not None:
            self._stdout.close()
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
            del self._api
        del self._sm
        gc.collect()
            

class ProgramParentDir(StateManagerTests):
        
    def test_getprogramparentdir_initial(self):
        self.assertEqual('/RunState', self._sm.getProgramParentDir())
        
    def test_getprogramparentdir_changed(self):
        self._api.mkdir('/programs')
        self._api.set('/RunState/ReadoutParentDir', '/programs')
        time.sleep(0.5)           # Let the threads update this.
        self.assertEqual('/programs', self._sm.getProgramParentDir())
        
    def test_getprogramparentdir_argchecks(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.getProgramParentDir('abcde')
            
    def test_setprogramparentdir(self):
        self._api.mkdir('/programs')
        self._sm.setProgramParentDir('/programs')
        self.assertEqual('/programs', self._sm.getProgramParentDir())
    def test_setprogramparentdir_argchecks(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.setProgramParentDir()         # too few
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.setProgramParentDir('/a', '/b') # too many
        
class ProgramDef(StateManagerTests):
    def test_addProgram_ok(self):     # Fully specified program:
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }

        self._sm.addProgram('myprogram', program)
        self.assertEqual(('myprogram', 'test'), self._api.ls('/RunState'))
        self.assertEqual('true', self._api.get('/RunState/myprogram/enable'))
        self.assertEqual('false', self._api.get('/RunState/myprogram/standalone'))
        self.assertEqual('/users/fox/test', self._api.get('/RunState/myprogram/path'))
        self.assertEqual('charlie.nscl.msu.edu', self._api.get('/RunState/myprogram/host'))
        self.assertEqual('output', self._api.get('/RunState/myprogram/outring'))
        self.assertEqual('tcp://localhost/george',self._api.get('/RunState/myprogram/inring'))
        
    def test_addProgram_defaults(self):    # Partially speced gives defaults.
        program = {
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
        }                         # Minimal def.

        self._sm.addProgram('myprogram', program)
        self.assertEqual(('myprogram', 'test'), self._api.ls('/RunState'))
        self.assertEqual('true', self._api.get('/RunState/myprogram/enable'))
        self.assertEqual('false', self._api.get('/RunState/myprogram/standalone'))
        self.assertEqual('/users/fox/test', self._api.get('/RunState/myprogram/path'))
        self.assertEqual('charlie.nscl.msu.edu', self._api.get('/RunState/myprogram/host'))
        self.assertEqual('', self._api.get('/RunState/myprogram/outring'))
        self.assertEqual('',self._api.get('/RunState/myprogram/inring'))
                
    def test_addProgram_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.addProgram('myprogram')
        program = {
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
        }  
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.addProgram('myprogram', program, 'junk')
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.addProgram('myprogram', 'junk')   # program def mb dict.
        
    def test_addProgram_dupname(self):
        program = {
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
        }                         # Minimal def.
        self._sm.addProgram('myprogram', program)
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.addProgram('myprogram', program)
        
    def test_addProgram_underspecified(self):
        program = {
             'host' : 'charlie.nscl.msu.edu',
        }
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.addProgram('myprogram', program)
         
        program = { 'path' : '/user/fox/junk' }
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            self._sm.addProgram('myprogram', program)
        
    def test_getProgramDef_ok(self):
        self.assertEqual(
            {'standalone' : False, 'enabled' : True, 'path' : '', 'host' : '',
             'outring' : '', 'inring' : ''},
            self._sm.getProgramDefinition('test'))
        
    def test_getProgramDef_badName(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getProgramDefinition('junk')
        
    def test_getProgramDef_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getProgramDefinition()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getProgramDefinition('test', 'junk')
        
    def test_modifyProgram_ok(self):

        progDef = self._sm.getProgramDefinition('test')
        #
        #  Make some mods:
        #
        progDef['path'] = '/home/fox/stuff'
        progDef['host'] = 'spdaq20.nscl.msu.edu'
    
        self._sm.modifyProgram('test', progDef)
        
        #  Check that they took.
        
        self.assertEqual(progDef, self._sm.getProgramDefinition('test'))
        
        
    def test_modifyProgram_nosuch(self):
        progDef = self._sm.getProgramDefinition('test')
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.modifyProgram('junk', progDef)    # No such program.

        
    def test_modifyProgram_checkargs(self):
        progDef = self._sm.getProgramDefinition('test')
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.modifyProgram('test')     # need def.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.modifyProgram('test', progDef, 'junk')   #extra stuff
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.modifyProgram('test', 'junk')    # def not dict.
        
    def test_setEditorPosition(self):
        self._sm.setEditorPosition('test', 100, 200)
        
        self.assertEqual("100", self._api.get('/RunState/test/editorx'))
        self.assertEqual("200", self._api.get('/RunState/test/editory'))
        
    def test_getXpos(self):

        
        self._sm.setEditorPosition('test', 100, 200)
        self.assertEqual(100, self._sm.getEditorXPosition('test'))

        
    def test_getYpos(self):
        self._sm.setEditorPosition('test', 100, 200)
        self.assertEqual(200, self._sm.getEditorYPosition('test'))

class ProgramParticipation(StateManagerTests):        
    def test_enableProgram_ok(self):
        self._api.set('/RunState/test/enable', 'false')   # first disable.

        self._sm.enableProgram('test')
        self.assertEqual('true', self._api.get('/RunState/test/enable'))

        
    def test_enableProgram_nosuch(self):

        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.enableProgram('junk')
        
    def test_enableProgram_argCheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.enableProgram()            # need name.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.enableProgram('test', 'junk') # Extra param
        
    def test_disableProgram_Ok(self):
        self._api.set('/RunState/test/enable', 'true')   # first enable.

        self._sm.disableProgram('test')
        self.assertEqual('false', self._api.get('/RunState/test/enable'))
        
    def test_disableProgram_nosuch(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.disableProgram('junk')
        
    def test_disablProgram_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.disableProgram()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.disableProgram('test', 'junk')
        
    def test_isProgramEnabled_ok(self):
        self._sm.enableProgram('test')
        self.assertTrue(self._sm.isProgramEnabled('test'))
        
        self._sm.disableProgram('test')
        self.assertFalse(self._sm.isProgramEnabled('test'))
        
    def test_isProgramEnabled_nosuch(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isProgramEnabled('junk')
        
    def test_isProgramEnabled_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isProgramEnabled()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isProgramEnabled('test', 'junk')
        
    def test_setStandalone_ok(self):
        self._api.set('/RunState/test/standalone', 'false')  #ensure false first.

        self._sm.setProgramStandalone('test')
        self.assertEqual('true', self._api.get('/RunState/test/standalone'))
        
    def test_setStandalone_nosuch(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramStandalone('junk')
        
    def test_setStandalone_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramStandalone()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramStandalone('test', 'junk')
        
    def test_setNostandalone_ok(self):
        self._api.set('/RunState/test/standalone', 'true')  #ensure true
        self._sm.setProgramNoStandalone('test')
        self.assertEqual('false', self._api.get('/RunState/test/standalone'))
        
    def test_setNostandalone_nosuch(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramNoStandalone('junk')
        
    def test_setNostandalone_argcheck(self):

        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramNoStandalone()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramNoStandalone('test', 'junk')

    def test_isStandalone_ok(self):
        self._sm.setProgramStandalone('test')
        self.assertTrue(self._sm.isProgramStandalone('test'))
        
        self._sm.setProgramNoStandalone('test')
        self.assertFalse(self._sm.isProgramStandalone('test'))
        
        
    def test_isStandalone_nosuch(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isProgramStandalone('junk')
        
    def test_isStandalone_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isProgramStandalone()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isProgramStandalone('test', 'junk')
        
class ProgramListing(StateManagerTests):   
    def test_listprograms_1(self):
        self.assertEquals(['test'], self._sm.listPrograms())
        
    def test_listprograms_a_few(self):

        # Add some programs:
        
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('atest', program)
        self._sm.addProgram('btest', program)
        self._sm.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'test', 'ztest'],
            self._sm.listPrograms()
        )
        
    def test_listprograms_none(self):
        self._api.mkdir("/programs")         # empty.

        self._sm.setProgramParentDir('/programs')
        self.assertEquals([], self._sm.listPrograms())
        
    def test_listprograms_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.listPrograms('junk')
        
    def test_listenabled_1(self):
        self.assertEquals(['test'], self._sm.listEnabledPrograms())
        
    def test_listenabled_multiple(self):
        # Add some programs (all enabled):
        
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('atest', program)
        self._sm.addProgram('btest', program)
        self._sm.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'test', 'ztest'],
            self._sm.listEnabledPrograms()
        )
        
    def test_listenabled_none(self):
        # disable test:
        
        self._sm.disableProgram('test')
        
        self.assertEquals([], self._sm.listEnabledPrograms())
        
    def test_listenabled_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.listEnabledPrograms('junk')

        
    def test_liststandalone_none(self) :
        self.assertEquals([], self._sm.listStandalonePrograms())
        
    def test_liststandalone_1(self) :
        self._sm.setProgramStandalone('test')
        self.assertEquals(['test'], self._sm.listStandalonePrograms())
        
    def test_liststandalone_many(self):
        program = {
            'enabled' : True, 'standalone' : True,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('atest', program)
        self._sm.addProgram('btest', program)
        self._sm.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'ztest'],
            self._sm.listStandalonePrograms()
        )
        
    def test_liststandalone_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.listStandalonePrograms('junk')
        
    # Active programs are those that are enabled and not standalone.
    
    def test_listinactive_none(self):
        self.assertEquals([], self._sm.listInactivePrograms())
        
    def test_listinactive_1(self):

        self._sm.disableProgram('test')
        self.assertEquals(['test'], self._sm.listInactivePrograms())
        
        self._sm.enableProgram('test')
        self._sm.setProgramStandalone('test')
        self.assertEquals(['test'], self._sm.listInactivePrograms())
        
    def test_listinactive_several(self):
        program = {
            'enabled' : True, 'standalone' : True,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('atest', program)
        
        program['standalone'] = False
        program['enabled']    = False
        self._sm.addProgram('btest', program)
        
        program['standalone'] = True
        self._sm.addProgram('ztest', program)
        
        self.assertEquals(
            ['atest', 'btest', 'ztest'], self._sm.listInactivePrograms()
        )
        
    def test_listinactive_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.listInactivePrograms('junk')
        
    def test_listactive_1(self):
        self.assertEquals(['test'], self._sm.listActivePrograms())
        
    def test_listactive_0(self):
        self._sm.disableProgram('test')
        self.assertEquals([], self._sm.listActivePrograms())
        
        self._sm.enableProgram('test')
        self._sm.setProgramStandalone('test')
        self.assertEquals([], self._sm.listActivePrograms())
        
    def test_listactive_many(self):
        program = {
            'enabled' : True, 'standalone' : True,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('atest', program)
        
        program['standalone'] = False
        self._sm.addProgram('btest', program)
        
        program ['enabled'] = False
        self._sm.addProgram('ztest', program)
        
        self.assertEqual(['btest', 'test'], self._sm.listActivePrograms())
        
    def test_listactive_argcheck(self) :
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.listActivePrograms('junk')
        
class DeleteProgram(StateManagerTests):
    def test_deleteprog_ok(self):

        self._sm.deleteProgram('test')
        self.assertEqual([], self._sm.listPrograms())
        
    def test_deleteprog_nox(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.deleteProgram('nosuch')
        
    def test_deleteprog_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.deleteProgram()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.deleteProgram('test', 'junk')
        
class StateS(StateManagerTests):
    def test_setglobalstate_ok(self):
        self._sm.setGlobalState('NotReady')
        self.assertEquals('NotReady', self._api.get('/RunState/State'))

        
    def test_setglobalstate_badstate(self) :
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setGlobalState('Readying')        # Inv transtion.
        
    def test_setglobalstate_argcheck(self) :
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setGlobalState()        # Missing param.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setGlobalState('NotReady', 'junk')
        
    def test_getglobalstate_initial(self):
        self.assertEqual('0Initial', self._sm.getGlobalState())
        
    def test_getglobalstate_changed(self):
        self._sm.setGlobalState('NotReady')
        self.assertEqual('NotReady', self._sm.getGlobalState())
        
    def test_getglobalstate_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getGlobalState('junk')
        
    def test_getpartstates_initial(self):
        self.assertEqual(
            {'test' : '0Initial'}, self._sm.getParticipantStates()
        )
        
    def test_getpartstates_seversl(self):

        programDef = {
        'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('atest', programDef)
        self._sm.addProgram('ztest', programDef)
        
        self.assertEqual(
            {'atest' : '0Initial', 'test': '0Initial', 'ztest' : '0Initial'},
            self._sm.getParticipantStates()
        )
        
    def test_getpartstates_modified(self):
        programDef = {
        'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('atest', programDef)
        self._sm.addProgram('ztest', programDef)
        
        #  Diddle some states:
        
        self._api.set('/RunState/test/State', 'NotReady')
        self._api.set('/RunState/test/State', 'Readying')
        
        self._api.set('/RunState/ztest/State', 'NotReady')
        self._api.set('/RunState/atest/State', 'NotReady')
        
        self.assertEqual(
            {'atest' : 'NotReady', 'test' : 'Readying', 'ztest': 'NotReady'},
            self._sm.getParticipantStates()
        )
        
    def test_getpartstates_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getParticipantStates(0)
        
    def test_getSystemStatusConsistent(self):
        self.assertEqual('Consistent', self._sm.getSystemStatus())

    def test_getSystemStatusInconsistent(self):
        self._api.set('/RunState/SystemStatus', 'Inconsistent')
        self.assertEqual('Inconsistent', self._sm.getSystemStatus())

        
class RunParameters(StateManagerTests):   
    def test_gettitle_initial(self):
        self.assertEqual('', self._sm.getTitle())
        
    def test_gettitle_modified(self):
        title = 'A new title'
        
        self._api.set('/RunState/Title', title)
        self.assertEqual(title, self._sm.getTitle())
        
    def test_gettitle_argcheck(self):

        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getTitle(0)
        
    def test_settitle_ok(self):
        title = 'This is a test title'
        self._sm.setTitle(title)
        self.assertEqual(title, self._sm.getTitle())
        
    def test_settitle_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setTitle()           # Need parameter.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setTitle('abcde', 0)   # extra param.
        
    def test_gettimeout_initial(self):
        self.assertEqual(60, self._sm.getTimeout())
        
    def test_gettimeout_modified(self):
        self._api.set("/RunState/Timeout", '45')
        self.assertEqual(45, self._sm.getTimeout())
        
    def test_gettimeout_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getTimeout(0)
        
    def test_settimeout_ok(self):
        self._sm.setTimeout(1234)
        self.assertEqual(1234, self._sm.getTimeout())
        
    def test_settimeout_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setTimeout()    # missing arg.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setTimeout('bad')   # Bad time.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setTimeout(1234, 42)   #Extra args.
        
    def test_isrecording_initial(self):
        self.assertFalse(self._sm.isRecording())
        
    def test_isrecording_changed(self):
        self._api.set('/RunState/Recording', 'false')
        self.assertFalse(self._sm.isRecording())
        
    def test_isrecording_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isRecording(0)
        
    def test_setrecording_ok(self):
        self._sm.setRecording(True)
        self.assertTrue(self._sm.isRecording())
        
        self._sm.setRecording(False)
        self.assertFalse(self._sm.isRecording())
        
    def test_setrecording_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setRecording()      # Missing parameter.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setRecording(True, 0)   # Extra param.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setRecording('1234')      # not a bool.
        
    def test_getrunnum_initial(self):
        self.assertEqual(0, self._sm.getRunNumber())

    def test_getrunnum_changed(self):
        self._api.set("/RunState/RunNumber", "1234")
        self.assertEqual(1234, self._sm.getRunNumber())

        
    def test_getrunnum_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getRunNumber(0)
        
    def test_setrunnum_ok(self):
        self._sm.setRunNumber(1234)
        self.assertEqual(1234, self._sm.getRunNumber())
        
    def test_setrunnum_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setRunNumber()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setRunNumber(1234, 0)
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setRunNumber('abcd')
        

class WaitTransitionTests(StateManagerTests):
    def __init__(self, *args, **kwargs):
        super(WaitTransitionTests, self).__init__(*args, **kwargs)
        self._callbacklist = None

    
    def StateTransitionCallback(self, api, program, state, cd):
        self._api.set('/RunState/%s/State' % program, state)
        self._callbacklist.append([program, state, cd])

        
    def test_waittransition_timeout(self):
        self._sm.setTimeout(1)     # 1 second shortest timeout possible.
        self._sm.setGlobalState('NotReady')
        self.assertFalse(self._sm.waitTransition())
        
    def test_waittransition_onestep(self):
        self._sm.setGlobalState('NotReady')
        self._api.set('/RunState/test/State', 'NotReady')
        self.assertTrue(self._sm.waitTransition())
        
        
    def test_waittransition_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.waitTransition(1, 0)    # Not callable!
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.waitTransition(
                self.test_waittransition_argcheck, 0, 1
            )                          # too many params.
        
class ProcessMessageTests(StateManagerTests):
    def __init__(self, *args, **kwargs):
        super(ProcessMessageTests, self).__init__(*args, **kwargs)
       
        
    def setUp(self) :
        super(ProcessMessageTests, self).setUp()
        self._callbacklist = list()
        
    def tearDown(self):
        super(ProcessMessageTests, self).tearDown()
        
    def Callback(self, api, Notification, cd):
        self._callbacklist.append([Notification, cd])
    
    def test_processMessages_noCallbacks(self):

        self._sm.processMessages(self.Callback, 1)
        self.assertEquals(
            [], self._callbacklist
        )
        
    def test_processMessages_gblchange(self):
        self._sm.setGlobalState('NotReady')
        time.sleep(0.5)        # Wait for the publications.
        self._sm.processMessages(self.Callback, 2)
        self.assertEquals(
            [
                [{'type' : "GlobalStateChange", "state" : "NotReady"}, 2],
                [{'type': "VarChanged", "path": "/RunState/SystemStatus", "value": "Inconsistent"}, 2],
            ],
            
            self._callbacklist
        )
        
    def test_processMessage_programchange(self):
        self._api.set('/RunState/test/State', 'NotReady')
        time.sleep(0.5)
        self._sm.processMessages(self.Callback, 3)
        self.assertEquals(
            [[{'type' : "ProgramStateChange",
               "state" : "NotReady",
               'program' : 'test'}, 3]],
            self._callbacklist
        )
        
    def test_processMessage_joins(self):
        program = {
            'enabled' : True, 'standalone' : False,
            'path' : '/users/fox/test', 'host' : 'charlie.nscl.msu.edu',
            'outring' : 'output', 'inring' : 'tcp://localhost/george'
        }
        self._sm.addProgram('newprogram', program)
        time.sleep(0.5)
        self._sm.processMessages(self.Callback, 4)
        self.assertEquals(
            [[{'type' : 'ProgramJoins', 'program': 'newprogram'}, 4]],
            self._callbacklist
        )
        
    def test_processMessage_leaves(self):
        self._sm.deleteProgram('test')
        time.sleep(0.5)
        self._sm.processMessages(self.Callback, 5)
        self.assertEquals(
            [[{'type' : 'ProgramLeaves', 'program' : 'test'}, 5]],
            self._callbacklist
        )
        
    def test_processMessages_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.processMessages()
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.processMessages(1)
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.processMessages(self.Callback, 1, 2)

class ProgramStates(StateManagerTests):
    def test_isactive_ok(self):
        self.assertTrue(self._sm.isActive('test'))
        
        self._sm.disableProgram('test')
        self.assertFalse(self._sm.isActive('test'))
        
    def test_isactive_nosuch(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isActive('no-such-program')
        
    def test_isactive_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isActive()           # Need program.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.isActive('test', 'junk') # Extra param.
        
    def test_setprogstate_ok(self):
        self._sm.setProgramState('test', 'NotReady')
        self.assertEquals(
            {'test' : 'NotReady'}, self._sm.getParticipantStates()
        )
        
    def test_setprogstate_nosuch(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramState('nosuch', 'NotReady')
        
    def test_setprogstate_badstate(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramState('test', 'Ready')
        
    def test_setprogstate_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramState('test')  # Too few args.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.setProgramState('test', 'NotReady', 0) #Too many
        
    def test_getprogstate_ok(self):
        self.assertEqual('0Initial', self._sm.getProgramState('test'))
        self._sm.setProgramState('test', 'NotReady')
        self.assertEqual('NotReady', self._sm.getProgramState('test'))
        
    def test_getprogstate_argcheck(self):
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getProgramState()    # Need program.
        with self.assertRaises(nscldaq.vardb.statemanager.error) :
            self._sm.getProgramState('test', 0) # Extra param.

# Where the api only uses one URI


class ProgramOnlyTests(StateManagerTests):
    def test_setprogramDir(self):
        api = nscldaq.vardb.statemanager.Api('tcp://localhost')
        api.setProgramParentDir('/')
        self.assertEqual('/', api.getProgramParentDir())
        del api
        
    def test_badmethod(self):
        api = nscldaq.vardb.statemanager.Api('tcp://localhost')
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.setGlobalState('NotReady')
        del api
        
    def test_setProperty_create(self ):
        api = nscldaq.vardb.statemanager.Api('tcp://localhost')
        api.setProgramProperty('test', 'myprop', 'avalue')
        self.assertEqual('avalue' , self._api.get('/RunState/test/myprop'))
        del api
        
    def test_setProperty_afterCreation(self):
        api = nscldaq.vardb.statemanager.Api('tcp://localhost')
        api.setProgramProperty('test', 'myprop', 'avalue')
        api.setProgramProperty('test', 'myprop', 'bvalue', False)
        self.assertEqual('bvalue' , self._api.get('/RunState/test/myprop'))
        del api
        
    def test_setProperty_nosuch(self):
        api = nscldaq.vardb.statemanager.Api('tcp://localhost')
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.setProgramProperty('test', 'myprop', 'bvalue', False)
        del api
        
    def test_getProperty(self):
        api = nscldaq.vardb.statemanager.Api('tcp://localhost')
        api.setProgramProperty('test', 'myprop', 'avalue')
        self.assertEqual('avalue', api.getProgramProperty('test', 'myprop'))
        del api
        
    def test_getProperty_nosuch(self):
        api = nscldaq.vardb.statemanager.Api('tcp://localhost')
        with self.assertRaises(nscldaq.vardb.statemanager.error):
            api.getProgramProperty('test', 'myprop')
        del api
        
# Run the tests if this is main:


if __name__ == '__main__':
    unittest.main(verbosity=0)
    gc.collect()
