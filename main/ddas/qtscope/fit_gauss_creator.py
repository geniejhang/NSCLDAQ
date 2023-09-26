from fit_function import FitFunction
import numpy as np

class GaussFit(FitFunction):
    """Gaussian fitting function class used by QtScope.

    Implements function-specific feval and set_initial_parameters methods from
    the base class. See the documentation for the FitFunction base class in 
    fit_function.py for details.
    """
    
    def feval(self, x, *p):
        """Evaluate the fit function over x.

        Implement a Gaussian fitting function.

        Parameters
        ----------
        x : ndarray
            Array of x values in the fitting range.
        p : array-like
            Parameters used to evaluate the function over x.

        Returns
        ------
        ndarray
            Array containing the fit values over the range.
        """
        return p[0]*np.exp(-(x-p[1])**2 / (2*p[2]**2))

    def set_initial_parameters(self, x, y, params):
        """Set initial parameter values. 

        Guess at the amplitude, mean, and stddev using the defined fit range
        if no parameters are provided on the fit panel.

        Parameters
        ----------
        x : list
            x data values.
        y : list 
            y data values.
        params : list
            Array of fit parameters.
        """
        super().set_initial_parameters(x, y, params)
        if self.pinit[0] == 0.0:
            self.pinit[0] = max(y)
        if self.pinit[1] == 0.0:
            self.pinit[1] = np.mean(x)
        if self.pinit[2] == 0.0:
            self.pinit[2] = np.std(x)

class GaussFitBuilder:
    """Builder method for factory creation."""
    
    def __init__(self):
        """GaussFitBuilder class constructor."""        
        self._instance = None

    def __call__(self, params=[], form=""):
        """Create the fitting function.

        Create an instance of the fit function if it does not exist and 
        return it to the caller. Parameters passed as unpacked **kwargs 
        from the fit factory.

        Parameters
        ----------
        params : array-like
            Array of initial parameters. In general not used, but at least 
            ensures the class is initialized with valid and/or reasonable 
            starting guesses.
        form : str 
            Function formula.

        Returns
        -------
        GaussFit
            Instance of the fit class.
        """       
        if not self._instance:
            self._instance = GaussFit(params, form)
            
        return self._instance
