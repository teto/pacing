#include "strategy-impl.h"


Time
PacingStrategyImpl::GetProbeDelay(uint8_t i) const
{
  NS_ASSERT_MSG( (i >= 0) && (i < m_probesInARound),"There is no probe with such an id");
  /**
  The choice of the delay between probe is really critical for the performance of the algorithm
  (speed of convergence, overhead etc...).
  Here we choose a constant delay for the sake of simplicity but we can imagine many schemes depending
  on the variance of the deltaOWD, the number of available probes (depend on the size of the global/local windows etc...)
  */

  Time delayBetweenProbes = MilliSeconds(5);


  //! need to go into signed mode
  int j = i;
  //! so as to shift j, since we want probes to be centered around DeltaOWD.
  j -= m_probesInARound/2;
  NS_LOG_INFO("j [" << j << "]");
  return (m_estimatedForwardDeltaOwd + MilliSeconds(delayBetweenProbes.GetMilliSeconds()*j) );
}
