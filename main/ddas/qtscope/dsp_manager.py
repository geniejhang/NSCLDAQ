import logging
import pandas as pd
import inspect

from pixie_utilities import DSPUtilities
import xia_constants as xia

class DSPManager:
    """Internal DSP parameter management class. 

    DSP parameters are stored internally in a dictionary keyed by the module 
    number. Each dictionary value corresponds to a complete set of DSP 
    parameters which are addressable by the code. The dataframe chan_par is an 
    nchannel x nChanDSP dataframe of channel DSP parameters and the dataframe 
    mod_par is a 1 x nModDSP dataframe of module DSP parameters. Channel and 
    module DSP parameters are defined by XIA.The rest of the system uses this 
    management class to modify the internal DSP parameter values stroed in the 
    dataframe as well as to read/write values to/from the modules via 
    getters/setters and the  DSPUtilities class.

    Attributes
    ----------
    _dsp : dict
        Internal storage of DSP parameters. DSP parameters are stored in a 
        nested dictionary of dataframes.
    _utils : DSPUtilities
        XIA API interface utilities to read/write information to/from modules.
    _dsp_deps : dict
        Dictionary of parameter values which depend on the value of other 
        parameters. Key is the i.d. parameter name and the value a list of 
        dependant parameter names.
    _nmodules : int
        Number of modules installed in the crate.
    _nchannels : int 
        Number of channels per module.

    Methods
    -------
    initialize_dsp(nmod, nchan): 
        Create and fill the DSP dictionary.
    get_chan_par(mod, chan, pname): 
        Get a channel parameter value from the dataframe.
    set_chan_par(mod, chan, pname, value): 
        Set a channel parameter value in the dataframe.
    get_mod_par(mod, pname): 
        Get a module parameter value from the dataframe.
    set_mod_par(mod, pname, value): 
        Set a module parameter value in the dataframe.
    read(mod, pnames): 
        Read DSP parameter(s) into the dataframe.
    write(mod, pnames): 
        Write DSP parameter(s) from the dataframe.
    adjust_offsets(mod): 
        Adjust the DC offset of all channels on a single module.
    load_new_dsp(): 
        Load new DSP settings into the dataframe.
    """

    def __init__(self):
        """DSPManager class constructor."""
        self._utils = DSPUtilities()

        # Set dependent paramters e.g. TRIGGER_THRESHOLD is automatically
        # adjusted by the API when TRIGGER_RISETIME is. Prevents Principle
        # of Least Astonishment violations.      
        self._dsp_deps = {
            "TRIGGER_RISETIME": ["TRIGGER_THRESHOLD"],
            "SLOW_FILTER_RANGE": ["ENERGY_RISETIME", "ENERGY_FLATTOP"]
        }

    def initialize_dsp(self, nmod, nchan=16):
        """Initialize the DSP dataframe. 

        Read DSP parameters from the modules into the dataframe storage.

        Parameters
        ----------
        nmod : int 
            Number of modules installed in the system.
        nchan : int, default=16
            Number of channels per module.
        """
        self._dsp = {}
        
        self._nmodules = nmod
        self._nchannels = nchan
        
        for i in range(self._nmodules):            
            self._dsp[i] = {}  # dict of dicts. Key is module number.
            
            # Load channel parameters:            
            cpars = {}
            for pname in xia.CHAN_PARS:
                p = []
                for j in range(self._nchannels):
                    p.append(self._utils.read_chan_par(i, j, pname))
                cpars[pname] = p
            self._dsp[i]["chan_par"] = pd.DataFrame.from_dict(cpars)                     
            # Load module parameters:            
            mpars = {}
            for pname in xia.MOD_PARS:
                mpars[pname] = [self._utils.read_mod_par(i, pname)]
            self._dsp[i]["mod_par"] = pd.DataFrame.from_dict(mpars)

    def get_chan_par(self, mod, chan, pname):
        """Get a channel parameter value from the dataframe.

        Parameters
        ----------
        mod : int 
            Module number.
        chan : int 
            Channel number on the module.
        pname : str 
            Channel parameter name.

        Returns
        -------
        float
            Channel parameter value.
        None
            Channel parameter name is unknown.

        Raises
        ------
        ValueError
            Channel parameter name is unknown.
        """
        try:
            if pname not in xia.CHAN_PARS:
                raise ValueError(f"{pname} is not a channel paramter name")
        except ValueError as e:
            print(
                "{}:{}: Caught exception -- {}.".format(
                    self.__class__.__name__,
                    inspect.currentframe().f_code.co_name,
                    e
                )
            )
            return None
        else:
            return self._dsp[mod]["chan_par"].at[chan, pname]
    
    def set_chan_par(self, mod, chan, pname, value):
        """Set a channel parameter value in the dataframe. 

        Ensure the passed value is floating point before writing.

        Parameters
        ----------
        mod : int 
            Module number.
        chan : int 
            Channel number on the module.
        pname : str 
            Channel parameter name.
        value : float 
            Channel parameter value to set.

        Raises
        ------
        ValueError 
            Channel parameter name is unknown.
        """
        if type(value) is not float:
            print(f"WARNING -- {pname} value {value} is type {type(value)}, converting to float.")
            value = float(value)
        
        try:
            if pname not in xia.CHAN_PARS:
                raise ValueError(f"{pname} is not a channel paramter name")
        except ValueError as e:
            print(
                "{}:{}: Caught exception -- {}.".format(
                    self.__class__.__name__,
                    inspect.currentframe().f_code.co_name,
                    e
                )
            )
        else:
            self._dsp[mod]["chan_par"].at[chan, pname] = value
    
    def get_mod_par(self, mod, pname):
        """Get a module parameter value from the dataframe.

        Parameters
        ----------
        mod : int 
            Module number.
        pname : str 
            Module parameter name.

        Returns
        -------
        int  
            Module parameter value.
        None 
            Module parameter name is unknown.

        Raises
        ------
        ValueError 
            Module parameter name is unknown.
        """        
        try:
            if pname not in xia.MOD_PARS:
                raise ValueError(
                    "{} is not a module paramter name".format(pname)
                )
        except ValueError as e:
            print("{}:{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            return None
        else:        
            return self._dsp[mod]["mod_par"].at[0, pname]

    
    def set_mod_par(self, mod, pname, value):
        """Set a module parameter value in the dataframe.

        Parameters
        ----------
        mod : int 
            Module number.
        pname : str 
            Module parameter name.
        value : int 
            Module parameter value to set.

        Raises
        ------
        ValueError 
            Module parameter name is unknown.
        """
        if type(value) is not int:
            print(f"WARNING -- {pname} value {value} is type {type(value)}, converting to int.")
            value = int(value)
        
        try:
            if pname not in xia.MOD_PARS:
                raise ValueError(f"{pname} is not a channel module name")
        except ValueError as e:
            print("{}:{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
        else:
            self._dsp[mod]["mod_par"].at[0, pname] = value

    def read(self, mod, pnames):
        """Read DSP settings into internal storage.

        Parameters
        ----------
        mod : int 
            Module number.
        pnames : list
            List of XIA DSP parameter names.
        """        
        for p in pnames:
            if p in xia.CHAN_PARS:
                for i in range(self._nchannels):
                    val = self._utils.read_chan_par(mod, i, p)
                    self.set_chan_par(mod, i, p, val)
                    
                    logging.getLogger("qtscope_logger").debug(
                        "{}.{}: Read {} {} {} {}".format(
                            self.__class__.__name__,
                            inspect.currentframe().f_code.co_name,
                            mod, i, p, val
                        )
                    )
                        
            elif p in xia.MOD_PARS:
                val = self._utils.read_mod_par(mod, p)
                self.set_mod_par(mod, p, val)
                
                logging.getLogger("qtscope_logger").debug(
                    "{}.{}: Read {} {} {}".format(
                        self.__class__.__name__,
                        inspect.currentframe().f_code.co_name,
                        mod, p, val
                    )
                )
                    
            else:                
                # @todo Exception handling for passing a bad parameter or do
                # we just let the API handle it?                
                pass

    def write(self, mod, pnames):
        """Write DSP settings from internal storage.

        Parameters
        ----------
        mod : int 
            Module number.
        pnames : list 
            List of XIA DSP parameter names.
        """        
        for p in pnames:
            if p in xia.CHAN_PARS:
                for i in range(self._nchannels):
                    val = self.get_chan_par(mod, i, p)
                    
                    logging.getLogger("qtscope_logger").debug(
                        "{}.{}: Write {} {} {} {}".format(
                            self.__class__.__name__,
                            inspect.currentframe().f_code.co_name,
                            mod, i, p, val
                        )
                    )
                        
                    # Write, read back, and set parameters:
                    self._utils.write_chan_par(mod, i, p, val)
                    val = self._utils.read_chan_par(mod, i, p)
                    self.set_chan_par(mod, i, p, val)
                    
                    # If this parameter has dependents, write, read, and set
                    # as well. We know dependent parameters are always channel
                    # parameters:                    
                    if p in self._dsp_deps:
                        for dp in self._dsp_deps[p]:
                            val = self.get_chan_par(mod, i, dp)
                            
                            logging.getLogger("qtscope_logger").debug(
                                "{}.{}: Write deps  {} {} {} {}".format(
                                    self.__class__.__name__,
                                    inspect.currentframe().f_code.co_name,
                                    mod, i, dp, val
                                )
                            )
                            
                            self._utils.write_chan_par(mod, i, dp, val)
                            val = self._utils.read_chan_par(mod, i, dp)
                            self.set_chan_par(mod, i, dp, val)
            elif p in xia.MOD_PARS:               
                val = self.get_mod_par(mod, p)
                
                logging.getLogger("qtscope_logger").debug(
                    "{}.{}: Write {} {} {}".format(
                        self.__class__.__name__,
                        inspect.currentframe().f_code.co_name,
                        mod, p, val
                    )
                )
                    
                # Write, read back, and set parameters:                
                if p == "SLOW_FILTER_RANGE":
                    # Slow filter range recalculates filter parameters and
                    # is slow, only write if changed. @todo: Only write the
                    # changed parameters.                    
                    current_val = self._utils.read_mod_par(mod, p)
                    if current_val != val:
                        self._utils.write_mod_par(mod, p, val)
                        print("Module {}: New energy filter range = {} -- filter parameters may have changed!".format(mod, val))
                else:
                    self._utils.write_mod_par(mod, p, val)                
                val = self._utils.read_mod_par(mod, p)
                self.set_mod_par(mod, p, val)  

                # If this parameter has dependents, write, read, and set
                # as well. We know dependent parameters are always channel
                # parameters:                                    
                for i in range(self._nchannels):
                    if p in self._dsp_deps:
                        for dp in self._dsp_deps[p]:
                            val = self.get_chan_par(mod, i, dp)
                            
                            logging.getLogger("qtscope_logger").debug(
                                "{}.{}: Write deps {} {} {} {}".format(
                                    self.__class__.__name__,
                                    inspect.currentframe().f_code.co_name,
                                    mod, i, dp, val
                                )
                            )
                            
                            self._utils.write_chan_par(mod, i, dp, val)
                            val = self._utils.read_chan_par(mod, i, dp)
                            self.set_chan_par(mod, i, dp, val)
            else:
                # @todo Exception handling for passing a bad parameter or do
                # we just let the API handle it?
                pass

    def adjust_offsets(self, mod):
        """Adjust DC offsets for all channels on a single module.

        Parameters
        ----------
        mod : int 
            Module number.
        """        
        self._utils.adjust_offsets(mod)
            
    def load_new_dsp(self):
        """Load a new set of DSP settings into the dataframe.

        Reloads DSP settings from the module into the dataframe. Most common 
        use case is the user loads a new settings file after booting the 
        system. DSP settings are loaded on the modules without the need for a 
        reboot, but the dataframe will hold the old values until this function 
        is called.
        """        
        for mod in range(self._nmodules):
            self.read(mod, [*xia.CHAN_PARS, *xia.MOD_PARS])
