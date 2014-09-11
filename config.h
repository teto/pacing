#pragma once


#define NUM_INTERFACES 2
#define CFG_PORT 1234


enum ProbingMode {
BOOTSTRAP,  //!< Just getting RTTs
ESTIMATING_OWD  //!< Different techniques might follow
};
