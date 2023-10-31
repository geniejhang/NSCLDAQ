import numpy as np
from scipy.optimize import curve_fit
import logging

class FitFunction:
    """Base class for fitting functions used by QtScope.

    Attributes
    ----------
    pinit : list
        Initial parameter value guesses.
    form : str
        Function formula string.
    logger : Logger
        QtScope Logger instance.

    Methods
    -------
    feval(x, *p)
        Returns an array of function values evaluated over the range x.
    set_initial_parameters(x, y, pinit)
        Set initial parameter values. Guess from the data if none are provided.
    start(x, y, pinit, ax, result)
        Implementation of the fitting algorithm.
    """

    def __init__(self, params, form):
        """Class constructor.

        Set initial fit parameters.

        Parameters
        ----------
        params : list
            Initial parameters to set.
        form : str
            Function formula to display on the fitting panel.
        """
        self.logger = logging.getLogger("qtscope_logger")
        self.pinit = params
        self.form = form

    def feval(self, x, *p):
        """Evaluate the fit function over x.

        This function is not implemented in the base class and must be 
        implemented by each fitting function.

        Parameters
        ----------
        x : ndarray
            Array of x values in the fitting range.
        p : float
            Parameters used to evaluate the function over x.

        Raises
        ------
        NotImplementedError
            If feval from the base class is called. This method must be 
            implemented in derived classes.
        """
        raise NotImplementedError()

    def set_initial_parameters(self, x, y, params):
        """Set initial parameter values. 

        Default behavior is to set initial parameter guesses using the values 
        provided on the fit panel.

        Parameters
        ----------
        x : ndarray
            x data values.
        y : ndarray 
            y data values.
        params : array-like
            Array of fit parameters.
        """
        self.pinit = params[0:len(self.pinit)]
                
    def start(self, x, y, params, axis):
        """Implementation of the fitting algorithm.

        Parameters
        ----------
        x : list
            x data values.
        y : list
            y data values.
        params : list
            Array of fit parameters.
        axis : matplotlib axes
            Axes for the plot.
        
        Returns
        -------
        popt, pcov : array, 2-D array
            Optimized parameters and parameter covariance matrix from the fit.
        """
        self.set_initial_parameters(x, y, params)
        self.logger.debug(f"Parameter initial guesses: {self.pinit}")
        popt, pcov = curve_fit(
            self.feval, x, y, p0=self.pinit, sigma=np.sqrt(y),
            absolute_sigma=True, maxfev=5000
        )
        self.logger.debug(f"Fit parameters: {popt}")
        self.logger.debug(f"Fit covariance matrix:\n{pcov}")
        
        return popt, pcov
