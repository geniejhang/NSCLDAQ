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
# @file   statusdbtets.py
# @brief  Tests of the python bindings for CStatusDb (nscldaq.status.statusdb).
# @author <fox@nscl.msu.edu>

import unittest
import sqlite3
import tempfile
import time

from nscldaq.status  import statusdb
from nscldaq.status  import statusmessages
from nscldaq.sqlite  import where

class StatusDbTests(unittest.TestCase):
    def setUp(self):
        dbfile       = tempfile.NamedTemporaryFile()
        self._dbname  = dbfile.name
        dbfile.close()
        self._api    = statusdb.statusdb(self._dbname)
        self._sqlite = sqlite3.connect(self._dbname)
        self._sqlite.row_factory = sqlite3.Row
        
    def teardown(self):
        self._sqlite.close()
        self._api = None                   # Should force destruction via GC.

    # Add log message test:
    
    def test_addLogMessage(self):
        self._api.addLogMessage(
            statusmessages.SeverityLevels.WARNING, 'some-application',
            'charlie.nscl.msu.edu', 1234, 'Something to talk about'
        )
        #  Should be an appropriate entry in 
        
        c = self._sqlite.cursor()
        c.execute('SELECT * FROM  log_messages')
        r = c.fetchone()
        self.assertEqual(1, r['id'])
        self.assertEqual('WARNING', r['severity'])
        self.assertEqual('some-application', r['application'])
        self.assertEqual('charlie.nscl.msu.edu', r['source'])
        self.assertEqual(1234, r['timestamp'])
        self.assertEqual('Something to talk about', r['message'])
    
    # Tests for addRingStatistics.
    
    def test_addRingDef(self):
        self._api.addRingStatistics(
            statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
            'charlie.nscl.msu.edu',
            {'timestamp': 12345, 'name' : 'aring'}
        )
        c = self._sqlite.cursor()
        c.execute('SELECT * FROM ring_buffer')
        r = c.fetchone();
        self.assertEqual(1, r['id'])
        self.assertEqual('aring', r['name'])
        self.assertEqual('charlie.nscl.msu.edu', r['host'])
    def test_addRingClient(self):
        client = {
            'operations' : 100, 'bytes' : 1000, 'producer' : True,
            'command': ['this', 'is', 'the', 'command'], 'backlog' : 0,
            'pid': 123
        }
        clients = [client, ]
        ring = {'timestamp': 12345, 'name' : 'aring'}
        self._api.addRingStatistics(
            statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
            'charlie.nscl.msu.edu', ring, clients
        )
        # Should make an entry in the clients table:
        
        c = self._sqlite.cursor()
        c.execute('SELECT * FROM ring_client')
        r = c.fetchone()
        
        self.assertEqual(1, r['id'])
        self.assertEqual(1, r['ring_id'])
        self.assertEqual(123, r['pid'])
        self.assertEqual(1, r['producer'])
        self.assertEqual('this is the command', r['command'])
        
    def test_addRingStatistics(self):
        self.test_addRingClient()      # also makes a statistics entry:
        
        c = self._sqlite.cursor()
        c.execute('SELECT * FROM ring_client_statistics')
        r = c.fetchone()
        
        self.assertEqual(1, r['id'])
        self.assertEqual(1, r['ring_id'])
        self.assertEqual(1, r['client_id'])
        self.assertEqual(12345, r['timestamp'])
        self.assertEqual(100, r['operations'])
        self.assertEqual(1000, r['bytes'])
        self.assertEqual(0, r['backlog'])
        
    def test_addRingStatRingMbDict(self):
        with self.assertRaises(statusdb.exception) :
            self._api.addRingStatistics(
                statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
                'charlie.nscl.msu.edu', 1)
                                        
    def test_addRingStatRingMissingKeys(self):
        with self.assertRaises(statusdb.exception) :
            self._api.addRingStatistics(
                statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
                'charlie.nscl.msu.edu', {'name': 'aring'}
            )
    def test_addRingStatClientNotiterable(self):
        with self.assertRaises(statusdb.exception):
            self._api.addRingStatistics(
                statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
                'charlie.nscl.msu.edu', {'name': 'aring', 'timestamp' : 12345},
                5
            )
    def test_addRingStatClientMissingClientKey(self):
        client = {
            'operations' : 100, 'bytes' : 1000, 'producer' : True,
            'command': ['this', 'is', 'the', 'command'], 'backlog' : 0,
        }                      # pid missing
        with self.assertRaises(statusdb.exception):
            self._api.addRingStatistics(
                statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
                'charlie.nscl.msu.edu', {'name': 'aring', 'timestamp' : 12345},
                [client,]
            )
            
     #  Test(s) for addStateChange:
     
    def test_addStateChange_app(self):
        ts = int(time.time())
        self._api.addStateChange(
            statusmessages.SeverityLevels.INFO, 'Readout', 'charlie.nscl.msu.edu',
            ts, 'NotReady', 'Readying'
        )
        
        cursor = self._sqlite.cursor()
        cursor.execute('SELECT * FROM state_application')
        r = cursor.fetchone()
        self.assertEqual(1, r['id'])
        self.assertEqual('Readout', r['name'])
        self.assertEqual('charlie.nscl.msu.edu', r['host'])
        
        return ts                # So client texts can use our timestamp.
    
    def test_addStateChange_transition(self):
        ts = self.test_addStateChange_app()
        
        # Should also have produced a state change record:
        
        cursor = self._sqlite.cursor()
        cursor.execute('SELECT * FROM state_transitions')
        r = cursor.fetchone()
        self.assertEqual(1, r['id'])
        self.assertEqual(1, r['app_id'])
        self.assertEqual(ts, r['timestamp'])
        self.assertEqual('NotReady', r['leaving'])
        self.assertEqual('Readying', r['entering'])
        
    # Tests for the addReadoutStatistics wrapper.
    
    def test_addReadoutStats_program(self):
        ts = int(time.time())
        self._api.addReadoutStatistics(
            statusmessages.SeverityLevels.INFO, 'VMUSBReadout',
            'charlie.nscl.msu.edu',  ts, 12, 'This is a run title'
        )
        # Should make an entry in readout_program:
        
        cursor = self._sqlite.cursor()
        cursor.execute('SELECT * FROM readout_program')
        r = cursor.fetchone()
        self.assertEqual(1, r['id'])
        self.assertEqual('VMUSBReadout', r['name'])
        self.assertEqual('charlie.nscl.msu.edu', r['host'])
        
        return ts
    
    def test_addReadoutStats_run(self):
        ts = self.test_addReadoutStats_program()
        
        #  also adds a run definition:
        
        cursor = self._sqlite.cursor()
        cursor.execute('SELECT * FROM run_info')
        r = cursor.fetchone()
        
        self.assertEqual(1, r['id'])
        self.assertEqual(1, r['readout_id'])
        self.assertEqual(ts, r['start'])
        self.assertEqual(12, r['run'])
        self.assertEqual('This is a run title', r['title'])
    
    def test_addReadoutStats_stats(self):
        ts = int(time.time())
        counterinfo = {
            'timestamp': ts + 5, 'elapsed': 5, 'triggers' : 100, 'events' : 99,
            'bytes' : 1111
        }
        self._api.addReadoutStatistics(
            statusmessages.SeverityLevels.INFO, 'VMUSBReadout',
            'charlie.nscl.msu.edu',  ts, 12, 'This is a run title',
            counterinfo
        )
    
        # Should add a stats entry.
        
        fielddict = {
            'timestamp' : 'timestamp', 'elapsed': 'elapsedtime',
            'triggers': 'triggers', 'events': 'events', 'bytes': 'bytes'
        }                  # Maps counterinfo keys to record keys.
        
        cursor = self._sqlite.cursor()
        cursor.execute('SELECT * FROM readout_statistics')
        r  = cursor.fetchone()
        
        self.assertEqual(1, r['id'])
        self.assertEqual(1, r['run_id'])
        self.assertEqual(1, r['readout_id'])
        
        for f in fielddict.keys():
            self.assertEqual(counterinfo[f], r[fielddict[f]])
    
    def test_queryLogMessages_nofilter(self):
        self._api.addLogMessage(
             statusmessages.SeverityLevels.WARNING, 'some-application',
            'charlie.nscl.msu.edu', 1234, 'Something to talk about'
        )
        self._api.addLogMessage(
            statusmessages.SeverityLevels.INFO, 'another-app',
            'spdaq20.nscl.msu.edu', 1555,
            "Every little thing's going to be all right"
        )
        result = self._api.queryLogMessages()
        self.assertEqual(2, len(result))
        self.maxDiff = 1000
        self.assertEqual((
            {
                'id': 1, 'timestamp': 1234, 'message': 'Something to talk about',
                'severity': statusmessages.SeverityLevels.WARNING,
                'application': 'some-application',
                'source': 'charlie.nscl.msu.edu'
            },
            {
                'id': 2, 'timestamp': 1555,
                'message': "Every little thing's going to be all right",
                'severity': statusmessages.SeverityLevels.INFO,
                'application': 'another-app',
                'source': 'spdaq20.nscl.msu.edu'
            }
        ), result)
        
    def test_queryLogMessages_filterlevel(self):
        self.test_queryLogMessages_nofilter()
        filter = where.RelationToString(
            'severity', '=', 'INFO'
        )
        result = self._api.queryLogMessages(filter)
        self.assertEqual(1, len(result))
        self.assertEqual((
           {
                'id': 2, 'timestamp': 1555,
                'message': "Every little thing's going to be all right",
                'severity': statusmessages.SeverityLevels.INFO,
                'application': 'another-app',
                'source': 'spdaq20.nscl.msu.edu'
            },
        ), result)
        
    def test_listRings_nofilter(self):
        self._api.addRingStatistics(
            statusmessages.SeverityLevels.INFO, "ringstatdaemon",
            'charlie.nscl.msu.edu',
            {'timestamp': 12345, 'name': 'ring1'}
        )
        self._api.addRingStatistics(
            statusmessages.SeverityLevels.INFO, "ringstatdaemon",
            "spdaq20.nscl.msu.edu",
            {'timestamp': 12346, 'name': 'ring2'}
        )
        result = self._api.listRings()              # no filter get all.
        
        #  We should have two ringbuffers:
        
        self.assertEqual(2, len(result))
        
        #  They should  look like this:
        
        sb = (
            {
                'id': 1, 'name': 'ring1', 'host': 'charlie.nscl.msu.edu',
                'fqname': 'ring1@charlie.nscl.msu.edu'
            },
            {
                'id': 2, 'name': 'ring2', 'host': 'spdaq20.nscl.msu.edu',
                'fqname': 'ring2@spdaq20.nscl.msu.edu'
            }
            )
        self.assertEqual(sb, result)
    
    def tets_listRings_withFilter(self):
        self.test_listRings_nofilter()          # Stocks database.
        filter = where.RelationToString('r.host', '=', 'charlie@nscl.msu.edu')
        result = self.listRings(filter)
        
        self.assertEqual(1, len(result))
        self.assertEqual((
           {
                'id': 1, 'name': 'ring1', 'host': 'charlie.nscl.msu.edu',
                'fqname': 'ring1@charlie.nscl.msu.edu'
            }, 
        ), result)
    def test_listRingsAndClients_nofilter(self):
        client1 = {
            'operations' : 100, 'bytes' : 1000, 'producer' : True,
            'command': ['this', 'is', 'the', 'command'], 'backlog' : 0,
            'pid': 123
        }
        client2 = {
            'operations' : 150, 'bytes' : 2000, 'producer' : False,
            'command': ['dumper', '--source=tcp://localhost/aring'],
            'backlog' : 650, 'pid': 200           
        }
        clients = [client1, client2]
        ring = {'timestamp': 12345, 'name' : 'aring'}
        self._api.addRingStatistics(
            statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
            'charlie.nscl.msu.edu', ring, clients
        )
        
        ring = {'timestamp': 12400, 'name': 'bring'}
        self._api.addRingStatistics(
            statusmessages.SeverityLevels.INFO, 'ringstatdaemon',
            'spdaq20.nscl.msu.edu', ring, [client1, ]
        )
        
        result = self._api.listRingsAndClients()
        
        self.assertEqual(2, len(result))      # Two dict entries.
        sb = {
            'aring@charlie.nscl.msu.edu' : (
                {
                    'id': 1, 'name': 'aring', 'host': 'charlie.nscl.msu.edu',
                    'fqname': 'aring@charlie.nscl.msu.edu'
                },
                (
                    {
                        'id': 1, 'pid': 123, 'producer': True,
                        'command': 'this is the command'
                    },
                    {
                        'id': 2, 'pid': 200, 'producer': False,
                        'command': 'dumper --source=tcp://localhost/aring'
                    }
                )
            ),
            'bring@spdaq20.nscl.msu.edu': (
                {
                    'id': 2, 'name': 'bring', 'host': 'spdaq20.nscl.msu.edu',
                    'fqname': 'bring@spdaq20.nscl.msu.edu'
                },
                (
                    {
                        'id': 3, 'pid': 123, 'producer': True,
                        'command': 'this is the command'
                    }, 
                )
            )
        }
        self.assertEqual(sb, result)
        
        def test_listRingsAndClients_withfilter(self):
            self.test_listRingsAndClients_nofilter()
            filter = where.RelationToNonString('c.producer', '=', 1)
            result = self_api.listRingsAndClients(filter)         # Only producers.
            
            self.assertEqual(2, len(result))
            sb = {
            'aring@charlie.nscl.msu.edu' : (
                {
                    'id': 1, 'name': 'aring', 'host': 'charlie.nscl.msu.edu',
                    'fqname': 'aring@charlie.nscl.msu.edu'
                },
                (
                    {
                        'id': 1, 'pid': 123, 'producer': True,
                        'command': 'this is the command'
                    },
                )
            ),
            'bring@spdaq20.nscl.msu.edu': (
                {
                    'id': 2, 'name': 'bring', 'host': 'spdaq20.nscl.msu.edu',
                    'fqname': 'bring@spdaq20.nscl.msu.edu'
                },
                (
                    {
                        'id': 3, 'pid': 123, 'producer': True,
                        'command': 'this is the command'
                    }, 
                )
            )
        }
        self.assertEqual(sb, result)
        
        def test_queryRingStatistics_nofilter(self):
            self.test_listRingsAndClients_nofilter()    # Stocks db.
            result = test._api.queryRingStatistics()
            
            #  This is a bit to complicated for me to do a literal so
            #  we'll check the size and then break things down a bit:
            
            self.assertEqual(2, len(result))
            
            self.assertTrue("aring@charlie.nscl.msu.edu" in result.keys())
            self.assertTrue("bring@spdaq20.nscl.msu.edu" in result.keys())
            
            aring = result['aring@charlie.nscl.msu.edu']
            bring = result['bring@spdaq20.nscl.msu.edu']
            
            # Ensure the ring ids are right.
            
            self.assertEqual({
                'name': 'aring', id: 1, 'host': 'charlie.nscl.msu.edu',
                'fqname': 'aring@charlie.nscl.msu.edu'
            }, aring[0])
            self.assertEqual({
                id: 2, 'name': 'bring', 'host': 'spdaq20.nscl.msu.edu',
                'fqname': 'bring@spdaq20.nscl.msu.edu'
            }, bring[0])
            
            aringClients = aring[1]
            bringClients = bring[1]
            
            self.assertEqual(2, len(aRingClients))
            self.assertEqual(1, len(bRingClients))
            
            self.assertEqual(
                (
                    (
                        {
                            'id': 1, 'pid': 123, 'producer': True,
                            'command': 'this is the command'
                        },
                        (
                            {
                                 'id': 1,    'timestamp': 12345,
                                 'operations': 100, 'bytes': 1000, 'backlog': 0
                            },
                        )
                    ),
                    (
                        {
                            'id': 2, 'pid': 200, 'producer': False,
                            'command': 'dumper --source=tcp://localhost/aring'
                        },
                        ({
                            'id': 2, 'timestamp': 12345,
                            'operations': 150, 'bytes': 2000, 'backlog': 650
                        }, )
                    )
                ), aRingClients
            )
            self.assertEquals(
            (
                (
                    {
                        'id': 3, 'pid': 123, 'producer': True,
                        'command': 'this is the command'
                    },
                    (
                        {
                            'id': 3, 'operations': 100, 'bytes':1000,
                            'backlog':0
                        },
                    )
                ),
            )
            , bRingClients 
            )
            
    def test_listStateApps_nofilter(self):
        ts = int(time.time())
        self._api.addStateChange(
            statusmessages.SeverityLevels.INFO, "Readout", 'charlie.nscl.msu.edu',
            ts, 'NotReady', 'Readying'
        )
        self._api.addStateChange(
            statusmessages.SeverityLevels.WARNING, "VMUSBReadout", 'spdaq20.nscl.msu.edu',
            ts, 'NotReady', 'Readying'
        )
        self._api.addStateChange(
            statusmessages.SeverityLevels.WARNING, "VMUSBReadout", 'spdaq20.nscl.msu.edu',
            ts+1, 'Readying', 'Ready'
        )
        self._api.addStateChange(
            statusmessages.SeverityLevels.INFO, "Readout", 'charlie.nscl.msu.edu',
            ts+2, 'Readying', 'Ready'
        )
        
        result = self._api.listStateApplications()
        
        self.assertEqual(2, len(result))
        self.assertEqual(
            (
                {'id': 1, 'name': 'Readout', 'host': 'charlie.nscl.msu.edu'},
                {'id': 2, 'name': 'VMUSBReadout', 'host': 'spdaq20.nscl.msu.edu'}
            ), result
        )
        return ts
    
    def test_listStateApps_filter(self):
        ts = self.test_listStateApps_nofilter()
        filter = where.RelationToString('a.host', '=', 'charlie.nscl.msu.edu')
        
        result = self._api.listStateApplications(filter)
        self.assertEqual(1, len(result))
        self.assertEqual(
            (
                {'id': 1, 'name': 'Readout', 'host': 'charlie.nscl.msu.edu'},
            ), result
        )

    def test_queryStateTransitions_nofilter(self):
        ts = self.test_listStateApps_nofilter()   # Stocks the database.
        result = self._api.queryStateTransitions()
        
        self.assertEqual(4, len(result))
        self.maxDiff = 2000
        self.assertEqual((
            {
                'application':  {'id': 1, 'name': 'Readout', 'host': 'charlie.nscl.msu.edu'},
                'appid': 1, 'transitionId': 1, 'timestamp': ts,
                'leaving': 'NotReady', 'entering': 'Readying'
            },
            {
                'application':  {'id': 2, 'name': 'VMUSBReadout', 'host': 'spdaq20.nscl.msu.edu'},
                'appid': 2, 'transitionId': 2, 'timestamp': ts,
                'leaving': 'NotReady', 'entering': 'Readying'
            },
            {
                'application':  {'id': 2, 'name': 'VMUSBReadout', 'host': 'spdaq20.nscl.msu.edu'},
                'appid': 2, 'transitionId': 3, 'timestamp': ts+1,
                'leaving': 'Readying', 'entering': 'Ready'
            },
            {
                'application': {'id': 1, 'name': 'Readout', 'host': 'charlie.nscl.msu.edu'},
                'appid': 1, 'transitionId': 4, 'timestamp': ts+2,
                'leaving': 'Readying', 'entering': 'Ready'
            }
            ), result)
        
    def test_queryStateTransitions_filter(self, ):
        ts = self.test_listStateApps_nofilter()
        filter = where.RelationToNonString('t.timestamp', '>', ts)
        result = self._api.queryStateTransitions(filter)
        
        self.assertEqual(2, len(result))
        self.assertEqual(
            (
                {
                'application':  {'id': 2, 'name': 'VMUSBReadout', 'host': 'spdaq20.nscl.msu.edu'},
                'appid': 2, 'transitionId': 3, 'timestamp': ts+1,
                'leaving': 'Readying', 'entering': 'Ready'
            },
            {
                'application': {'id': 1, 'name': 'Readout', 'host': 'charlie.nscl.msu.edu'},
                'appid': 1, 'transitionId': 4, 'timestamp': ts+2,
                'leaving': 'Readying', 'entering': 'Ready'
            }
            ), result
        )
    def test_listReadoutApps_nofilter(self):
        ts = int(time.time())
        self._api.addReadoutStatistics(
            statusmessages.SeverityLevels.INFO, 'VMUSBReadout',
            'charlie.nscl.msu.edu',  ts, 12, 'This is a run title'
        )
        self._api.addReadoutStatistics(
            statusmessages.SeverityLevels.INFO, 'Readout',
            'spdaq20.nscl.msu.edu', ts+1, 12, 'This is a run title on spdaq20'
        )
        self._api.addReadoutStatistics(
            statusmessages.SeverityLevels.INFO, 'CCUSBReadout',
            'spdaq19.nscl.msu.edu', ts+2, 12, 'This is a run title on spdaq19'
        )
        result = self._api.listReadoutApps()
        
        self.assertEqual(3, len(result))
        self.assertEqual(
            (
                {'id': 1, 'name' : 'VMUSBReadout', 'host': 'charlie.nscl.msu.edu'},
                {'id': 2, 'name' : 'Readout', 'host': 'spdaq20.nscl.msu.edu'},
                {'id': 3, 'name' : 'CCUSBReadout', 'host': 'spdaq19.nscl.msu.edu' }
            ), result
        )
        return ts
    def test_listReadoutApps_filter(self):
        ts = self.test_listReadoutApps_nofilter()
        filter = where.InFilter('a.id')
        filter.addNumber(2)
        filter.addNumber(3)
        result = self._api.listReadoutApps(filter)
        self.assertEqual(2, len(result))
        self.assertEqual(
            (
                {'id': 2, 'name' : 'Readout', 'host': 'spdaq20.nscl.msu.edu'},
                {'id': 3, 'name' : 'CCUSBReadout', 'host': 'spdaq19.nscl.msu.edu'}
            ), result
        )
    def test_listRuns_nofilter(self):
        ts = self.test_listReadoutApps_nofilter()
        result = self._api.listRuns()
        
        self.assertEqual(3, len(result))
        self.assertEqual([1,2,3], result.keys())
        
        self.assertEqual(
            {
                1: (
                    {'id': 1, 'name' : 'VMUSBReadout', 'host': 'charlie.nscl.msu.edu'},
                    (
                        { 'id': 1, 'number': 12,
                         'title': 'This is a run title', 'start': ts}
                        ,
                    )
                   ),
                2: (
                    {'id': 2, 'name' : 'Readout', 'host': 'spdaq20.nscl.msu.edu'},
                    (
                      { 'id': 2, 'number': 12,
                       'title': 'This is a run title on spdaq20', 'start': ts + 1}
                        ,
                    )
                   ),
                3: (
                    {'id': 3, 'name' : 'CCUSBReadout', 'host': 'spdaq19.nscl.msu.edu' },
                    (
                        { 'id': 3, 'number': 12,
                       'title': 'This is a run title on spdaq19', 'start': ts + 2}
                        ,
                    )
                   )
            }, result
        )
    def test_listRuns_filter(self):
        ts = self.test_listReadoutApps_nofilter()
        filter = where.RelationToString('a.host', 'LIKE', 'spdaq%')
        result = self._api.listRuns(filter)
        
        self.assertEqual(2, len(result))
        self.assertEqual(
            {
                2: (
                    {'id': 2, 'name' : 'Readout', 'host': 'spdaq20.nscl.msu.edu'},
                    (
                      { 'id': 2, 'number': 12,
                       'title': 'This is a run title on spdaq20', 'start': ts + 1}
                        ,
                    )
                   ),
                3: (
                    {'id': 3, 'name' : 'CCUSBReadout', 'host': 'spdaq19.nscl.msu.edu' },
                    (
                        { 'id': 3, 'number': 12,
                       'title': 'This is a run title on spdaq19', 'start': ts + 2}
                        ,
                    )
                   )
            }, result)
    
    def test_queryReadoutStats_nofilter(self):
        #  Stock the apps and runs:
        
        ts = self.test_listReadoutApps_nofilter()
        
        #  Now some statistics entries for each app's run 1.
        
if __name__ == '__main__':
    unittest.main()
