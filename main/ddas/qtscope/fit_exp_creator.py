import numpy as np
from scipy.optimize import curve_fit
import logging

class ExpFit:
    """Exponential fitting function
    
    Function formula is f(x) = A*exp(k*x) + C where A, k, C are free params.

    Attributes
    ----------
    A : float
        Amplitude.
    k : float
        Exponential rate constant.
    C : float
        Constant baseline.
    form : str
        Function formula.
    logger : Logger
        QtScope Logger instance.

    Methods
    -------
    feval(x, A, k, C): 
        Returns the array of function values evaluated over the fitting range.
    set_initial_parameters(y, params): 
        Set initial parameter values. Guess from the data if none are provided.
    start(x, y, params, axis, results): 
        Implementation of the fitting algorithm.
    """
    
    def __init__(self, A, k, C, form):
        """Exponential fit function class constructor. 

        Sets initial fit parameters.
        
        Parameters
        ----------
        A : float
            Amplitude.
        k : float
            Exponential rate constant.
        C : float
            Constant baseline.
        form : str
            Function formula to display on the fitting panel.
        """
        self.logger = logging.getLogger("qtscope_logger")
        self.A = A
        self.k = k
        self.C = C
        self.form = form
            
    def feval(self, x, A, k, C):
        """Evaluate the fit function over x.

        Parameters
        ----------
        x : ndarray
            Array of x values in the fitting range.
        A : float
            Amplitude.
        k : float
            Exponential rate constant.
        C : float
            Constant baseline.

        Returns
        -------
        ndarray
            Array containing the fit values over the range.
        """        
        return A*np.exp(k*(x-x[0])) + C

    def set_initial_parameters(self, y, params):
        """Set initial parameter values. 

        Guess at the amplitude and baseline using the defined fit range if 
        no parameters are provided on the fit panel. If no decay constant is
        provided, assume a tau of 20 microseconds from the config data.

        Parameters
        ----------
        y : list
            y data values.
        params : list
            Array of fit parameters.
        """
        if (params[0] != 0.0):
            self.A = params[0]
        else:
            self.A = max(y) - min(y)
            
        if (params[1] != 0.0):
            self.k = params[1]
            
        if (params[2] != 0.0):
            self.C = params[2]
        else:
            self.C = min(y)
            
    def start(self, x, y, params, axis, results):
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
        results : QTextEdit 
            Display widget for fit results.
        
        Returns
        -------
        fitln: list of Line2D objects
            List of lines representing the plotted fit data.
        """
        fitln = None        
        self.set_initial_parameters(y, params)        
        pinit = [self.A, self.k, self.C]
        self.logger.debug(f"Parameter initial guesses: {pinit}")
        popt, pcov = curve_fit(self.feval, x, y, p0=pinit, maxfev=5000)
        perr = np.sqrt(np.diag(pcov))  # Parameter sigma from cov. matrix.
        self.logger.debug(f"Fit parameters: {popt}")
        self.logger.debug(f"Fit covariance matrix:\n{pcov}")
        self.logger.debug(f"Fit parameter errors: {perr}")
        try:
            x_fit = np.linspace(x[0], x[-1], 10000)
            y_fit = self.feval(x_fit, *popt)
            
            fitln = axis.plot(x_fit, y_fit, 'r-')
            
            for i in range(len(popt)):
                s = "p[{}]: {:.6e} +/- {:.6e}".format(i, popt[i], perr[i])
                results.append(s)
                if i == (len(popt) - 1):
                    results.append("\n")
        except:
            pass
        
        return fitln        

class ExpFitBuilder:
    """Builder method for factory creation."""
    
    def __init__(self):
        """ExpFitBuilder class constructor."""        
        self._instance = None

    def __call__(self, A=0, k=0, C=0, form=""):
        """Create the fit function.

        Create an instance of the fit function if it does not exist and 
        return it to the caller. Arguments passed as **kwargs from factory.

        Parameters
        ----------
        A : float 
            Amplitude.
        k : float 
            Exponential rate constant.
        C : float 
            Constant baseline.
        form : str 
            Function formula.

        Returns
        -------
        ExpFit
            Instance of the fit class.
        """       
        if not self._instance:
            self._instance = ExpFit(A, k, C, form)
            
        return self._instance
