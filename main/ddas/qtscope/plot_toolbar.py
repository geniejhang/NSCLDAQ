from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT

from PyQt5.QtWidgets import (
    QPushButton, QCheckBox, QSlider, QHBoxLayout, QLabel, QGroupBox
)
from PyQt5.QtCore import Qt

import colors

class PlotToolBar(NavigationToolbar2QT):
    """ToolBar for matplotlib plotting widget. 

    Defualt feautres support basic plot manipulation (zoom region, pan, etc.). 
    Extra plotting featrues are enabled using additional widgets. Unwanted 
    default plot configuration options can be removed via their associated 
    action.

    Attributes
    ----------
    binning_slider : QSlider
        Slider to control histogram binning.
    binning_factor : QLabel
        ADC/units per bin determined from the slider position.
    logscale : QCheckBox
        Enable/disable logscale plotting.
    b_fit_panel : QPushButton
        Button to display the fitting panel GUI.

    Methods
    -------
    disable() 
        Disable all toolbar widgets.
    enable()
        Enable toolbar widgets for system idle.
    """
    
    def __init__(self, *args, **kwargs):
        """PlotToolBar class constructor."""        
        super().__init__(*args, **kwargs)

        self.binning_slider = QSlider(Qt.Horizontal)
        self.binning_slider.setRange(0, 4)
        self.binning_slider.setSliderPosition(0)
        self.binning_slider.setSingleStep(1)
        self.binning_slider.setMaximumWidth(100)
        label = QLabel("Run histogram ADC units/bin:")
        self.binning_factor = QLabel(f"{pow(2, self.binning_slider.value())}")
        self.binning_factor.setMinimumWidth(20)

        
        self.logscale = QCheckBox("Log y-axis", self)
        
        self.b_fit_panel = QPushButton("Fit panel", self)        
        self.b_fit_panel.setStyleSheet(colors.YELLOW)

        #self.addWidget(label)
        #self.addWidget(self.binning_slider)
        #self.addWidget(self.binning_factor)
        self.addWidget(self.logscale)
        self.addWidget(self.b_fit_panel)

        # Remove buttons from toolbar:
        
        unwanted_buttons = ["Back", "Forward", "Subplots", "Pan", "Customize"]
        for action in self.actions():
            if action.text() in unwanted_buttons:
                self.removeAction(action)

        ##
        # Signal connections
        #

        #self.binning_slider.valueChanged.connect(self._set_factor)

    def disable(self):
        """Disable child widgets in the plot toolbar."""
        self.logscale.setEnabled(False)
        self.b_fit_panel.setEnabled(False)
        
    def enable(self):
        """Enable child widgets in the plot toolbar."""
        self.logscale.setEnabled(True)
        self.b_fit_panel.setEnabled(True)

    ##
    # Private methods
    #

    def _set_factor(self):
        """Change the binning factor label when the slider value changes."""
        self.binning_factor.setText(f"{pow(2, self.binning_slider.value())}")

class PlotToolBarBuilder:
    """Builder method for factory creation."""
    
    def __init__(self, *args, **kwargs):
        """PlotToolbarBuilder class constructor."""
        
    def __call__(self, *args, **kwargs):
        """Create an instance of the toolbar and return it to the caller.

        Returns
        -------
        PlotToolBar
            Instance of the toolbar class.
        """        
        return PlotToolBar(*args, **kwargs)
