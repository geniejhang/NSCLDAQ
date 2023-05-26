DEBUG=False

class FitFactory:
    """Factory method for fitting functions.
    
    Attributes
    ----------
    builders : dict 
        Dictionary of builder methods.
    config : dict 
        Dictionary of configurations.

    Methods
    -------
    register_builder(key, builder, config)
        Register a builder and config by key name.
    create(key)
        Create an instance of a fit function from a key name.
    initialize(item) 
        Initial setup of the fit factory for supported functions.
    """
    
    def __init__(self):
        """FitFactory constructor."""        
        self.builders = {}
        self.configs = {}

    def register_builder(self, key, builder, config):
        """Register a fitting function builder.

        Parameters
        ----------
        key : str 
            Key name for the fitting function.
        builder 
            Builder method for the fitting function class.
        config : dict 
            Parameter dictionary with initial guesses.
        """        
        self.builders[key] = builder
        self.configs[key] = config
        if DEBUG:
            print("  Registered: {}".format(key))

    def create(self, key, **kwargs):
        """Create an instance of the fit function from a key value. 

        Fit function parameters and their inital guesses are passed to the 
        builder as **kwargs.

        Parameters
        ----------
        key : str
            Key name for the fitting function.

        Keyword arguments
        -----------------
        config : dict
            Dictionary containing the fit parameters and initial guesses.

        Returns
        -------
        An instance of the fitting function class.

        Raises
        ------
        ValueError
            If the key does not correspond to a registered builder.
        """
        builder = self.builders.get(key)        
        if not builder:
            raise ValueError(key)        
        return builder(**kwargs)
        
    def initialize(self, item):
        """Initialize the list of fitting functions.

        Parameters
        ----------
        item : QWidget
            Add known fit methods to a widget. Practically this is a QComboBox
            in the FitPanel.
        """
        if DEBUG:
            print("Initializing fit functions:")        
        for key in self.builders:
            item.addItem(key)
            if DEBUG:
                print("  Added:", key)
