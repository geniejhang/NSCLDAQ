import matplotlib
matplotlib.use("Qt5Agg")
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
import numpy as np
import inspect
import copy
import sys
import logging
import time
from math import ceil, floor

from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QMessageBox
from PyQt5.QtGui import QPaintEvent

from fit_panel import FitPanel

import xia_constants as xia
from run_type import RunType

class Plot(QWidget):
    """Plotting widget for the GUI utilizing the matplotlib Qt5 backend.

    Attributes
    ----------
    figure : pyplot figure 
        matplotlib pyplot figure instance.
    canvas : FigureCanvasQTAgg 
        Canvas the figure renders onto imported from Qt5 backend.
    fit_panel : QWidget 
        Trace and histogram fitting GUI widget.
    fit_factory : FitFactory 
        Factory method for fitting plot data.
    toolbar : NavigationToolbar2QT
        Figure navigation toolbar imported from Qt5 backend.
    logger : Logger
        QtScope Logger object.
    trace_x : list
        Trace data x values. Defines the left bin edge [i, i+1).
    hist_x : list
        Histogram data x values. Defines the left bin edge [i, i+1).
    raw_data : list
        "Raw" data returned from the digitizers transformed into a Python list.
        This data maintains the same size as the data array returned from the 
        digitizers if the histograms are rebinned.
    data : numpy.ndarray
        Re-histogrammed data; possibly with different binning than the raw 
        data. First index of the tuple returned from matplotlib.axes.Axes.hist.
    bins : numpy.ndarray
        Bin edges used to create NumPy histograms. Second index of the tuple 
        returned from matplotlib.axes.Axes.hist.
    artists : list
        Container of individual artists used to create the histogram. Third 
        index of the tuple returned from matplotlib.axes.Axes.hist.
    warned : bool
        Flag is true if the histogram interactive rebinning message has already
        been displayed.

    Methods
    -------
    draw_trace_data(data, nrows=1, ncols=1, idx=1)
        Draw single- or multuple-channel ADC trace data.
    draw_analyzed_trace(trace, fast_filter, cfd, slow_filter)
        Draw a single-channel ADC trace and its filter outputs. 
    draw_run_data(data, run_type, nrows=1, ncols=1, idx=1)
        Draw single- or multuple-channel energy histogram or baseline data.
    on_begin_run(run_type)
        Draw a blank histogram-style canvas when starting a histogram run.
    draw_test_data(idx)
        Draw a test figure with a random number of subplots.
    """
    
    def __init__(self, toolbar_factory, fit_factory, *args, **kwargs):
        """Plot class constructor.

        Arguments
        ---------
        toolbar_factory : ToolBarFactory
            Factory for implemented toolbars.
        fit_factory : FitFactory 
            Factory method for fitting plot data.
        """
        super().__init__(*args, **kwargs)

        # Get the logger instance:

        self.logger = logging.getLogger("qtscope_logger")

        # Initialize histogram ranges and data:
        
        self.hist_x = [i for i in range(xia.MAX_HISTOGRAM_LENGTH)]
        self.trace_x = [i for i in range(xia.MAX_ADC_TRACE_LEN)]
        self.raw_data = []
        self.data = np.empty(0)
        self.bins = np.empty(0)
        self.artists = []
        self.warned = False
        
        ##
        # Main layout
        #
        
        self.figure = plt.figure(tight_layout=True)
        self.canvas = FigureCanvasQTAgg(self.figure)
        self.fit_panel = FitPanel()
        self.toolbar = toolbar_factory.create("plot", self.canvas, self)

        # Initialize the factory with the known methods:
        
        self.fit_factory = fit_factory
        self.fit_factory.initialize(self.fit_panel.function_list)

        # Once initialized, we assume the config information contains some
        # valid text field giving the fit function formula:

        self._update_formula()

        # Disable toolbar on creation (enabled on boot):
        
        self.toolbar.disable()
        
        # Blank canvas with axes to show that this is the plot area:
        
        ax = self.figure.add_subplot(1, 1, 1)
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)

        layout = QVBoxLayout()
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        self.setLayout(layout)

        ##
        # Signal connections
        #
        
        self.toolbar.logscale.clicked.connect(
            lambda ax=None: self._set_yscale(ax=None)
        )
        self.toolbar.b_fit_panel.clicked.connect(self._show_fit_panel)
        self.toolbar.bin_slider.valueChanged.connect(self._rebin)

        self.fit_panel.b_fit.clicked.connect(self._fit)
        self.fit_panel.b_clear.clicked.connect(self._clear_fit)
        self.fit_panel.b_cancel.clicked.connect(self._close_fit_panel)
        self.fit_panel.function_list.currentTextChanged.connect(
            self._update_formula
        )
        
    def draw_trace_data(self, data, nrows=1, ncols=1, idx=1):
        """Draws an ADC trace on the plot canvas.

        Parameters
        ----------
        data : list
            List of trace data values.
        nrows : int  
            Number of subplot rows (optional, default=1).
        ncols : int  
            Number of subplot columns (optional, default=1).
        idx : int 
            Subplot index in [1, nrows*ncols] (optional).
        """        
        ax = self.figure.add_subplot(nrows, ncols, idx)            
        self._histogram_data(ax, data, xia.MAX_ADC_TRACE_LEN, "trace")        
        ax.set_xlabel("Sample number (60 ns/sample)")
        ax.set_ylabel("Voltage (ADC units)")
        ax.set_xlim(0, xia.MAX_ADC_TRACE_LEN)
        self._set_yscale(ax)

        if (nrows*ncols) > 1:
            ax.set_title("Channel {}".format(idx-1))  # Channels are 0-indexed.
            ax.tick_params(axis='x', labelsize=6)
            ax.tick_params(axis='y', labelsize=6)
            ax.xaxis.label.set_size(6)
            ax.yaxis.label.set_size(6)
        
        self.canvas.draw_idle()
        
    def draw_analyzed_trace(self, trace, fast_filter, cfd, slow_filter):
        """Draw a trace and its filter outputs.

        Parameters
        ----------
        trace : list
            Single-channel ADC trace data.
        fast_filter : list
            Computed fast filter output.
        cfd : list
            Computed CFD output.
        slow_filter : list
            Computed slow filter output.
        """
        ax1 = self.figure.add_subplot(3, 1, 1)
        ax1.set_title("Trace")
        self._histogram_data(ax1, trace, xia.MAX_ADC_TRACE_LEN, "trace")
        self._set_yscale(ax1, pad=0.1)
        
        ax2 = self.figure.add_subplot(3, 1, 2)
        ax2.set_title("Timing filters")
        self._histogram_data(
            ax2, fast_filter, xia.MAX_ADC_TRACE_LEN, "trace"
        )
        self._set_yscale(ax2, pad=0.1) # Let the fast filter set the range.
        self._histogram_data(
            ax2, cfd, xia.MAX_ADC_TRACE_LEN, "trace", color = "tab:orange"
        )        
        ax2.legend(["Trigger filter", "CFD"], loc="upper right")
        
        ax3 = self.figure.add_subplot(3, 1, 3)
        ax3.set_title("Energy filter")
        self._histogram_data(
            ax3, slow_filter, xia.MAX_ADC_TRACE_LEN, "trace"
        )
        self._set_yscale(ax3, pad=0.1)

        for ax in self.figure.get_axes():
            ax.set_xlim(0, xia.MAX_ADC_TRACE_LEN)
            
        self.canvas.draw_idle()
        
    def draw_run_data(self, data, run_type, nrows=1, ncols=1, idx=1):
        """Draws a data histogram on the plot canvas.

        Parameters
        ----------
        data : array 
            Single-channel run data.
        run_type : Enum member 
            Type of run data to draw.
        nrows : int, optional, default=1
            Number of subplot rows.
        ncols : int, optional, default=1
            Number of subplot columns.
        idx : int, optional, default=1
            Subplot index in [1, nrows*ncols].
        """        
        ax = self.figure.add_subplot(nrows, ncols, idx)
        factor = int(self.toolbar.bin_factor.text())
        nbins = int(xia.MAX_HISTOGRAM_LENGTH/factor)
        self._histogram_data(ax, data, nbins, "hist")
                
        if run_type == RunType.HISTOGRAM:
            ax.set_xlabel("Energy (ADC units)")
        elif run_type == RunType.BASELINE:
            ax.set_xlabel("Baseline value (ADC units)")
            
        ax.set_ylabel("Counts/bin")
        ax.set_xlim(0, xia.MAX_HISTOGRAM_LENGTH)
        self._set_yscale(ax)
        
        if (nrows*ncols) > 1:
            chan = idx - 1  # Channels are zero-indexed.
            ax.set_title("Channel {}".format(chan))
            ax.tick_params(axis='x', labelsize=6)
            ax.tick_params(axis='y', labelsize=6)
            ax.xaxis.label.set_size(6)
            ax.yaxis.label.set_size(6)
            
        self.canvas.draw_idle()
                        
    def on_begin_run(self, run_type):
        """Draws a blank histogram canvas when beginning a new run.
        
        Parameters
        ----------
        run_type : Enum member
            Type of run data to draw.
        """
        self.figure.clear()
        try:
            if run_type != RunType.HISTOGRAM and run_type != RunType.BASELINE:
                raise ValueError(f"Encountered unexpected run type {run_type}, select a valid run type and begin a new run")  
        except ValueError as e:
            self.logger.exception("Encountered unknown run type")
            print(e)            
        else:
            ax = self.figure.add_subplot(1, 1, 1)
            if run_type == RunType.HISTOGRAM:
                ax.set_xlabel("ADC energy")
            elif run_type == RunType.BASELINE:
                ax.set_xlabel("Baseline value")
            ax.set_ylabel("Counts/bin")
            ax.set_xlim(0, xia.MAX_HISTOGRAM_LENGTH)
            ax.set_ylim(0, 1)
            self._set_yscale(ax)        
            self.canvas.draw_idle()

    def draw_test_data(self):
        """Draw test data. 

        Draw data points in (-1, 1) on a test canvas with a random number of 
        rows and columns with rows, columns in [1, 4]
        """        
        self.figure.clear()        
        rng = np.random.default_rng()
        nrows = rng.integers(1, 5)
        ncols = rng.integers(1, 5)        
        for i in range(nrows):
            for j in range(ncols):
                idx = ncols*i + j + 1
                ax = self.figure.add_subplot(nrows, ncols, idx)
                data = 2*rng.random(size=5) - 1
                ax.plot(data, "-")                
        self.canvas.draw_idle()
        
    ##
    # Private methods
    #
    
    def _set_yscale(self, ax, pad=0.05):
        """Configurs the axes display on the canvas.

        If the log scale checkbox is selected while displaying data, 
        immidiately redraw all subplots.

        Parameters
        ----------
        ax : matplotlib Axes
            matplotlib class containing the figure elements.
        pad : float, optional, default=0.05
            Padding as a percent of the full data range added to the y-axis 
            limits when drawing the canvas.
        """        
        if self.sender():
            for ax in self.figure.get_axes():
                self._set_subplot_yscale(ax, pad)
            self.canvas.draw_idle()
        else:
            self._set_subplot_yscale(ax, pad)
                    
    def _set_subplot_yscale(self, ax, pad):
        """Sets the y-axis display for a single figure (linear or log).
    
        The y-axis is autoscaled in log for trace data because all values are 
        greater than zero. For histogram data the axis scaling in log is 
        handled automatically, but in linear the y-axis is set to start at zero.

        Parameters
        ----------
        ax : matplotlib Axes
            matplotlib class containing the figure elements.
        pad : float
            Padding as a percent of the full data range added to the y-axis 
            limits when drawing the canvas.
        """        
        if len(self.data) > 0:
            maxval = max(self.data)
            minval = min(self.data)
            yrange = maxval - minval
            ymax = maxval + pad*yrange
            ymin = minval - pad*yrange
        else:
            ymax, ymin = 0, 0
            
        ax.autoscale(axis="y")
        
        if self.toolbar.logscale.isChecked():           
            # Symmetric log (should?) handle possible zeroes nicely in the
            # case of empty histograms or inverted traces.
            if ymax > 0:
                ax.set_yscale("log")
            else:
                ax.set_yscale("symlog")
                ax.set_ylim(0.1, 1)
        else:
            ax.set_yscale("linear")
            npts = len(self.raw_data)
            if npts == xia.MAX_HISTOGRAM_LENGTH:
                ax.set_ylim(0, ymax)
            elif npts == xia.MAX_ADC_TRACE_LEN:
                ax.set_ylim(ymin, ymax)
            else:
                ax.set_ylim(0, 1)
              
    def _show_fit_panel(self):
        """Show the fit panel GUI in a popup window."""
        self.fit_panel.show()

    def _rebin(self):
        """Rebin histogrammed data."""
        # Check length of last raw data. There are no mixed plots, so the last
        # data will do. Only immidiately rebin using the slider for a single
        # histogram, as the plotter only keeps track of the last data it saw.
        naxes = len(self.figure.get_axes())
        if len(self.raw_data) == xia.MAX_HISTOGRAM_LENGTH and naxes == 1:
            ax = plt.gca()
            factor = int(self.toolbar.bin_factor.text())
            nbins = int(xia.MAX_HISTOGRAM_LENGTH/factor)
            # Remove all drawn histograms but not any axis labeling, etc.
            for artist in self.artists:
                artist.remove()                
            self._histogram_data(ax, self.raw_data, nbins, "hist")           
            self._set_yscale(ax)
            self.canvas.draw_idle()
        else:
            if not self.warned:
                QMessageBox.about(self, "Warning", "Interactive rebinning using the slider is only enabled for single-channel list-mode histogram or baseline plots. Multi-channel list-mode histogram or baseline data will be binned using the value set on the slider the next time they are acquired.")
                self.warned = True
            
    def _fit(self):
        """Perform the fit based on the current fit panel settings."""        
        if len(self.figure.get_axes()) == 1:
            ax = plt.gca()
            fcn = self.fit_panel.function_list.currentText()
            config = self.fit_factory.configs.get(fcn)
            fit = self.fit_factory.create(fcn, **config)
            xmin, xmax = self._axis_limits(ax)
            params = [
                float(self.fit_panel.p0.text()),
                float(self.fit_panel.p1.text()),
                float(self.fit_panel.p2.text()),
                float(self.fit_panel.p3.text()),
                float(self.fit_panel.p4.text()),
                float(self.fit_panel.p5.text())
            ]

            factor = int(self.toolbar.bin_factor.text())
            idx_min = xmin
            idx_max = xmax
            if len(self.raw_data) == xia.MAX_HISTOGRAM_LENGTH:
                idx_min = ceil(int(idx_min/factor))
                idx_max = floor(int(idx_max/factor))
                
            self.logger.debug(f"Function config params: {config}")
            self.logger.debug(f"Fit limits: {xmin}, {xmax}")
            self.logger.debug(f"Fit limit indices: {idx_min}, {idx_max}")
            self.logger.debug(f"Fit panel guess params: {params}")
            self.logger.debug(f"Run data binning factor: {factor}")
            
            x = (self.bins[:-1] + self.bins[1:])/2 # Bin centers
            fitln = fit.start(
                x[idx_min:idx_max], self.data[idx_min:idx_max], params,
                ax, self.fit_panel.results
            )
            self.canvas.draw_idle()            
        else:
            QMessageBox.about(self, "Warning", "Cannot perform the fit! Currently displaying data from multiple channels or an analyzed trace. Please acquire single-channel data and attempt the fit again.")
            
    def _axis_limits(self, ax):
        """Get the fit limits based on the selected range. 

        If no range is provided the currently displayed axes limits are assumed
        to be the fit range. Fit limits are rounded to the nearest bin edge 
        within the fitting range using the ceil (left limit) and floor (right 
        limit) functions.

        Parameters
        ----------
        ax : PyPlot axes
            Currently displayed single channel data plot axis.
        
        Returns
        -------
        int, int
            Left and right axis limits.
        """        
        left, right = ax.get_xlim()
        if self.fit_panel.range_min.text():
            left = ceil(float(self.fit_panel.range_min.text()))
        else:
            left = ceil(ax.get_xlim()[0]) 
        if self.fit_panel.range_max.text():
            right = floor(float(self.fit_panel.range_max.text()))
        else:
            right = floor(ax.get_xlim()[1])
            
        return left, right

    def _clear_fit(self):
        """Clear previous fits and reset the fit panel GUI."""
        if len(self.figure.get_axes()) == 1:
            ax = plt.gca()
            for i in range(len(ax.lines)):
                ax.lines.pop()
            self.canvas.draw_idle()
            self.fit_panel.reset()

    def _update_formula(self):
        """Update the fit function formula when a new function is selected."""
        fcn = self.fit_panel.function_list.currentText()
        config = self.fit_factory.configs.get(fcn)
        self.fit_panel.function_formula.setText(config["form"])

    def _close_fit_panel(self):
        """Close the fit panel window."""
        self.fit_panel.close()
   
    def _histogram_data(
            self, ax, raw_data, nbins, data_type, color="tab:blue"
    ):
        """Make a histogram out of array-like raw data.

        Parameters
        ----------
        ax : matplotlib Axes
            matplotlib class containing the figure elements.
        raw_data : list
            "Raw" data with default binning from XIA modules used to create
            the histogram.
        nbins : int
            Number of histogram bins.
        data_type : str
            Type of data to histogram. Characterized by the length of the data
            and assumed to be the maximum size allowed by the API. Valid types
            are 'hist' or 'trace'.
        color : str, optional, default="tab:blue"
            Histogram line color. Can be any of 
            https://matplotlib.org/stable/tutorials/colors/colors.html.

        Raises
        ------
        ValueError
            If the data type argument is an invalid value.
        """
        try:
            if data_type == "hist":
                x = self.hist_x
                data_range = (0, xia.MAX_HISTOGRAM_LENGTH)
            elif data_type == "trace":
                x = self.trace_x
                data_range = (0, xia.MAX_ADC_TRACE_LEN)
            else:
                raise ValueError(f"Unrecognized data type {data_type}. Allowed values are 'hist' or 'trace.'")
        except ValueError as e:
            self.logger.exception("Failed to histogram run data")
            print(e)
        else:
            self.raw_data = raw_data
            self.data, self.bins, self.artists = ax.hist(
                x,
                range=data_range,
                bins=nbins,
                weights=self.raw_data,
                histtype="step",
                color=color
            )
