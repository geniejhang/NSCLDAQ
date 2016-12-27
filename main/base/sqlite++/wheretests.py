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
# @file   wheretests.py
# @brief  tests for the nscldaq.sqlite.where package
# @author <fox@nscl.msu.edu>

from nscldaq.sqlite import where
import unittest

class WhereTests(unittest.TestCase):
    def testRaw(self):
        rawstring = 'a = b'
        f = where.RawFilter(rawstring)
        self.assertEquals(rawstring, f.toString())
        
    def test_RelationToNonString(self):
        f = where.RelationToNonString(
            'a', where.BinaryRelation.notequal, 1234
        )
        self.assertEquals('a <> 1234', f.toString())
    
    def test_RelationToString(self):
        f = where.RelationToString(
            'a', where.BinaryRelation.ge, 'astring'
        )
        self.assertEqual("a >= 'astring'", f.toString())

    def test_AndFilter(self):
        f = where.AndFilter()
        f1 = where.RelationToNonString(
            'a', where.BinaryRelation.gt, 1)
        f2 = where.RelationToNonString(
            'b', where.BinaryRelation.equal, 2)
        f.addClause(f1)
        f.addClause(f2)
        self.assertEqual(
            '((a > 1) AND (b = 2))', f.toString()
        )
    def test_OrFilter(self):
        f = where.OrFilter()
        f1 = where.RelationToNonString(
            'a', where.BinaryRelation.gt, 1)
        f2 = where.RelationToNonString(
            'b', where.BinaryRelation.equal, 2)
        f.addClause(f1)
        f.addClause(f2)
        self.assertEqual(
            '((a > 1) OR (b = 2))', f.toString()
        )
    def test_InFilter(self):
        f = where.InFilter('a')
        f.addString('abcd')
        f.addNumber(1234)
        self.assertEqual("a IN ('abcd', 1234)", f.toString())
        
if __name__ == '__main__':
    unittest.main()


