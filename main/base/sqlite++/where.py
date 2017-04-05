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
# @file   where.py
# @brief  Implement nscldaq.sqlite.where package.
# @author <fox@nscl.msu.edu>


##
# This package provides a set of classes that can be used to build SQLITE
# query WHERE clauses without any knowledge of SQL.
#

##
# base class of all of the filter classes.  Provides the need for
# subclasses to implement a toString method that emits SQL WHERE clauses.
#

class QueryFilter(object):
    def toString(self):
        pass


#  Query whose string is passed unaltered (user knows how to formulate WHERE
# clauses).
class RawFilter(QueryFilter) :
    def __init__(self, whereClause) :
        self.whereClause = whereClause
    def toString(self):
        return self.whereClause

# binaryRelation - a relation of the form field op thing
#
class BinaryRelation(QueryFilter):
    equal    = '='
    notequal = '<>'
    gt       = '>'
    ge       = '>='
    lt       = '<'
    le       = '<='
    def __init__(self, field, op, thing):
        self._field = field
        self._op    = op
        self._thing = thing
    def toString(self):
        return self._field + ' ' + self._op +  ' ' + str(self._thing)

    
class RelationToNonString(BinaryRelation):
    def __init__(self, field, op, thing):
        super(RelationToNonString, self).__init__(field, op, thing)
        
class RelationToString(BinaryRelation):
    def __init__(self, field, op, thing):
        super(RelationToString, self).__init__(field, op, "'%s'" % thing)
        
##
#  These classes are compound in that they involve a list of several items.
#  The base class assumes the items are QueryFilter objects, but a getter
#  is provided for the items allowing other tpes of items (e.g. IN) to
#  be used.
#

class CompoundFilter(QueryFilter): 
    def __init__(self,  op):
        super(CompoundFilter, self).__init__()
        self._op    = op
        self._items = list()
        
    def addClause(self, newItem):
        self._items.append(newItem)
        
    def toString(self):
        itemList = list()
        for item in self._items:
            itemList.append('(' + item.toString() + ')')
        return '(' + (' ' + self._op + ' ').join(itemList) + ')'
        
class AndFilter(CompoundFilter):
    def __init__(self):
        super(AndFilter, self).__init__('AND')

class OrFilter(CompoundFilter):
    def __init__(self):
        super(OrFilter,self).__init__('OR')

class InFilter(QueryFilter):
    def __init__(self, field):
        super(InFilter, self).__init__()
        self._items = list()
        self._field = field
        
    def addString(self, string):
        self._items.append("'" + string + "'")
    def addNumber(self, number):
        self._items.append(str(number))
    def toString(self):
        items = '(' + ', '.join(self._items) + ')'
        return self._field + ' IN ' + items
    