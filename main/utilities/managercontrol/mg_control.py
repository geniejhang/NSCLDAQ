#! /usr/bin/env python3
"""This program provides the main program for the new control panel of the
    Managed experiment control panel.
"""

from nscldaq.manager_control.config import Configuration
from nscldaq.manager_control.cfgwizard import ConfigWizard
from nscldaq.manager_control.maingui import MainGui
from PyQt5.QtWidgets import (
    QApplication, QLabel, QLineEdit,
    QMainWindow
)
import sys
import os


CONFIGURATION_FILE="mg_guiconfig.toml"




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


#---------------------------------- Entry point -----------------------
#


app = QApplication(sys.argv)

# We need to either read or create the configuration:

config = get_configuration()

# Now the GUI:

main_window = QMainWindow()

gui = MainGui()
main_window.setCentralWidget(gui)

main_window.show()

app.exec()