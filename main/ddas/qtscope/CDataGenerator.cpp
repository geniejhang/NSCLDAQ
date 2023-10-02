/**
 * @file CDataGenerator.cpp
 * @brief Implementation of the offline data generation class.
 */

#include "CDataGenerator.h"

#include <iostream>
#include <cmath>

CDataGenerator::CDataGenerator() :
    m_engine((std::random_device())())
{}

CDataGenerator::~CDataGenerator() {}

/**
 * @details
 * Params are a pointer to the start of the data storage and a size, as is 
 * done in the XIA API for easier integration/consistency.
 */
int
CDataGenerator::GetTraceData(
    unsigned short* data, int dataSize, double binWidth
    )
{
    std::uniform_real_distribution<double> C(1000, 2000);
    std::uniform_real_distribution<double> A(100, 10000);
    std::uniform_real_distribution<double> t0(0.05*dataSize, 0.95*dataSize);
    std::normal_distribution<double> rise(0.5, 0.05);
    std::normal_distribution<double> decay(5, 0.05);

    double myC = C(m_engine);         // ADC units.
    double myA = A(m_engine);         // ADC units.
    double myT0 = t0(m_engine);       // Sample number.
    double myRise = rise(m_engine);   // Microseconds.
    double myDecay = decay(m_engine); // Microseconds.
  
    for (int i = 0; i < dataSize; i++) {
	data[i] = SinglePulse(myC, myA, myT0, myRise, myDecay, i, binWidth);
    }
  
    return 0;
}

/**
 * @details
 * Params are a pointer to the start of the data storage and a size, as 
 * is done in the XIA API for easier integration/consistency. Data is 
 * stored as a histogram, default binning 1 ADC unit per bin.
 */
int
CDataGenerator::GetHistogramData(unsigned int* data, int dataSize)
{
    std::normal_distribution<double> gaus(dataSize/4, 10); // Mean, stddev.    
    int ene = 0; // Event energy.
    for (int i = 0; i < 10000; i++) {
	ene = static_cast<unsigned int>(gaus(m_engine));
	data[ene]++;	
    }  

    return 0;  
}

/**
 * @details
 * Params are a pointer to the start of the data storage and a size, as is 
 * done in the XIA API for easier integration/consistency.
 */
int
CDataGenerator::GetBaselineData(double* data, int dataSize)
{
    std::uniform_real_distribution<double> dist(4500, 5500); // range
    for (int i = 0; i < dataSize; i++) {
	data[i] = dist(m_engine);
    }

    return 0;  
}

unsigned short
CDataGenerator::SinglePulse(
    double C, double A, double t0, double rise, double decay,
    int sample, double binWidth
    )
{
    // Convert position to dt in us using the binWidth determined by the XDT
    // channel parameter value:
  
    double dt = (sample - t0)*binWidth;
  
    std::normal_distribution<double> noise(0, 10); // Mean, stddev.
    unsigned short pval = 0;  
  
    if (sample < t0) {
	pval =  static_cast<unsigned short>(C + noise(m_engine));
    } else {
	pval =  static_cast<unsigned short>(
	    C + A*(1-std::exp(-dt/rise))*std::exp(-dt/decay) + noise(m_engine)
	    );
    }

    return pval;
}
