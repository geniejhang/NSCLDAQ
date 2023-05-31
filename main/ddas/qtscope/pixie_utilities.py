import sys
from ctypes import *
import inspect

from run_type import RunType
from converters import str2char
import xia_constants as xia

"""pixie_utilities.py

The libPixieUtilities.so library contains a set of utilities to read and write 
DSP parameters, start and stop data runs, acquire traces, etc. on the modules 
using the XIA API. This module defines a set of Python classes which interact 
with elements of the XIA Pixie-16 API via the provided shared library using the
Python ctypes interface.

Classes
-------
SystemUtilities 
    Python wrapper for running a 'system' of modules: boot, load/save settings,
    exit, etc. and reading system configuration information.
DSPUtilities
    Python wrapper for reading and writing DSP settings to modules.
RunUtilities
    Python wrapper for managing run states and getting run data from modules.
TraceUtilities
    Python wrapper for reading and analyzing trace data.

"""

lib = CDLL("libPixieUtilities.so") # Must be in LD_LIBRARY_PATH.

##########################################################################
# SystemUtilities
#
    
class SystemUtilities:
    """Python SystemUtilities.

    Python wrapper for running a 'system' of modules: boot, load/save settings,
    exit, etc. and reading system configuration information.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the SystemUtilities object.

    Methods:
    boot()
        Boot the system.
    save_set_file(name)
        Save an XIA settings to file called name.
    load_set_file(name)
        Load XIA settings file called name.
    exit_system() 
        Release resources used by the modules prior to exit.
    boot_offline(offline)
        Set the system boot mode.
    get_boot_mode() 
        Get the system boot mode.
    get_boot_status()
        Get the system boot status.
    get_num_modules() 
        Get the number of installed modules.
    get_module_msps()
        Get the sampling rate in MSPS for module.
    """
    
    def __init__(self):
        """SystemUtilities class constructor."""        
        # Ctor:        
        lib.CPixieSystemUtilities_new.restype = POINTER(c_char)
        
        # Boot:
        lib.CPixieSystemUtilities_Boot.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_Boot.restype = c_int
        
        # Save set file:        
        lib.CPixieSystemUtilities_SaveSetFile.argtypes = [c_void_p, c_char_p]
        lib.CPixieSystemUtilities_SaveSetFile.restype = c_int

        # Load set file:        
        lib.CPixieSystemUtilities_LoadSetFile.argtypes = [c_void_p, c_char_p]
        lib.CPixieSystemUtilities_LoadSetFile.restype = c_int
        
        # Exit system:        
        lib.CPixieSystemUtilities_ExitSystem.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_ExitSystem.restype = c_int
        
        # Set boot mode:        
        lib.CPixieSystemUtilities_SetBootMode.argtypes = [c_void_p, c_int]
        lib.CPixieSystemUtilities_SetBootMode.restype = c_void_p

        # Get boot mode:        
        lib.CPixieSystemUtilities_GetBootMode.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_GetBootMode.restype = c_int
        
        # Get boot status:        
        lib.CPixieSystemUtilities_GetBootStatus.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_GetBootStatus.restype = c_bool
        
        # Get number of modules:        
        lib.CPixieSystemUtilities_GetNumModules.argtypes = [c_void_p]
        lib.CPixieSystemUtilities_GetNumModules.restype = c_ushort
        
        # Get module MSPS:        
        lib.CPixieSystemUtilities_GetModuleMSPS.argtypes = [c_void_p, c_int]
        lib.CPixieSystemUtilities_GetModuleMSPS.restype = c_ushort
        
        # Dtor:        
        lib.CPixieSystemUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieSystemUtilities_new()
        
    def boot(self):
        """Wrapper function to system boot.
        
        Raises
        ------
        RuntimeError
            If the boot fails.
        """        
        try: 
            retval = lib.CPixieSystemUtilities_Boot(self.obj)       
            if retval < 0:
                raise RuntimeError(
                    "System boot failed with retval {}".format(retval)
                )
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            
    def save_set_file(self, name):
        """Wrapper function to save an XIA settings file.

        Parameters
        ----------
        name : str
            Name of the file to save.

        Raises
        ------
        RuntimeError
            If the save operation fails.
        """        
        try: 
            retval = lib.CPixieSystemUtilities_SaveSetFile(
                self.obj, str2char(name)
            )            
            if retval < 0:
                raise RuntimeError(
                    "Save settings file failed with retval {}".format(retval)
                )            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))    
            
    def load_set_file(self, name):
        """Wrapper function to load an XIA settings file.

        Parameters
        ----------
        name : str
            Name of the file to load.

        Raises
        ------
        RuntimeError
            If the load operation fails.
        """        
        try: 
            retval = lib.CPixieSystemUtilities_LoadSetFile(
                self.obj, str2char(name)
            )            
            if retval < 0:
                raise RuntimeError(
                    "Load settings file failed with retval {}".format(retval)
                )            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))  
    
    def exit_system(self):
        """Wrapper for system exit. 

        Releases resources used by the modules.

        Raises
        ------
        RuntimeError
            If a module fails to exit properly.
        """
        try: 
            retval = lib.CPixieSystemUtilities_ExitSystem(self.obj)            
            if retval < 0:
                raise RuntimeError(
                    "System exit failed with retval {}".format(retval)
                )            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e)) 
    
    def boot_offline(self, offline=False):
        """Wrapper to set system boot mode.

        Boot modules in offline mode with no attached hardware or online 
        mode with hardware. Offline boot mode is configured by reading the 
        value of the envoironment variable QTSCOPE_OFFLINE at execution.

        Parameters
        ----------
        offline : bool, default=False
            If True, boot the system in offline mode.

        Note
        ----
        Offline boot only supported in XIA API 2 as of 5/26/23.
        """
        boot_mode = 0  # Value of OfflineMode in Pixie16InitSystem().
        if offline:
            boot_mode = 1  # 1 == OFFLINE
        return lib.CPixieSystemUtilities_SetBootMode(self.obj, boot_mode)

    def get_boot_mode(self):
        """Wrapper to get system boot mode (online or offline).

        Returns
        -------
        int
            Offline (1) or online (0) module boot mode flag.
        """        
        return lib.CPixieSystemUtilities_GetBootMode(self.obj)
    
    def get_boot_status(self):
        """Wrapper to get the boot status of the system.

        Returns
        -------
        bool
            True if the system has been booted, otherwise False.
        """        
        return lib.CPixieSystemUtilities_GetBootStatus(self.obj)
    
    def get_num_modules(self):
        """Wrapper to get the number of modules present in the system.

        Returns
        -------
        int
            Number of modules installed in the system.
        """        
        return lib.CPixieSystemUtilities_GetNumModules(self.obj)

    def get_module_msps(self, module):
        """Wrapper to read ADC sampling rate in MSPS from a module.

        Returns
        -------
        int
            Sampling rate in MSPS.
        """        
        return lib.CPixieSystemUtilities_GetModuleMSPS(self.obj, module)
    
    def __del__(self):
        """SystemUtilities class destructor."""        
        return lib.CPixieSystemUtilities_delete(self.obj)

##########################################################################
# DSP Utilities
#

class DSPUtilities:
    """Python DSPUtilities.

    Python wrapper for reading and writing DSP settings to modules.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the DSPUtilities object.

    Methods
    -------
    adjust_offsets(module) 
        Adjust DC offsets on a single module.
    write_chan_par(module, channel, name, val) 
        Write a channel parameter.
    read_chan_par(module, channel, name) 
        Read a channel parameter.
    write_mod_par(module, name, val) 
        Write a module parameter.
    read_mod_par(module, name) 
        Read a module parameter.
    """
    
    def __init__(self):
        """DSPUtilities class constructor"""        
        # Ctor:        
        lib.CPixieDSPUtilities_new.restype = POINTER(c_char)

        # Adjust offsets:        
        lib.CPixieDSPUtilities_AdjustOffsets.argtypes = [c_void_p, c_int]
        lib.CPixieDSPUtilities_AdjustOffsets.restype = c_int

        # Write channel parameter:        
        lib.CPixieDSPUtilities_WriteChanPar.argtypes = [
            c_void_p, c_int, c_int, c_char_p, c_double
        ]
        lib.CPixieDSPUtilities_WriteChanPar.restype = c_int

        # Read channel parameter:        
        lib.CPixieDSPUtilities_ReadChanPar.argtypes = [
            c_void_p, c_int, c_int, c_char_p, POINTER(c_double)
        ]
        lib.CPixieDSPUtilities_ReadChanPar.restype = c_int

        # Write module parameter:        
        lib.CPixieDSPUtilities_WriteModPar.argtypes = [
            c_void_p, c_int, c_char_p, c_uint
        ]        
        lib.CPixieDSPUtilities_WriteModPar.restype = c_int

        # Read module parameter:        
        lib.CPixieDSPUtilities_ReadModPar.argtypes = [
            c_void_p, c_int, c_char_p, POINTER(c_uint)
        ]
        lib.CPixieDSPUtilities_ReadModPar.restype = c_int
        
        # Dtor:        
        lib.CPixieDSPUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieDSPUtilities_new()
    
    def adjust_offsets(self, module):
        """Wrapper to adjust DC offsets for all channels on a given module.

        Parameters
        ----------
        module : int 
            Module number.

        Raises
        ------
        RuntimeError 
            If the offset adjustment fails.
        """        
        try:
            retval = lib.CPixieDSPUtilities_AdjustOffsets(self.obj, module)
            if retval < 0:
                raise RuntimeError("Failed to adjust offsets in Mod. {} with retval {}".format(module, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))            
        
    def write_chan_par(self, module, channel, name, val):
        """Wrapper to write a channel parameter to a module.

        The parameter name is converted from Python string to char*.

        Parameters
        ----------
        module : int 
            Module number.
        channel : int 
            Channel number.
        name : str 
            Parameter name.
        val : float 
            Channel parameter value.

        Raises
        ------
        RuntimeError
            If the write operation fails.
        """        
        try:
            retval = lib.CPixieDSPUtilities_WriteChanPar(
                self.obj, module, channel, str2char(name), val
            )            
            if retval < 0:
                raise RuntimeError("Failed to write parameter {} to Mod. {}, Ch. {} with retval {}".format(name, module, channel, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))     
    
    def read_chan_par(self, module, channel, name):
        """Wrapper to read a channel parameter from a module.

        The parameter name is converted from Python string to char*.

        Parameters
        ----------
        module : int
            Module number.
        channel : int
            Channel number.
        name : str
            Parameter name.

        Returns
        -------
        float
            Value of the read parameter if success.
        None
            If exception is raised.

        Raises
        ------
        RuntimeError
            If the read operation fails.
        """        
        read_param = c_double()        
        try:
            retval = lib.CPixieDSPUtilities_ReadChanPar(
                self.obj, module, channel, str2char(name), byref(read_param)
            )            
            if retval < 0:
                raise RuntimeError("Failed to read parameter {} from Mod. {}, Ch. {} with retval {}".format(name, module, channel, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            return None        
        else:
            return read_param.value
    
    def write_mod_par(self, module, name, val):
        """Wrapper to write a module parameter.

        The parameter name is converted from Python string to char*, parameter 
        value is converted to an int.

        Parameters
        ----------
        module : int
            Module number.
        name : str
            Parameter name.
        val : float
            Channel parameter value.

        Raises
        ------
        RuntimeError
            If the write operation fails.
        """        
        try:
            retval = lib.CPixieDSPUtilities_WriteModPar(
                self.obj, module, str2char(name), int(val)
            )            
            if retval < 0:
                raise RuntimeError("Failed to write parameter {} to Mod. {} with retval {}".format(name, module, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))             
        
    def read_mod_par(self, module, name):
        """Wrapper to read a module parameter.

        The parameter name is converted from Python string to char*.

        Parameters
        ----------
        module : int
            Module number.
        name : str
            Parameter name.

        Returns
        -------
        int
            Value of the read parameter if success.
        None
            If exception is raised.

        Raises
        ------
        RuntimeError
            If the read operation fails.
        """        
        read_param = c_uint()        
        try:
            retval = lib.CPixieDSPUtilities_ReadModPar(
                self.obj, module, str2char(name), byref(read_param)
            )            
            if retval < 0:
                raise RuntimeError("Failed to read paramter {} from Mod. {} with retval {}".format(name, module, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            return None        
        else:
            return read_param.value
            
    def __del__(self):
        """DSPUtilities destructor."""        
        return lib.CPixieDSPUtilities_delete(self.obj)

##########################################################################
# RunUtilities
#
    
class RunUtilities:
    """Python wrapper for managing run states and getting run data.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the RunUtilities object.

    Methods
    -------
    begin_run(module, run_type) 
        Begin a run_type run in a single module.
    end_run(module, run_type) 
        End a run_type run in a single module.
    read_data(module, channel, run_type) 
        Read run_type run data for a single module.
    read_stats(module) 
        Read run statistics from the specified module.
    get_data(run_type) 
        Get single channel run_type data.
    get_run_active() 
        Get the active run status of the system.
    use_generator_data(mode) 
        Set ParameterManager offline mode.
    """
    
    def __init__(self):
        """RunUtilities class constructor."""
        # Ctor:        
        lib.CPixieRunUtilities_new.restype = POINTER(c_char)
        
        # Begin histogram data run:        
        lib.CPixieRunUtilities_BeginHistogramRun.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_BeginHistogramRun.restype = c_int
        
        # End histogram data run:        
        lib.CPixieRunUtilities_EndHistogramRun.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_EndHistogramRun.restype = c_int
        
        # Read histogram from module:        
        lib.CPixieRunUtilities_ReadHistogram.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieRunUtilities_ReadHistogram.restype = c_int
        
        # Begin baseline data run:        
        lib.CPixieRunUtilities_BeginBaselineRun.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_BeginBaselineRun.restype = c_int
        
        # End baseline data run:        
        lib.CPixieRunUtilities_EndBaselineRun.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_EndBaselineRun.restype = c_int
        
        # Read baseline from module:        
        lib.CPixieRunUtilities_ReadBaseline.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieRunUtilities_ReadBaseline.restype = c_int
        
        # Read run statistics from module:        
        lib.CPixieRunUtilities_ReadModuleStats.argtypes = [c_void_p, c_int]
        lib.CPixieRunUtilities_ReadModuleStats.restype = c_int  

        # Returns a pointer to the underlying histogram data from the vector:
        lib.CPixieRunUtilities_GetHistogramData.argtypes = [c_void_p]
        lib.CPixieRunUtilities_GetHistogramData.restype = POINTER(
            c_uint * xia.MAX_HISTOGRAM_LENGTH
        )
        
        # Returns a pointer to the underlying baseline data from the vector:
        lib.CPixieRunUtilities_GetBaselineData.argtypes = [c_void_p]
        lib.CPixieRunUtilities_GetBaselineData.restype = POINTER(
            c_uint * xia.MAX_HISTOGRAM_LENGTH
        )

        # Run active status:        
        lib.CPixieRunUtilities_GetRunActive.argtypes = [c_void_p]
        lib.CPixieRunUtilities_GetRunActive.restype = c_bool
        
        # Use generator data:        
        lib.CPixieRunUtilities_SetUseGenerator.argtypes = [c_void_p, c_bool]
        lib.CPixieRunUtilities_SetUseGenerator.restype = c_void_p

        # Dtor:
        lib.CPixieRunUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieRunUtilities_new()
    
    def begin_run(self, module, run_type):
        """Wrapper to begin a histogram run in a single module.

        Parameters
        ----------
        module : int 
            Module number.
        run_type : Enum member 
            Type of run to begin.

        Raises
        ------
        ValueError
            If the run mode is invalid.
        RuntimeError
            If the start run operation fails.
        """        
        try:
            if run_type == RunType.HISTOGRAM:
                retval = lib.CPixieRunUtilities_BeginHistogramRun(
                    self.obj, module
                )     
                if retval < 0:
                    raise RuntimeError("Begin histogram run in Mod. {} failed with retval {}".format(retval, module))                
            elif run_type == RunType.BASELINE:                
                retval =  lib.CPixieRunUtilities_BeginBaselineRun(
                    self.obj, module
                )
                if retval < 0:
                    raise RuntimeError("Begin baseline run in Mod. {} failed with retval {}".format(retval, module))                
            else:
                raise ValueError("Unable to begin run in Mod. {}, run type {} is not a valid type of data run".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, run_type))            
        except ValueError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))    
            
    def end_run(self, module, run_type):
        """Wrapper to end a histogram run in a single module.
        
        Parameters
        ----------
        module : int
            Module number.
        run_type : Enum member
            Type of run to begin.

        Raises
        ------
        ValueError
            If the run mode is invalid.
        """        
        try:
            if run_type == RunType.HISTOGRAM:
                lib.CPixieRunUtilities_EndHistogramRun(self.obj, module)  
            elif run_type == RunType.BASELINE:
                lib.CPixieRunUtilities_EndBaselineRun(self.obj, module)
            else:
                raise ValueError("{}.{}: Unable to end run in Mod. {}, run type {} is not a valid type of data run.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, run_type))            
        except ValueError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))
            
    def read_data(self, module, channel, run_type):
        """Wrapper to read run data from a single channel.

        Parameters
        ----------
        module : int
            Module number.
        channel : int
            Channel number.
        run_type : Enum member
            Type of run data to read.

        Raises
        ------
        ValueError
            If the run mode is invalid.
        RuntimeError 
            If the API data read fails.
        """            
        try:            
            if run_type == RunType.HISTOGRAM:            
                retval =  lib.CPixieRunUtilities_ReadHistogram(
                    self.obj, module, channel
                )
                if retval < 0:
                    raise RuntimeError("Histogram read from Mod. {}, Ch. {} failed with retval {}".format(module, channel, retval))                
            elif run_type == RunType.BASELINE:
                retval = lib.CPixieRunUtilities_ReadBaseline(
                    self.obj, module, channel
                )
                if retval < 0:
                    raise RuntimeError("Baseline read from Mod. {}, Ch. {} failed with retval {}".format(module, channel, retval))                
            else:
                raise ValueError("{}.{}: unable to read data from Mod. {}, run type {} is not a valid type of data run".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, module, run_type))            
        except ValueError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))

    def read_stats(self, module):
        """Wrapper to read the run statistics from a single module.

        Parameters
        ----------
        module : int
            Module number.

        Raises
        ------
        RuntimeError
            If the stats read fails.
        """
        try: 
            retval = lib.CPixieRunUtilities_ReadModuleStats(self.obj, module)   
            if retval < 0:
                raise RuntimeError("Reading statistics from Mod. {} failed with retval {}".format(module, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))    
    
    def get_data(self, run_type):
        """
        Wrapper to provide access the acquired energy histogram data.

        Parameters
        ----------
        run_type : Enum member
            Type of run data to retrieve.
        
        Returns
        -------
        array
            Array of list-mode histogram or baseline run data.
        """        
        if run_type == RunType.HISTOGRAM:
            return lib.CPixieRunUtilities_GetHistogramData(self.obj).contents
        elif run_type == RunType.BASELINE:
            return lib.CPixieRunUtilities_GetBaselineData(self.obj).contents
                
    def get_run_active(self):
        """Wrapper to get the active run status.

        Returns
        -------
        bool
            True if a run is active, False otherwise.
        """        
        return lib.CPixieRunUtilities_GetRunActive(self.obj)

    def use_generator_data(self, mode):
        """Wrapper to set the manager to use generated data.

        Parameters
        ----------
        mode : bool
            True to enable generated data.
        """        
        return lib.CPixieRunUtilities_SetUseGenerator(self.obj, mode)

    def __del__(self):
        """RunUtilities class destructor."""        
        return lib.CPixieRunUtilities_delete(self.obj)

##########################################################################
# TraceUtilities
#

class TraceUtilities:
    """Python wrapper for reading and analyzing trace data.

    Attributes
    ----------
    obj : POINTER(c_char)
        Handle for the TraceUtilities object.

    Methods
    -------
    read_trace(module, channel) 
        Read trace from module/channel.
    read_fast_trace(module, channel) 
        Read unvalidated trace from module/channel.
    get_trace_data() 
        Access the trace data.
    use_generator_data(mode) 
        Set use of trace data generator to bool value for testing.
    """
    
    def __init__(self):
        """TraceUtilities constructor."""
        # Ctor:        
        lib.CPixieTraceUtilities_new.restype = POINTER(c_char)

        # Read trace from module:        
        lib.CPixieTraceUtilities_ReadTrace.argtypes = [c_void_p, c_int, c_int]
        lib.CPixieTraceUtilities_ReadTrace.restype = c_int
        
        # Read trace from module without signal validation:        
        lib.CPixieTraceUtilities_ReadFastTrace.argtypes = [
            c_void_p, c_int, c_int
        ]
        lib.CPixieTraceUtilities_ReadFastTrace.restype = c_int

        # Returns a pointer to the underlying trace data from the vector:
        lib.CPixieTraceUtilities_GetTraceData.argtypes = [c_void_p]
        lib.CPixieTraceUtilities_GetTraceData.restype = POINTER(
            c_ushort * xia.MAX_ADC_TRACE_LEN
        )
        
        # Use generator data:        
        lib.CPixieTraceUtilities_SetUseGenerator.argtypes = [c_void_p, c_bool]
        lib.CPixieTraceUtilities_SetUseGenerator.restype = c_void_p

        # Dtor:        
        lib.CPixieTraceUtilities_delete.argtypes = [POINTER(c_char)]

        self.obj = lib.CPixieTraceUtilities_new()
    
    def read_trace(self, module, channel):
        """Wrapper to read a trace from a single channel.

        Parameters
        ----------
        module : int 
            Module number.
        channel : int 
            Channel number.

        Raises
        ------
        RuntimeError
            If the trace cannot be read.
        """
        try:
            retval = lib.CPixieTraceUtilities_ReadTrace(
                self.obj, module, channel
            )            
            if retval < 0:
                raise RuntimeError("Read trace from Mod. {}, Ch. {} failed with retval {}".format(module, channel, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}.".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))        
            
    def read_fast_trace(self, module, channel):
        """Wrapper to read an unvalidated trace from an single channel.

        Parameters
        ----------
        module : int 
            Module number.
        channel : int 
            Channel number.

        Raises
        ------
        RuntimeError
            If the trace cannot be read.
        """        
        try:
            retval = lib.CPixieTraceUtilities_ReadFastTrace(
                self.obj, module, channel
            )            
            if retval < 0:
                raise RuntimeError("Read trace from Mod. {}, Ch. {} failed with retval {}".format(module, channel, retval))            
        except RuntimeError as e:
            print("{}.{}: Caught exception -- {}".format(self.__class__.__name__, inspect.currentframe().f_code.co_name, e))    
    
    def get_trace_data(self):
        """Wrapper to provide access the acquired trace data.

        Returns
        -------
        array 
            Python array of trace data.
        """        
        return lib.CPixieTraceUtilities_GetTraceData(self.obj).contents

    def use_generator_data(self, mode):
        """ Wrapper to set the manager to use generated data.

        Parameters
        ----------
        mode : bool
            True to enable generated data.
        """        
        return lib.CPixieTraceUtilities_SetUseGenerator(self.obj, mode) 
    
    def __del__(self):
        """TraceUtilities destructor. Deletes itself."""        
        return lib.CPixieTraceUtilities_delete(self.obj)
