
#pragma once

#include "owd-client.h"

class OWdClientBasic : public OWDHost
{
public:
  OWdClientBasic();
  ~OWdClientBasic();

  virtual int GetNumberOfProbesThisRound();
  virtual void StartNewOwdEstimationRound();
  virtual void NewPacketFromSlowPath(int position, Time arrivalTime,const SeqTsHeader&);
  virtual void NewPacketFromFastPath(int position, Time arrivalTime,const SeqTsHeader&);

  Time
  GetProbeDelay(uint8_t i) const;

  //
  virtual bool ReachedConvergence() const;

  virtual RoundStats FinishRound();

protected:
//  m_arrivalPositionLastProbeBeforeSlowPath = -1;
//  m_arrivalPositionFirstProbeAfterSlowPath = -1;

  //! already declared in parent's
//  RoundStats m_currentRoundStats; //!<

  /** arrival position at the server side, not on the client sie
  so it means order is deduced from seq nb
  */
//  int m_arrivalPositionSlowPacket;  //!<

  //! for each packet I could save real forward delay/real backward
  //! ArrivalTime/ Departue Time
  typedef std::map<Time,PacketStats> PacketList;
  PacketList m_probes;
  // Arrival/departure
  std::pair<Time,Time> m_slowPacket;


  Time m_arrivalTimeLastProbeBeforeSlowPath;  //!<
  Time m_arrivalTimeFirstProbeAfterSlowPath;  //!<
//  int m_probeNoTimeLastProbeBeforeSlowPath


  Time m_arrivalTimeOnSlowPath;

//  Time m_estimatedForwardDeltaOwd;  //!< besoin réellement ?

  const uint32_t m_probesInARound;  //!< Number of probes in a round (default 3);
  const uint32_t m_owdMaxRounds;  //!< How many times we sample the RTT before changing mode
};
