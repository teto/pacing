
#pragma once

#include "strategy.h"

/**
abstract class

Allow to compare different set of strategy
**/
class PacingStrategyImpl : public Strategy
{
public:
  virtual int ForwardFastSubflowId();

  /**
  **/
  UpdateRound();

  virtual bool ReachedConvergence() ;

  /**
  Returns the delay to wait after a packet on slow path
  before sending the "i" th probe
  */
  virtual Time GetProbeDelay(uint8_t i) const = 0;

  /**
  Should be of format os << "#roundId rtt0 rtt1 realForwardOWD0 realReverseOWD0 realForwardOWD1 realReverseOWD1 estimatedForwardOWD0 estimatedReverseOwd1 estimatedForwardOWD1 estimatedReverseOwd0 halfRTT0 halfRTT1 id(Fastest)" << std::endl;
  **/
  virtual void DumpCollection(std::ostream& os) = 0;

protected:
  const uint32_t m_probesInARound;  //!< Number of probes in a round (default 3);

};
