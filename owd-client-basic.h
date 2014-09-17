
#pragma once

#include "owd-client.h"

class OWdClientBasic
{
public:
  OWdClientBasic();
  ~OWdClientBasic();


  virtual void StartNewOwdEstimationRound();
  virtual void NewPacketFromSlowPath(int position, Time arrivalTime,SeqTsHeader&) = 0;
  virtual void NewPacketFromFastPath(int position, Time arrivalTime,SeqTsHeader&) = 0;

  Time
  GetProbeDelay(uint8_t i) const;

  //
  virtual bool ReachedConvergence() const = 0;

  virtual RoundStats FinishRound() = 0;

protected:
//  m_arrivalPositionLastProbeBeforeSlowPath = -1;
//  m_arrivalPositionFirstProbeAfterSlowPath = -1;

  RoundStats m_currentRoundStats; //!<

  /** arrival position at the server side, not on the client sie
  so it means order is deduced from seq nb
  */
  int m_arrivalPositionSlowPacket;  //!<
  int m_arrivalPositionLastProbeBeforeSlowPath;  //!<
  int m_arrivalPositionFirstProbeAfterSlowPath;  //!<

  Time m_estimatedForwardDeltaOwd;

  const uint32_t m_probesInARound;  //!< Number of probes in a round (default 3);
};
