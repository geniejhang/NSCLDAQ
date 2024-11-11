#! /usr/bin/env python3
"""This program provides the main program for the new control panel of the
    Managed experiment control panel.
"""

from nscldaq.manager_control.config import Configuration
from nscldaq.manager_control.programlist import ProgramView
from nscldaq.manager_control.cfgwizard import ConfigWizard
from nscldaq.manager_control.maingui import MainGui
from nscldaq.manager_client import Programs
from PyQt5.QtWidgets import (
    QApplication, QLabel, QLineEdit,
    QMainWindow
)
from PyQt5.QtCore import QTimer
import sys
import os


CONFIGURATION_FILE="mg_guiconfig.toml"
UPDATE_INTERVAL=2        # Units are sedonds

Updater = None          # Early in initialization this becomes a QTimer. Updaters hook to the timeout signal.



def get_configuration():
    global CONFIGURATION_FILE
    #   If the file exists we just use it.  Otherwise we must use a widget to instantiate it.
    
    if os.access(CONFIGURATION_FILE, os.R_OK):
        return Configuration(CONFIGURATION_FILE)
    else:
        c = Configuration ('/dev/null')   # All defaults.
        wiz = ConfigWizard(c)
        wiz.exec()
        c.dump(CONFIGURATION_FILE)
        return c


def addProgramDisplay(window):
    programList = ProgramView()
    tabs = window.tabs()
    tabs.addTab(programList, 'Programs')
    return programList
    

def updateProgramsTab():
    programs = Programs(config.host(), config.user(), config.rest_service())
    print('updating')
    programlist.model().update(programs.status())

#---------------------------------- Entry point -----------------------
#


app = QApplication(sys.argv)
Updater = QTimer()
Updater.start(UPDATE_INTERVAL*1000)

# Updater is a timer that can be hooked into to do periodic updates... It will run every
# UPDATE_INTERVAL seconds.

# We need to either read or create the configuration:

config = get_configuration()

# Now the GUI:

main_window = QMainWindow()

gui = MainGui()
programlist = addProgramDisplay(gui)
filter = config.readouts()
print("filtering to ", filter)
programlist.model().setFilter(config.readouts())
Updater.timeout.connect(updateProgramsTab)
main_window.setCentralWidget(gui)

main_window.show()

app.exec()