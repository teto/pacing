
#pragma once

#include "ns3/time.h"


struct
{
  sockId, SeqNb,
  Time
};

/**
abstract class

Allow to compare different set of strategy
**/
class PacingStrategy
{
public:
  PacingStrategy() {}
  virtual ~PacingStrategy() {};

//  virtual int ForwardFastSubflowId() = 0;
//  virtual int ForwardSlowSubflowId() = 0;

  virtual void GenerateScheduling( std::vector<sockId, SeqNb, Time>&) = 0;
  /***
dd
  **/
//  virtual RoundStats FinishRound() = 0;

//  virtual int GetNbOfProbes() = 0;

  ReceivedPacket(sockId)
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
//  virtual Time GetProbeDelay(uint8_t i) const = 0;

  /**
  Should
  **/
//  virtual void DumpCollection(std::ostream& os) = 0;
};
