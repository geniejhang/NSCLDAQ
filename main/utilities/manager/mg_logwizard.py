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
    QLabel, QLineEdit, QPushButton, QFileDialog, QComboBox,
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
        

class WherePage(QWizardPage):
    def __init__(self, *args):
        global db     # Database handle.
        #  Choose where the wizard runs.
        # this is the host as well as the container.
        #
        super().__init__(*args)
        
        # Instructions:
        
        layout = QVBoxLayout()
        
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
In this page we'll select where the event logger runs (its host computer)
and in which container on that system it will be started.

            '''
        )
        layout.addWidget(self._instructions)
        
        # The host:
        
        hostlayout = QHBoxLayout()
        self._hostlabel = QLabel('Host TCP/IP address: ', self)
        self._host      = QLineEdit(self)
        hostlayout.addWidget(self._hostlabel)
        hostlayout.addWidget(self._host)
        
        layout.addLayout(hostlayout)
        
        
        # Enumerate the containers:
        
        c = Container(db)
        containers = [x['name'] for x in c.list()]
        
        containerlayout = QHBoxLayout()
        self._containerlbl = QLabel('Select container: ', self)
        self._container = QComboBox(self)
        self._container.addItems(containers)
        
        containerlayout.addWidget(self._containerlbl)
        containerlayout.addWidget(self._container)
        
        
        layout.addLayout(containerlayout)
        
        
        
        self.setLayout(layout)
        
    # Selectors:
    
    def host(self):
        return self._host.text()
    def container(self):
        return self._container.currentText()
        
class DAQRootPage(QWizardPage):
    def __init__(self,*args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
            <p>
            On this page you will select the Root of the version of FRIBDAQ whose event
            log program will be run to record data.  Note that the directory you choose must 
            be valid inside the container  in which it will run.  In some cases this means
            you won't be able to browse to the directory.  
            </p>
            <p>
            However, If your init script for that container usees <tt>daqsetup.bash</tt>
            to define the environment variables for a version of FRIBDAQ, you may use
            those environment variables ($DAQROOT e.g.) for the DAQ root.
            </p>
            '''
        )
        layout.addWidget(self._instructions)
        
        rootlayout = QHBoxLayout()
        self._rootlbl = QLabel('DAQ Root Dir:', self)
        self._root    = QLineEdit('$DAQROOT', self)
        self._browseroot = QPushButton('Browse...', self)
        rootlayout.addWidget(self._rootlbl)
        rootlayout.addWidget(self._root)
        rootlayout.addWidget(self._browseroot)
        layout.addLayout(rootlayout)
        
        self.setLayout(layout)
        
        self._browseroot.clicked.connect(self._browse)
        
    def _browse(self):
        browser = QFileDialog(self)
        browser.setAcceptMode(QFileDialog.AcceptOpen)
        browser.setFileMode(QFileDialog.Directory)
        browser.setDirectory('/usr/opt/daq')
        browser.exec()
        
        root = browser.selectedFiles()
        if len(root) != 0:
            self._root.setText(root[0])
    
    def root(self):
        return self._root.text()    
        
class OptionPage(QWizardPage):
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        
        # The instructions:
        
        
class EvlogWizard(QWizard):
    def __init__(self):
        super().__init__()
        self._srcdestpg = SourceDestPage(self)
        self.addPage(self._srcdestpg)
        
        self._where = WherePage(self)
        self.addPage(self._where)
        
        self._root = DAQRootPage(self)
        self.addPage(self._root)
    
    def source_url(self):
        return self._srcdestpg.source()
    def destination_dir(self):
        return self._srcdestpg.destination()
    
    def host(self):
        return self._where.host()
    def container(self):
        return self._where.container()
    def daqroot(self):
        return self._root.root()
    
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
print('Host', wizard.host())
print('Container', wizard.container())
print('DAQROOT ', wizard.daqroot())