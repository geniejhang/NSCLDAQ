"""This file provides the elements of the main gui.  The main gui consist of a top part,
which contains the title, run number, and buttons for run control and event recording.
[for now timed runs are not supported]
Below that is a tabbed notebook.  The tabbed notebook has the following pre-built tabs:

*   Monitored programs which has a widget that shows the state of the monitored programs.
*   Eventloggers - shows the state of event loggers
*   Main output log  - which has to consolidated output log.

Note that the tabbed notebook is dynamic.  new tabs are added each time there's output from a program not yet seen.


"""

from PyQt5.QtWidgets import (
    QVBoxLayout, QHBoxLayout, QWidget, 
    QPushButton, QCheckBox, QLineEdit, QLabel, QTabWidget, 
    QTextEdit, QTableView
    
)
from PyQt5.QtGui import ( QStandardItemModel, QStandardItem)

from PyQt5.QtCore import pyqtSignal


class RunInfoWidget(QWidget):
    """This class provides information about the run:
      - A text entry for the title. - with change button.
      - Actual title - title from the database
      - A Text entry for the run number.
      - A pushbutton to  increment the run number.
      - Push button to set the run number.
      - Actual run number,
      
      
      
    Attributes:
        title - the contents of the title widget Not it's possible this differs from the 
                title in the config data base if, e.g. more than one editor is involved.
        actualTitle Actual title string value.
        runNumber - contents of the run number text edit (this will be a positive integer).
        actualRunNumber - contents of the actual run number string.
        
    Signals:
        setTitle(str) - Set title button was clicked.,
        setRunNumber(int) - Set the run number button was clicked.
        incrRun(int)   - Increment run number was clicked Note:
              slots for this _probably_ should get the run number fromt he
              config database, increment it set it back inboth run number and a tual run number.

    """
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QHBoxLayout()
        
        #  title:
        user_title_layout = QHBoxLayout()
        
        title_layout = QVBoxLayout()
        
        self._titlelabel = QLabel('Title: ', self)
        self._title = QLineEdit(self)
        self._setTitle = QPushButton('Set')
        
        user_title_layout.addWidget(self._titlelabel)
        user_title_layout.addWidget(self._title)
        user_title_layout.addWidget(self._setTitle)
        
        title_layout.addLayout(user_title_layout)
        
        actual_title_layout = QHBoxLayout()
        self._actual_titlelabel = QLabel('Actual', self)
        self._actual_title      = QLabel('', self)
        actual_title_layout.addWidget(self._actual_titlelabel)
        actual_title_layout.addWidget(self._actual_title)
        title_layout.addLayout(actual_title_layout)
        
        layout.addLayout(title_layout)
        
        #  Now the run number - same layout structure as title...just other widgets.
        
        run_layout = QVBoxLayout()
    
        user_run_layout = QHBoxLayout()
        self._runlabel = QLabel('Run #', self)
        self._run = QLineEdit(self)
        self._increment = QPushButton('+', self)
        self._setRun   = QPushButton('Set', self)
        
        user_run_layout.addWidget(self._runlabel)
        user_run_layout.addWidget(self._run)
        user_run_layout.addWidget(self._increment)
        user_run_layout.addWidget(self._setRun)
        
        run_layout.addLayout(user_run_layout)
        
        actual_run_layout = QHBoxLayout()
        self._actual_run_label = QLabel("Actual Run#", self)
        self._actual_run = QLabel('', self)
        
        actual_run_layout.addWidget(self._actual_run_label)
        actual_run_layout.addWidget(self._actual_run)
        
        run_layout.addLayout(actual_run_layout)
        
        layout.addLayout(run_layout)
        
        
        
        # put a validator on the run number so the user can't have a non-integer.
        
        
        # Connect signals.
        

        
        self.setLayout(layout)
        
        
if __name__ == "__main__":
    from PyQt5.QtWidgets import (QApplication, QMainWindow)
    import sys
    app = QApplication(sys.argv)
    main_window = QMainWindow()
    test_widget = RunInfoWidget()
    main_window.setCentralWidget(test_widget)

    main_window.show()
    app.exec()