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


from PyQt5.QtWidgets import (QWizard, QApplication, QWizardPage,
    QLabel, QLineEdit, QPushButton, QFileDialog,
    QVBoxLayout, QHBoxLayout
)


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
#
#  Wizard pages:

class SourceDestPage(QWizardPage):
    def __init__(self, *args):
        #
        #  We need line editors for
        #  The Ring URi and the Destination directory
        #  ANd a nice bit of instructions.
        #
        
        super().__init__(*args)
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
Welcome to the event logger creation wizard.<br />
<p>
This wizard will lead you through creating an event logger
in the managed experiment environment.
</p>
<p>
Each eventlogger has a source ringbuffer and a destination
directory.  On this page you will select those.  The
ringbuffer must be specified as URI to a ringbuffer that
will exist when this logger starts logging data.  For example
</p>
<tt>tcp://spdaq12/myring</tt>
<p>
The destination must be a directory path that is valid within
the container the event logger will run in on the host in which
it will run.
</p>

            '''
)
        layout = QVBoxLayout()
        layout.addWidget(self._instructions)
        
        #  Now the things needed to prompt for the source ring:
        
        srclayout = QHBoxLayout()
        self._srclabel = QLabel('Source [URI]', self)
        self._src      = QLineEdit('tcp://', self)
        srclayout.addWidget(self._srclabel)
        srclayout.addWidget(self._src)
        
        layout.addLayout(srclayout)
        
        # Now the things needed to prompt for the destination:
        
        destlayout =  QHBoxLayout()
        self._destlabel = QLabel('Destination (directory)', self)
        self._dest      = QLineEdit(self)
        self._destbrowse = QPushButton("Browse...", self)
        destlayout.addWidget(self._destlabel)
        destlayout.addWidget(self._dest)
        destlayout.addWidget(self._destbrowse)
        
        layout.addLayout(destlayout)
        
        self.setLayout(layout)
        
        #   Support browsing the destination directory:
        
        self._destbrowse.clicked.connect(self._browsedestination)

    # Selectors:
    
    def source(self):
        return self._src.text()
    def destination(self):
        return self._dest.text()
        
    def _browsedestination(self):
        browser = QFileDialog(self)
        browser.setAcceptMode(QFileDialog.AcceptOpen)
        browser.setFileMode(QFileDialog.Directory)
        browser.exec()
        
        dest = browser.selectedFiles()
        if len(dest) != 0:
            self._dest.setText(dest[0])
        


class EvlogWizard(QWizard):
    def __init__(self):
        super().__init__()
        self._srcdestpg = SourceDestPage(self)
        self.addPage(self._srcdestpg)
    
    def source_url(self):
        return self._srcdestpg.source()
    def destination_dir(self):
        return self._srcdestpg.destination()
#---------------------------------------------------------------------
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
wizard = EvlogWizard()
wizard.show()
app.exec()

print('Source', wizard.source_url())
print('Destination', wizard.destination_dir())