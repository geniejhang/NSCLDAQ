import inspect
import logging
import math
import numpy as np
from dataclasses import dataclass    

# @todo This class needs to know the module MSPS so it can set the fixed values
# for the CFD parameters. Probably the easiest way is to have some module
# configuration information known by the DSP manager which can be accessed by
# this class.

@dataclass
class FilterParameters:
    xdt: float = 0.0
    fast_risetime: int = 0
    fast_gap: int = 0
    cfd_delay: int = 0
    cfd_scale: int = 0
    slow_risetime: int = 0
    slow_gap: int = 0
    tau: int = 0

class TraceAnalyzer:
    """TraceAnalyzer class.
    
    Provides an interface for calculating filter (time, CFD, energy) output 
    for DDAS traces based on the channel DSP settings.

    Attributes
    ----------
    logger : Logger
        QtScope Logger object.
    dsp_mgr : DSPManager
        Manager for internal DSP and interface for XIA API read/write 
        operations.
    trace : array 
        Single channel ADC trace.
    fast_filter : list 
        Fast filter output calcualted from the trace.
    cfd : list 
        CFD output calculated from the fast filter.
    slow_filter  : list 
        Slow filter output calculated from the trace.
    """
    
    def __init__(self, mgr):
        """TraceAnalyzer constructor.
        
        Parameters
        ----------
        dsp_mgr : DSPManager 
            Manager for internal DSP and interface for XIA API read/write 
            operations.
        """
        self.dsp_mgr = mgr
        self.logger = logging.getLogger("qtscope_logger")
        
        self.trace = None
        self.fast_filter = None
        self.cfd = None
        self.slow_filter = None

    def analyze(self, mod, chan, trace):
        """Call other analyzers to calculate filter output.

        Parameters
        ----------        
        mod : int  
            Module number.
        chan : int 
            Channel number.
        trace : array 
            Single channel ADC trace.

        Raises
        ------
            Every exception back to the caller.
        """
        self.trace = trace        
        try:
            self._compute_filters(mod, chan)
        except:
            raise

    ##
    # Private methods
    #
    
    def _compute_filters(self, mod, chan):
        """Compute fast, CFD, and slow filter output.

        Parameters
        ----------
        mod : int 
            Module number.
        chan : int
            Channel number.

        Raises
        ------
        ValueError
            If the stored trace is empty.
        """        
        if not self.trace:
            raise ValueError("Trace is empty, cannot compute filters")
        
        filter_params = self._get_filter_parameters(mod, chan)
        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: {filter_params.__repr__()}")
        
        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Calculating fast filter output for trace from Mod. {mod} Ch. {chan}")
        self._compute_fast_filter(filter_params)

        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Calculating CFD output for fast filter output from Mod. {mod} Ch. {chan}")
        self._compute_cfd(filter_params)

        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Calculating slow filter output for trace from Mod. {mod} Ch. {chan}")
        self._compute_slow_filter(filter_params)
        
    def _compute_fast_filter(self, fp):
        """Compute the fast filter output.

        Compute fast (timing) filter for a single channel ADC trace.
        
        Parameters
        ----------
        fp : FilterParameters
            Filter parameters for this channel.
        """        
        self.fast_filter = [0.0] * len(self.trace)
        
        # Calculate fast filter. See Pixie-16 User's Manual Sec. 3.3.8.1,
        # Eq. 3-1 for details.
       
        for i, _ in enumerate(self.trace):
            s0 = 0  # Trailing sum.
            s1 = 0  # Leading sum.
            if i - 2*fp.fast_risetime - fp.fast_gap + 1 >= 0:
                # +1 on high limit for inclusive sum:
                s0 = np.sum(self.trace[i-2*fp.fast_risetime-fp.fast_gap+1:i-fp.fast_risetime-fp.fast_gap+1])
                s1 = np.sum(self.trace[i-fp.fast_risetime+1:i+1])
                self.fast_filter[i] = s1 - s0
                    
    def _compute_cfd(self, fp):
        """Compute the CFD.

        Compute the CFD from the fast filter of a single channel ADC trace.

        Parameters
        ----------
        fp : FilterParameters
            Filter parameters for this channel.
        """        
        self.cfd = [0.0] * len(self.fast_filter)
        
        # Compute the CFD from the raw trace. See Pixie-16 User's Manual
        # Sec. 3.3.8.2. This method is preferred because some derived
        # parameters are fixed for 500 MSPS modules. See Eq. 3-5.

        # Adopting the Manual convention:
        w = 1 - fp.cfd_scale/8
        L = fp.fast_risetime - 1
        B = fp.fast_risetime + fp.fast_gap
        D = fp.cfd_delay

        for i, _ in enumerate(self.trace):
            s0, s1, s2, s3 = 0, 0, 0, 0
            k = i + D - L
            if ((k - D - B) >= 0) and ((k + L) < len(self.trace)):
                # +1 on high limit for inclusive sum:
                s0 = np.sum(self.trace[k:k+L+1])
                s1 = np.sum(self.trace[k-B:k-B+L+1])
                s2 = np.sum(self.trace[k-D:k-D+L+1])
                s3 = np.sum(self.trace[k-D-B:k-D-B+L+1])
                self.cfd[k] = w*(s0 - s1) - (s2 - s3)                

    def _compute_slow_filter(self, fp):
        """Compute the slow filter output.

        Notes
        -----
        Slow (energy) filter calculation for a single-channel ADC trace. For 
        more information on the slow filter calcualtion, see [1]_.

        References
        ----------
        .. [1] H. Tan et al., "A Fast Digital Filter Algorithm for Gamma-Ray 
        Spectroscopy With Double-Exponential Decaying Scintillators," IEEE 
        T. Nucl. Sci. 51 1541 (2004).

        Parameters
        ----------
        fp : FilterParameters
            Filter parameters for this channel.
        """        
        self.slow_filter = [0] * len(self.trace)

        # Guess a baseline value by averaging 5 samples at the start and end
        # of the trace and taking the minimum value:        
        baseline = min(np.mean(self.trace[:5]), np.mean(self.trace[-5:]))

        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Estimated baseline {baseline}")

        # Using notation from Tan unless otherwise noted, with time in samples:
        b1 = math.exp(-1/fp.tau)  # Ratio for geometric series sum Eq. 1.
        bL = math.pow(b1, fp.slow_risetime)
        
        # Coefficients of the inverse matrix Eq. 2 (example matrix elements
        # given on the bottom of pg. 1542):        
        a0 = bL/(bL - 1)
        ag = 1
        a1 = 1/(1 - bL)

        self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Ratio {b1:.3f}, coefficients {a0:.3f} {ag:.3f} {a1:.3f}")

        for i, _ in enumerate(self.trace):
            s0 = 0  # Trailing sum.
            sg = 0  # Gap sum.
            s1 = 0  # Leading sum.
            ilow = i - 2*fp.slow_risetime - fp.slow_gap + 1
            ihigh = ilow + fp.slow_risetime

            # ihigh+1 since sums are inclusive:
            
            if ilow >= 0:
                s0 = sum(self.trace[ilow:ihigh+1]-baseline)
                
                # If the trailing sum is computed, compute the gap and leading
                # sums if they do not run off the end of the trace:
                ilow = ihigh
                ihigh = ilow + fp.slow_gap                
                if ihigh < len(self.trace):
                    sg = sum(self.trace[ilow:ihigh+1] - baseline)
                    
                ilow = ihigh
                ihigh = ilow + fp.slow_risetime
                if ihigh < len(self.trace):
                    s1 = sum(self.trace[ilow:ihigh+1] - baseline)
                    # Compute the filter value if we have not run off the end
                    # of the trace for the leading sum:                    
                    self.slow_filter[i] = a0*s0 + ag*sg + a1*s1

                if i == len(self.trace)/2:
                    self.logger.debug(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Sums {s0:.1f} {sg:.1f} {s1:.1f} filter {self.slow_filter[i]:.1f}")

    def _get_filter_parameters(self, mod, chan):
        """Read the filter parameters from the module, convert them to the nearest integer value in samples, pack them into a FilterParameters class object and return it.
        """
        # Load DSP needed to calculate filters:
        xdt = self.dsp_mgr.get_chan_par(mod, chan, "XDT")
        fast_risetime = self.dsp_mgr.get_chan_par(
            mod, chan, "TRIGGER_RISETIME"
        )
        fast_gap = self.dsp_mgr.get_chan_par(mod, chan, "TRIGGER_FLATTOP")
        cfd_scale = self.dsp_mgr.get_chan_par(mod, chan, "CFDScale")
        cfd_delay = self.dsp_mgr.get_chan_par(mod, chan, "CFDDelay")
        slow_risetime = self.dsp_mgr.get_chan_par(mod, chan, "ENERGY_RISETIME")
        slow_gap = self.dsp_mgr.get_chan_par(mod, chan, "ENERGY_FLATTOP")
        tau = self.dsp_mgr.get_chan_par(mod, chan, "TAU")

        # Warn users that short filters may not display correctly:

        if (2*fast_risetime + fast_gap <= xdt):
            print(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: WARNING: Fast filter length {2*fast_risetime + fast_gap} <= XDT sampling {xdt}\n\tThe analyzed trace may not display properly!")

        if (2*slow_risetime + slow_gap <= xdt):
            print(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: WARNING: Slow filter length {2*slow_risetime + slow_gap} <= XDT sampling {xdt}\n\tThe analyzed trace may not display properly!")
        
        # Since we're stuck with XDT binning, round the filter parameters to
        # the nearest integer multiple of the XDT value to convert to length
        # in samples. Because channel DSP paramters are double we must convert
        # explicitly to integers. Minimum of 1 sample for filter risetimes and
        # CFD delay. Triangular fast filters (gap = 0 samples) are allowed.

        if fast_risetime < xdt:
            fast_risetime = int(1)
        else:
            fast_risetime = int(round(fast_risetime/xdt))
        fast_gap = int(round(fast_gap/xdt))
        if cfd_delay < xdt:
            cfd_delay = int(1)
        else:
            cfd_delay = int(round(cfd_delay/xdt))
        cfd_scale = int(cfd_scale) # [0, ..., 7] read as a double.
        if slow_risetime < xdt:
            slow_risetime = int(1)
        else:
            slow_risetime = int(round(slow_risetime/xdt))
        slow_gap = int(round(slow_gap/xdt))
        if tau < xdt:
            tau = int(1)
        else:
            tau = int(round(tau/xdt))

        ns = xdt*1000  # Convert from samples to time in ns.
        print(f"{self.__class__.__name__}.{inspect.currentframe().f_code.co_name}: Filter calculation requires parameters to\nbe an integer multiple of XDT. Parameters have not been changed for acquisition.\n\t XDT (ns): {ns:.0f}\n\t Trig. risetime (ns): {fast_risetime*ns:.0f}\n\t Trig. gap (ns): {fast_gap*ns:.0f}\n\t CFD scale: {cfd_scale:.0f}\n\t CFD delay (ns): {cfd_delay*ns:.0f}\n\t Ene. risetime (ns): {slow_risetime*ns:.0f}\n\t Ene. gap (ns): {slow_gap*ns:.0f}\n\t Tau (ns): {tau*ns:.0f}")
        
        return FilterParameters(
            xdt=xdt, fast_risetime=fast_risetime, fast_gap=fast_gap,
            cfd_delay=cfd_delay, cfd_scale=cfd_scale,
            slow_risetime=slow_risetime, slow_gap=slow_gap, tau=tau
        )
