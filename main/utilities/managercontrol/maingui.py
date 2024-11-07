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
from PyQt5.QtGui import ( QStandardItemModel, QStandardItem, QIntValidator)

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
    updateTitle = pyqtSignal(str)
    updateRunNumber = pyqtSignal(int)
    incRun = pyqtSignal(int)
    
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QHBoxLayout()
        
        #  title:
        user_title_layout = QHBoxLayout()
        
        title_layout = QVBoxLayout()
        
        self._titlelabel = QLabel('Title: ', self)
        self._title = QLineEdit(self)
        self._title.setMaxLength(80)
        self._title.setMinimumWidth(80*5)
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
        self._run = QLineEdit('0', self)
        self._run.setMaxLength(8)
        self._increment = QPushButton('+', self)
        self._setRun   = QPushButton('Set', self)
        
        user_run_layout.addWidget(self._runlabel)
        user_run_layout.addWidget(self._run)
        user_run_layout.addWidget(self._increment)
        user_run_layout.addWidget(self._setRun)
        
        run_layout.addLayout(user_run_layout)
        
        actual_run_layout = QHBoxLayout()
        self._actual_run_label = QLabel("Actual Run#", self)
        self._actual_run = QLabel('0', self)
        
        actual_run_layout.addWidget(self._actual_run_label)
        actual_run_layout.addWidget(self._actual_run)
        
        run_layout.addLayout(actual_run_layout)
        
        layout.addLayout(run_layout)
        
        
        
        # put a validator on the run number so the user can't have a non-integer.
        # for now just an integer -- TODO require it be >0.
        
        self._run.setValidator(QIntValidator())
        
        # Connect signals.
        
        self._setTitle.clicked.connect(self._setTitleRelay)
        self._setRun.clicked.connect(self._setRunNumberRelay)
        self._increment.clicked.connect(self._incRun)
        
        self.setLayout(layout)
    
    # Attributes:
    
    def title(self):
        """Return the user title string.
        """
        return self._title.text()
    def setTitle(self, title):
        """Set the user title string. The actual title string is not modified.

        Args:
            title (str): new _user_ title string.
        """
        self._title.setText(title)
        
    def runNumber(self):
        """Return the run number (integer).
        """
        return int(self._run.text())
    
    def setRunNumber(self, run):
        """
        Set the user run number.

        Args:
            run (int): New run number should be a positive int.
            
        """
        self._valid_run(run)
        self._run.setText(str(run))
        
    
    # THe actual parameter attributes:
    
    def actualTitle(self):
        """Retturn the value of the actual title.
        """
        return self._actual_title.text()
    
    def setActualTitle(self, title):
        """Set the value of the actual title.

        Args:
            title (str): new title.
        """
        self._actual_title.setText(title)
    
    def actualRunNumber(self):
        """Return actual run number value."""
        return int(self._actual_run.text())
    def setActualRunNumber(self, run):
        """Set the new actual run number value

        Args:
            run (int): _description_

        Raises:
            TypeError: If run isn't an int.
            ValueError: run is an int but is negative.
        """
        self._valid_run(run)
        self._actual_run.setText(str(run))
        
    # private methods.
    
    def _valid_run(self, run):
        # Throw an appropriate exceptionm if a run number is rinvalid.
        
        if not isinstance(run, int):
            raise TypeError(f'{run} must be a positive integer but was a {type(run)}')
        r = int(run)
        if r < 0:
            raise ValueError(f'Run number must be a positive integer, was {run}')
    
    # Signal relays:
    
    def _setTitleRelay(self):
        # SIgnal relay called when the set title button was clicked. - 
        # emit setTitle passing the current title string
        
        self.updateTitle.emit(self._title.text())
    def _setRunNumberRelay(self):
        # SIgnal relay for the run number
        
        try:
            self.updateRunNumber.emit(int(self._run.text())) 
        except:
            # Probably integer conversion of run failed:
            
            pass
     
    def _incRun(self):
        #   Relay the increment run  click:
         
        try:
            self.incRun.emit(int(self._run.text()))
        except:
            pass     #likely int conversion failed.
        

        
if __name__ == "__main__":
    test_widget = None
    def titleHandler(title):
        test_widget.setActualTitle(title)
        print('Updated to ', test_widget.actualTitle())

    def runhandler(run):
        test_widget.setActualRunNumber(run)
        print('Updated to ', run)
    
    def inchandler(run):
        now =test_widget.runNumber()
        test_widget.setRunNumber(now+1)
    
    from PyQt5.QtWidgets import (QApplication, QMainWindow)
    import sys
    app = QApplication(sys.argv)
    main_window = QMainWindow()
    test_widget = RunInfoWidget()
    test_widget.updateTitle.connect(titleHandler)
    test_widget.updateRunNumber.connect(runhandler)
    test_widget.incRun.connect(inchandler)
    main_window.setCentralWidget(test_widget)

    main_window.show()
    app.exec()