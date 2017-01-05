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
        self._rowFact = None
        
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



if __name__ == '__main__':
    unittest.main()
