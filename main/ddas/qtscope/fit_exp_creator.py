from fit_function import FitFunction
import numpy as np

class ExpFit(FitFunction):
    """Exponential fitting function class used by QtScope.

    Implements function-specific model and set_initial_parameters methods from
    the base class. See the documentation for the FitFunction base class in 
    fit_function.py for details.

    """
    def model(self, x, params):
        """Evaluate the fit function over x.

        Implement an exponential fitting function.

        Parameters
        ----------
        x : ndarray
            Array of x values in the fitting range.
        params : array-like
            Parameters used to evaluate the function over x.

        Returns
        ------
        ndarray
            Array containing the fit values over the range.
        """
        return params[0]*np.exp(params[1]*(x-x[0])) + params[2]

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
        k_fallback = self.p_init[1]
        super().set_initial_parameters(x, y, params)
        if self.p_init[0] == 0.0:
            self.p_init[0] = max(y) - min(y)
        if self.p_init[1] == 0.0:
            self.p_init[1] = k_fallback
        if self.p_init[2] == 0.0:
            self.p_init[2] = min(y)

class ExpFitBuilder:
    """Builder method for factory creation.

    """
    def __init__(self):
        """ExpFitBuilder class constructor."""
        self._instance = None

    def __call__(self, params=[], form="", count_data=False):
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
        ExpFit
            Instance of the fit class.
        """
        if not self._instance:
            self._instance = ExpFit(params, form, count_data)
        return self._instance
