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
        

class RunControlWidget(QWidget):
    """Class that provides state transition controls. 
        This provides  buttons and internal state
        An internal state is used to control what the button look like:
        *  A shutdown button is provided that is enabled if the state is not SHUTDOWN
        *  A transition button is 
           - disabled if the state is SHUTDOWN
           - Labeled 'Initialize'  if the state is BOOT
           - BEGIN if the state is HWINIT or END
           - END if the state is BEGIN
           

    Parent class:
        QWIdget 
    Attributes:
        state(readonly)  Provides a state e.g. RunControlWidget.SHUTDOWN
        
    Methods:
        boot - transition the widget to RunControlWidget.BOOT
        shutdown - Transition the Widget to RunControlWidget.SHUTDOWN
        hwinit - Transition the widget to RunControlWidget.HWINT
        begin - Transition the widget to RunControlWidget.BEGIN
        end   - Transition the widget to RunControl.END
        
        state_name - translates the state value (from state()) to a state name string.
        
    Signals:
        transition(str) - A transition is requested to the new named state.
           Note if the shtdown button is clickeed transtion('SHUTDOWN') is emitteed.
        
    """
    # Class storage:
    
    # Any resemblance betwween these values and the id  field in the database
    # should be treated as purely conncidental.
    
    BOOT = 1
    SHUTDOWN = 2
    HWINIT = 3
    BEGIN = 4
    END = 5
    
    
    _StateNames = {
        BOOT: 'BOOT', SHUTDOWN: 'SHUTDOWN', HWINIT: 'HWINIT', BEGIN: 'BEGIN', END: 'END'
    }
    # Transition Button label table:
    
    _TransitionLabels = {
        SHUTDOWN : 'Boot', BOOT: 'HW Init', HWINIT: 'Begin', END: 'Begin', BEGIN: 'END'
    }
    #  The Shutdown button can't be alive if we're already shutdown:
    
    _ShutdownDisaledState = [SHUTDOWN]
    def __init__(self, *args):
        super().__init__(*args)
        
        layout = QHBoxLayout()
        
        # CUrrent state:
        
        self._statelabel = QLabel('State: ', self)
        layout.addWidget(self._statelabel)
        self._statename = QLabel('SHUTDOWN', self)
        layout.addWidget(self._statename)
        
        # SHutdown button
        
        self._reqshutdown = QPushButton('Shutdown', self)
        layout.addWidget(self._reqshutdown)
        
        #  State transition button:
        
        self._transitionreq  = QPushButton('BOOT', self)
        layout.addWidget(self._transitionreq)
        
        self._stateid = self.SHUTDOWN
        self._update_buttons()
        
        self.setLayout(layout)
    
    # Attributes
    
    def state(self):
        """Return the current state id. This attribute has no direct setter.
        """
        return self._stateid
    def state_name(self):
        """return the name of the currdnt state.  This attribute has no setter.
        """
        return self._StateNames[self._stateid]
    
    # Public methods:
    
    def boot(self):
        """ Transition to the 'BOOT' state._
        """
        self._transition(self.BOOT)
    def shudown(self):
        """Transitino to SHUTDOWN:
        """
        self._transition(self.SHUTDOWN)
    def hwinit(self):
        """Transition to the HWINIT state:
        """
        self._transition(self.HWINIT)
    def begin(self):
        """Transition to the BEGIN state:
        """
        self._transition(self.BEGIN)
    def end(self):
        """Transition to endid.
        """
        self._trasition(slef.END)
    
    # private methods
    
    def _transition(self, newid):
        # Tranition to the new state in the newid:
        
        self._stateid = newid
        self._update_buttons(self)
        
    
    def _update_buttons(self):
        # If necessary disable the SHUTDOWN button:
        
        if self._stateid in self._ShutdownDisaledState:
            self._reqshutdown.setDisabled(True)
        else:
            self._reqshutdown.setDisabled(False)
        
        # Set the label on the transition button:
        
        self._transitionreq.setText(self._TransitionLabels[self._stateid])
    
    
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
    test_widget = RunControlWidget()
    main_window.setCentralWidget(test_widget)

    main_window.show()
    app.exec()