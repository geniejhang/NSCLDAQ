#ifndef CMYSCALER_H
#define CMYSCALER_H

#include <vector>

#include <config.h>
#include <CScaler.h>

class CMyScaler : public CScaler
{
public:
    /** @brief Count raw and accepted triggers. */
    typedef struct _Counters {
	size_t s_nTriggers; //!< Raw triggers.
	size_t s_nAcceptedTriggers; //!< Accepted triggers (i.e. by the FPGA).
    } Counters;
    /** @brief Statistics are counters for cumulative and per-run triggers. */
    typedef struct _Statistics {
	Counters s_cumulative; //!< Cumulative. Not cleared on initialize.
	Counters s_perRun; //!< Per-run. Cleared on initialize.
    } Statistics;
    
private:
    unsigned short m_crate; //!< Crate ID value.
    unsigned short m_module; //!< Module number.
    double m_prevIC[16]; //!< Previous input counts (# raw fast triggers.)
    double m_prevOC[16]; //!< Previous output counts (# accepted triggers) 

    std::vector<uint32_t> m_scalers; //!< Vector of scaler data for the module.
    Statistics m_statistics; //!< Storage for calculated scaler data.
public:
    /**
     * @brief Constructor.
     * @param mod The module number.
     * @param crate The crate ID where the module resides.
     */
    CMyScaler(unsigned short mod, unsigned short crate);
    /** @brief Destructor. */
    ~CMyScaler();

    /** @brief Zero the per-run statistics and counters. */
    virtual void initialize();
    /**
     * @brief Read scalar data from a module.
     * @return Vector of scalar data for a single module.
     */
    virtual std::vector<uint32_t> read();
    /** @brief Cannot clear with Pixies. Does nothing. */
    virtual void clear() {};
    /** 
     * @brief Disable. Scalars do not need to be disabled at the end of a run.
     */
    virtual void disable() {};
    /** 
     * @breif Return the size of the scaler data.
     * @return Always 32 (only for 16-channel cards!)
     */
    virtual unsigned int size() { return 32; };
    /** 
     * @brief Get the run statistics.
     * @return Reference to the statistics storage object.
     */
    const Statistics& getStatistics() const { return m_statistics; }
private:
    /** 
     * @brief Clear the run counters.
     * @param[in,out] c Reference to the Counters struct to be cleared.
     */
    void clearCounters(Counters& c);
};

#endif
