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
    QComboBox, QLabel, QTableWidget,
    QHBoxLayout, QVBoxLayout
)

from PyQt5.Qt import *


#--------------------------------------------------------------------------------------------
#  GUI code:


class EditableTable(QWidget):
    #  Megawidget that has a table that can be edited.
    #  Editing consists of supported deletion of the current row,
    #  Adding rows to the end, and editing the entries.
    # 
    #  Layout is:
    #   
    #    +--------------------------+
    #    |    The table             | [+]
    #    |                          | [x]
    #    +--------------------------+
    #
    #   The table() method returns the table widget.
    #   this allows for things like setting the headers, the # of colomns
    #   loading it etc.
    #
    def __init__(self, *args):
        super().__init__(*args)
        layout = QHBoxLayout()
        
        self._table = QTableWidget(self)
        layout.addWidget(self._table)
        
        #  The buttons:
        
        buttonlayout = QVBoxLayout()
        self._addrow = QPushButton('+', self)
        buttonlayout.addWidget(self._addrow)
        self._deleterow = QPushButton(self)
        self._deleterow.setIcon(self.style().standardIcon(getattr(QStyle, 'SP_DialogCancelButton')))
        buttonlayout.addWidget(self._deleterow)
        
        
        layout.addLayout(buttonlayout)
        
        self.setLayout(layout)    
        self._addrow.clicked.connect(self._addRow)
        self._deleterow.clicked.connect(self._deleteRow)
        
    def _addRow(self):
        self._table.setRowCount(self._table.rowCount() + 1)
    def _deleteRow(self):
        row = self._table.currentRow()
        self._table.removeRow(row)
        
    def table(self):
        return self._table

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
        
        self._pgmchooser.clicked.connect(self._browse)
    
    def _browse(self):
        #  Browse for a program executable.
        
        dialog = QFileDialog(self)
        dialog.setAcceptMode(QFileDialog.AcceptOpen)
        dialog.setFileMode(QFileDialog.AnyFile)
        dialog.exec()
        
        dest = dialog.selectedFiles()
        if len(dest) != 0:
            self._pgm.setText(dest[0])
    
    #  Accessors:
    
    def program(self):
        return self._pgm.text()
    def name(self):
        return self._name.text()
    def host(self):
        return self._hostname.text()
    def container(self):
        return self._container.currentText()    
        

class TypeAndWdPage(QWizardPage):
    def __init__(self, db, *args):  
        super().__init__(*args)
        #
        #  Get the list of program types:
        #
        p = Program(db)
        types = p.types()
        
        layout = QVBoxLayout()
        
        #  The instructions:
        
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        # Note that program types below must be updated if additional types are added
        self._instructions.setText(
            '''
            <p> On this page we will set the working directory in effect when the program
            starts.  That directory must be a valid directory in the container in which the
            program is running.  We will also specify the program type.
            </p>
            <p>
              Program types are used to describe to the system what you want to happen
              if the program exits.  The following types are defined 
            </p>
            <ul>
            <li><b>Critical</b> - Critical programs are considered essential to the data acquisition
            system.  If a critical program exits, the experiment shuts down and has to be rebooted.
            For example, an event builder data sourcde is typically critical.
            </li>
            <li><b>Persistent</b> - Persistent programs are not critical but are expected to run for the
            life of the experiment.  If a critical program exits, a message will be emitted to the 
            output window <em>but</em> the experiment continues to run.  For example, a histogrammer
            failure does not need to shutdown the experiment.  Typically the current run can be cleanly
            ended and then the experiment rebooted to restart it, or a histogrammer external to the managed
            system can be attached to the data flow (interactively) and the experiment can continue
            to run until it's convenient to restart everything.
            </li>
            <li><b>Transitory</b> - Transitory programs are expected to run for some short time and then
            exit.  WHen they exit no special action is taken by the experiment manager. An example of a 
            transitory program are the programs that interact with readout programs to tell them to start and
            stop data taking.  These are clients of the Readout's REST plugin and simply send a REST request
            to an associated Readout to tell them to do something and then exit.
            </li>
            </ul>
            '''
        )
        layout.addWidget(self._instructions)
        
        #  Current working directory has a browse operation:
        
        wdlayout = QHBoxLayout()
        self._wdlabel = QLabel('Working Directory: ', self)
        wdlayout.addWidget(self._wdlabel)
        
        self._wd     = QLineEdit(self)
        wdlayout.addWidget(self._wd)
        
        self._wdbrowse = QPushButton('Browse...', self)
        wdlayout.addWidget(self._wdbrowse)
        
        layout.addLayout(wdlayout)
        
        # Combobox to select the program type:
        
        typelayout = QHBoxLayout()
        self._typelbl = QLabel('Program Type: ', self)
        typelayout.addWidget(self._typelbl)
        self._type = QComboBox(self)
        self._type.addItems(types)
        typelayout.addWidget(self._type)
        
        layout.addLayout(typelayout)
        
        
        self.setLayout(layout)
    # Accessors:
    def program_type(self):
        return self._type.currentText()
    def wd(self):
        return self._wd.text()
        
class IniScriptAndOptions(QWizardPage):
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
            <p>
                Now we will set up the way the program is called and its environment.
                is started and its environment.
            </p>
            <ul>
                <li>
                    <b>initscriptK/b> - This is an optional shell script that is run prior
                     to starting your program.   This can be used to perform complex setup operations
                     or source scripts containing environment variables like a <tt>daqsetup.bash</tt>
                     script.  Note that the script file path is interpreted in the context of this
                     wizard and the contents of that file are sucked into the configuration databse.
                </li>
                <li>
                    <b>options</b> - These are pairs of command parameters that are passed to the program on 
                    its invocation command line.  For example <tt>--sourceid 0</tt> would be an option
                    named <tt>--sourceid</tt> with mthe value <tt>0</tt>
                </li>
                <li>
                    <b>parameters</b> - These are command line parameters.  For example the names of files
                    needed by the program.  If you are passing file paths to a program they must make sense in
                    the contenxt of the container and host in which you have chosen to run the program.
                </li>
                <li>
                    <b>environmen</b>  - The program environment is a set of name/value pairs.
                    The <tt>getenv(3)</tt> library function can be used to retrieve the values of
                    an environment variable.  Each environment variable has a name and a value, for
                    example <tt>DAQROOT</tt> might have the value <tt>/usr/opt/daq/12.1-003</tt>
                </li>
            </ul>
            <p>
            This page and subsequent ones will allow you to specify these bits of the program environment.
            </p>
            '''
        )
        layout.addWidget(self._instructions)
        
        iniscript = QHBoxLayout()
        self._iniscrlabel = QLabel('Init script file: ', self)
        iniscript.addWidget(self._iniscrlabel)
        
        self._initscript = QLineEdit(self)
        iniscript.addWidget(self._initscript)
        
        self._inibrowse = QPushButton('Browse...', self)
        iniscript.addWidget(self._inibrowse)
        layout.addLayout(iniscript)
        
        self._optlabel = QLabel('Program options', self)
        layout.addWidget(self._optlabel)
        self._options = EditableTable(self)
        self._optiontable = self._options.table()
        self._optiontable.setColumnCount(2)
        self._optiontable.setRowCount(1)
        self._optiontable.setHorizontalHeaderLabels(['Option', 'Value'])
        layout.addWidget(self._options)
        
        
        self.setLayout(layout)
class ParameterPage(QWizardPage):
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QVBoxLayout()
        
        self._instructions = QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            '''
            <p>
                Fill in the table below with the ordered list of parameters that will
                be added to the program command line after the options filled in on the
                previous page.
            </p>
            '''
        )
        layout.addWidget(self._instructions)
        
        self._paramlabel = QLabel('Parameters:', self)
        layout.addWidget(self._paramlabel)
        
        self._paramtable = EditableTable(self)
        self._parameters = self._paramtable.table()
        self._parameters.setColumnCount(1)
        self._parameters.setRowCount(1)
        self._parameters.setHorizontalHeaderLabels(['Parameter',])
        layout.addWidget(self._paramtable)
        
        self.setLayout(layout)
    
class Environment(QWizardPage):    
        def __init__(self, *args):
            super().__init__(*args)
            layout = QVBoxLayout()
            
            self._instructions = QLabel(self)
            self._instructions.setWordWrap(True)
            self._instructions.setText(
                '''
                <p>
                Fill in the table below with environment variable names and their values.
                </p>
                '''
            )
            layout.addWidget(self._instructions)
            
            self._envlabel = QLabel('Environment: ', self)
            layout.addWidget(self._envlabel)
            self._envtable = EditableTable(self)
            self._env = self._envtable.table()
            self._env.setColumnCount(2)
            self._env.setRowCount(1)
            self._env.setHorizontalHeaderLabels(['Variable', 'Value'])
            layout.addWidget(self._env)
            
            self.setLayout(layout)
            
        
class ProgramWizard(QWizard):
    def __init__(self, db):
        super().__init__()

        self._ident = IdentificationPage(db, self)
        self.addPage(self._ident)
        
        self._wdtype = TypeAndWdPage(db, self)
        self.addPage(self._wdtype)
        
        self._iniopts = IniScriptAndOptions(self)
        self.addPage(self._iniopts)
        
        self._params = ParameterPage(self)
        self.addPage(self._params)

        self._env = Environment(self)
        self.addPage(self._env)
    
    # Accessors:
    
    def program(self):
        return self._ident.program()
    def name(self):
        return self._ident.name()
    def host(self):
        return self._ident.host()
    def container(self):
        return self._ident.container()
    
    def program_type(self):
        return self._wdtype.program_type()
    def wd(self):
        return self._wdtype.wd()
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
    
    
    
# Handle cancel or premature close:

def abort():
    exit(0)

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
wizard.rejected.connect(abort)
app.exec()

executable = wizard.program()
name       = wizard.name()
host       = wizard.host()
container   = wizard.container()
type      = wizard.program_type()
cwd        = wizard.wd()

print(f'Name: {name} is {executable} will run in {container}@{host}')
print(f'Program is {type}, and will run with cwd {cwd}')
