#pragma once


#define NUM_INTERFACES 2
#define CFG_PORT 1234


enum Mode {
RTTSampling,  //!< Just getting RTTs
OWDEstimation  //!< Different techniques might follow
};

