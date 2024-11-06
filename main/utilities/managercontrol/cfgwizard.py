"""Contains the configuration prompting wizard - ConfigWizard.
"""
    
from PyQt5.QtWidgets import (
    QWizard, QWizardPage, QLabel, QLineEdit
)


class IntroPage(QWizardPage):
    #  Wizard page that introduces the configuration wizard.
    #
    def __init__(self):
        super().__init__()
        
        layout = QVBoxLayout()
        self._instructions - QLabel(self)
        self._instructions.setWordWrap(True)
        self._instructions.setText(
            """
<h2>Instructions:</h2>
<p>
  The control panel requires some configuration.  This is expectged to be
  in the file <tt>mg_guiconfig.toml</tt> in the working directory at the
  time you run this program.
</p>
<p>
    If, as is the case now, that file does not exist, this wizard will prompt
    you for all of the information that is stored int that configuration file.
    Once you've finished the wizard, the <tt>mg_guiconfig.toml</tt>
    file will be written so you won't have to go through this again.
</p>
<p>
    You will need to know:
</p>
<h3>Connection parameters:</h3>
<ol>
    <li>Which host the manager will be run in.</li>
    <li>Which user will start the manager, if not this account</li>
    <li>If non standard service names for the ReST control and output monitor are advertised,
        the names of those services</li>
</ol>
<h3>Configuration information</h3>
<p>
   The names of programs that actually readout data.  These programs are monitored in one of the
   GUI status tabs so, if one fails, you can see which one.
</p>
<hr />
<p>
To get on with the wizard, click the <tt>Next</tt> button below.
"""
        layout.addWidget(seslf._instructions)
        
        self.setLayout(layout)

class ConfigWizard(QWizard):
    """Provides a self contained wizard for 
        configuring the control GUI.


    Subclass of:
       QWizard
       
    Note:
       exec - will, if necessary, modify the configuration passeed in to the constructor.
       
       
    """
    def __init__(self, config):
        super().__init__()
        self._config = config
        self._intro = IntroPage()
        self.addPage(self._intro)
        
        
if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication
    import sys
    from nscldaq.manager_control.config import Configuration
    
    app = QApplication(sys.argv)
    
    
    config = Configuration('/dev/null')
    wiz = ConfigWizard(config)
    wiz.exec()
    config.dump('wiztest.toml')