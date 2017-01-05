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

from nscldaq.status import statusdb
from nscldaq.status  import statusmessages

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
            
            
            
if __name__ == '__main__':
    unittest.main()
