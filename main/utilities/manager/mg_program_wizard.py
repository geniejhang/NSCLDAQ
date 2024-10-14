#!/usr/bin/env python3 
'''
This script provides a Qt5 wizard for making 
programs.  Programs have:

A name, a host, a container, and an executable.

They have a type which might be:
e.g. 'Critical.

They have an execution environment that consists of:

A working directory, environment variables, an initialization script.

They are parameterized by options which typically (but need not) have values.
And parameters which are values.






Usage:
   $DAQBIN/mg_program_wizard database-file

'''
import sys
import sqlite3
from nscldaq.mg_database import Program, Container
from pathlib import Path

from PyQt5.QtWidgets import (
    QWizard, QApplication, QWizardPage, QLineEdit, QPushButton, QFileDialog,
    QComboBox, QLabel,
    QHBoxLayout, QVBoxLayout
)

from PyQt5.Qt import *


#--------------------------------------------------------------------------------------------
#  GUI code:


class IdentificationPage(QWizardPage):
    def __init__(self, db, *args):
        super().__init__(*args)
        self._db = db
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
Welcome to the program creation wizard.<br/>
<p>
   In the managed data acquisition, programs are executable code, or
scripts that run in some host in a container.   This wizard will lead you
through the creation of a program.  Note that mg_config  features a program
editor that you can use to modify the program, once created or create new programs,
once you become a bit more confident in the process.
</p>
<p>
    On this page you will select
    <ul>
    <li>A unique name by which the program will be referred to within the system.</li>
    <li>The executable for the program. Note the path to this executable must be valid within
    the host/container in which it runs. </li>
    <li>host in which the program runs</li>
    <li>The container in which the program runs in that host</li>
    </ul>
</p>
<hr/>
            '''
        )
        
        layout = QVBoxLayout()
        layout.addWidget(self._instructions)
        
        # Name and program image:
        
        id = QHBoxLayout()
        
        self._namelbl = QLabel('Program Name:', self)
        id.addWidget(self._namelbl)
        self._name = QLineEdit(self)
        id.addWidget(self._name)
        
        self._pgrmlbl = QLabel('Executable:', self)
        id.addWidget(self._pgrmlbl)
        self._pgm = QLineEdit(self)
        id.addWidget(self._pgm)
        self._pgmchooser = QPushButton('Browse...',self)
        id.addWidget(self._pgmchooser)
        
        layout.addLayout(id)
        
        host = QHBoxLayout()
        self._hostlbl = QLabel('Host IP:', self)
        host.addWidget(self._hostlbl)
        self._hostname = QLineEdit(self)
        host.addWidget(self._hostname)
        layout.addLayout(host)
        
        c = Container(self._db)
        containers = [x['name'] for x in c.list()]
        container = QHBoxLayout()
        self._containerlbl = QLabel('Container:', self)
        container.addWidget(self._containerlbl)
        self._container = QComboBox(self)
        self._container.addItems(containers)
        container.addWidget(self._container)
        
        
        
        layout.addLayout(container)
        
        self.setLayout(layout)


class ProgramWizard(QWizard):
    def __init__(self, db):
        super().__init__()

        self._ident = IdentificationPage(db, self)
        self.addPage(self._ident)


#-------------------------------------------------------------------------------------------


#
def Usage():
    sys.stderr.write(
        '''
Usage:
    $DAQBIN/mg_program_wizard config-file-path
Where:
    config-file-path - is the path to the configuration file to edit.
        '''
    )
    sys.exit(-1)
    
    
    

#  Entry point:

if len(sys.argv) != 2:
    Usage()
    
config = sys.argv[1]
p = Path(config)
if not p.exists():
    sys.stderr.write(f'No such config file "{config}"\n')
    exit(-1)
    
    
db = sqlite3.connect(config)

# Do the wizard:

app = QApplication(sys.argv)
wizard = ProgramWizard(db)
wizard.show()
app.exec()

