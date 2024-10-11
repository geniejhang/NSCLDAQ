#! /usr/bin/env python3

'''
  Main program of the event log wizard.
  
  Usage;
     $DAQBIN/logwizard database-file
     
'''
import sys
import os
import sqlite3
from nscldaq.mg_database import Container, EventLog


from PyQt5.QtWidgets import (QWizard, QApplication, QWizardPage)


def Usage():
    #  Print program usage to stderr:
    
    sys.stderr.write(
        '''
Usage:
    $DAQABIN/logwizard  config-file
    
Where:
   config-file - is the path to the configuration database file.
        '''
    )

if len(sys.argv) != 2:
    Usage()
    sys.exit(-1)

dbfile = sys.argv[1]
if not os.path.exists(dbfile):
    sys.stderr.write(
        f'{dbfile} does not exist.  Use $DAQBIN/mg_mkconfig to create it.'
    )
    sys.exit(-1)
    
db = sqlite3.connect(dbfile)


app = QApplication(sys.argv)
wizard = QWizard()
wizard.show()
app.exec()