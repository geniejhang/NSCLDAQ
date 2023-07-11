/**
 * @addtogroup utilities libPixieUtilities.so
 * @brief Pixie-16 utilities for QtScope.
 *
 * This utility library is used by QtScope. It contains a number of classes
 * which call other parts of the DDAS code to boot and manage the modules. 
 * This library defines an API by which the pure-Python QtScope code can 
 * interact with the C/C++ FRIBDAQ and XIA API code needed to run a system of 
 * Pixie modules.
 * @{ 
 */

/**
 * @file  CPixieTraceUtilities.h
 * @brief Defines a class for trace management and a ctypes interface for 
 * the class.
 */

#ifndef CPIXIETRACEUTILITIES_H
#define CPIXIETRACEUTILITIES_H

#include <vector>

class CDataGenerator;

/**
 * @class CPixieTraceUtilities
 * @brief A class to read and fetch trace data from Pixie-16 modules.
 *
 * This class provides a ctypes-friendly interface to acquire "validated" 
 * (traces which are likely to contain a good signal pulse) and unvalidated 
 * traces. The class also provides methods to access the trace data.
 *
 * @todo Instead of validated traces can we process the trace using the fast 
 * filter parameters and wait for a real trigger?
 */

class CPixieTraceUtilities
{
public:
    CPixieTraceUtilities();
    ~CPixieTraceUtilities();

    int ReadTrace(int module, int channel);
    int ReadFastTrace(int module, int channel);

    /**
     * @brief Return the trace data.
     * @return unsigned short*  Pointer to the underlying trace storage.
     */
    unsigned short* GetTraceData() {return m_trace.data();}
    /**
     * @brief Set the flag for offline mode using the data generator.
     * @param mode  The generator flag is set to this input value.
     */
    void SetUseGenerator(bool mode) {m_useGenerator = mode;}
  
private:
    CDataGenerator* m_pGenerator; //!< The offline data generator.
    bool m_useGenerator; //!< True if using generated data, else online data.
    std::vector<unsigned short> m_trace; //!< Single channel trace data.

    void AcquireADCTrace(int module, int channel);
    template<typename T> double GetMedianValue(std::vector<T> v);
};
  
extern "C" {
    /** @brief Wrapper for the class constructor. */
    CPixieTraceUtilities* CPixieTraceUtilities_new()
    {
	return new CPixieTraceUtilities();
    }

    /** @brief Wrapper for reading a validated trace. */
    int CPixieTraceUtilities_ReadTrace(
	CPixieTraceUtilities* utils, int mod, int chan
	)
    {
	return utils->ReadTrace(mod, chan);
    }
    /** @brief Wrapper for reading an unvalidated trace. */
    int CPixieTraceUtilities_ReadFastTrace(
	CPixieTraceUtilities* utils, int mod, int chan
	)
    {
	return utils->ReadFastTrace(mod, chan);
    }
    /** @brief Wrapper to get trace data. */
    unsigned short* CPixieTraceUtilities_GetTraceData(
	CPixieTraceUtilities* utils
	)
    {
	return utils->GetTraceData();
    }
    /** @brief Wrapper to set generator use. */
    void CPixieTraceUtilities_SetUseGenerator(
	CPixieTraceUtilities* utils, bool mode
	)
    {
	return utils->SetUseGenerator(mode);
    }

    /** @brief Wrapper for the class destructor. */
    void CPixieTraceUtilities_delete(CPixieTraceUtilities* utils)
    {
	if(utils) {
	    delete utils;
	    utils = nullptr;
	}
    }
}

#endif

/** @} */
