
#pragma once

#include "ns3/time.h"

/**
abstract class

Allow to compare different set of strategy
**/
class PacingStrategy
{
public:
  PacingStrategy() {}
  virtual ~PacingStrategy() {};

  virtual int ForwardFastSubflowId() = 0;
  virtual int ForwardSlowSubflowId() = 0;

  /**

  */
  virtual bool ReachedConvergence() = 0;
  /**
  **/
//  UpdateRound();


  /**
  Returns the delay to wait after a packet on slow path
  before sending the "i" th probe
  */
  virtual Time GetProbeDelay(uint8_t i) const = 0;

  /**
  Should
  **/
//  virtual void DumpCollection(std::ostream& os) = 0;
};
