
#include "owd-client-basic.h"


OWdClientBasic::OWdClientBasic() :
    m_probesInARound(3)
{

}

OWdClientBasic::~OWdClientBasic()
{

}



void
OWdClientBasic::EstimateOWDStartNewRound(int& )
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT( m_currentMode == OWDEstimation);
  NS_ASSERT( m_inflight == 0);

//  Time ForwardDeltaOwd = 0;


  ///// First we need to decide which are forward slow & fast path
  //////////////////////////////////////////////////////
  if( GetRoundNo() == 0)
  {
    // If first round, we decide upon the RTTsampling phase
    NS_ASSERT( !m_rttRoundStats.empty() );

    m_forwardFastSubflow = m_rttRoundStats.back().ForwardFastSubflow;

    // Rtt slow -  abs(DeltaRTT)/2
//    m_estimatedForwardDeltaOwd = MicroSeconds(std::abs( (m_rttRoundStats.back().rtt[0]- m_rttRoundStats.back().rtt[1]).GetMicroSeconds() ) );
    m_estimatedForwardDeltaOwd = ns3::Abs( (m_rttRoundStats.back().rtt[0]- m_rttRoundStats.back().rtt[1]) );
    m_estimatedForwardDeltaOwd = MicroSeconds( m_estimatedForwardDeltaOwd.GetMicroSeconds()/2);
    NS_LOG_INFO("First OWD estimation round. Setting fast forward path to [" << m_forwardFastSubflow <<"]"
                << " and setting estimatedForward delay to " << m_estimatedForwardDeltaOwd
                );
  }
  else
  {
    NS_ASSERT( !m_owdRoundStats.empty() );
    //! TODO We should average over the past results
    m_forwardFastSubflow = m_owdRoundStats.back().ForwardFastSubflow;
    m_estimatedForwardDeltaOwd = m_owdRoundStats.back().EstimatedForwardDeltaOWD;
  }


  // First we should estimate the OWD from the past samples
  NS_ASSERT( ForwardSlowSubflowId() >= 0 && ForwardSlowSubflowId() < (int)m_sockets.size() );
  NS_ASSERT( ForwardFastSubflowId() >= 0 && ForwardFastSubflowId() < (int)m_sockets.size() );
  Ptr<Socket> slowSocket = m_sockets[ForwardSlowSubflowId()];
  Ptr<Socket> fastSocket = m_sockets[ForwardFastSubflowId()];

  //! Send seq nb 2 on slow socket
  Send(slowSocket,1);
//  Simulator::Now()

  //! Then we send the probes on the fast path
//  for(int i= -m_probesInARound/2; i < m_probesInARound/2+m_probesInARound%2; i++)
  for(uint8_t i= 0; i < m_probesInARound; i++)
  {
    //! sent on forward fast socket seq nb "1"
//    Time scheduledTime = m_estimatedForwardDeltaOwd + i*delayBetweenProbes;
    Time scheduledTime = GetProbeDelay(i);
    NS_LOG_INFO("Scheduling send on fast forward socket of seqnb 0 in "<< scheduledTime.GetMilliSeconds() << "ms");
    Simulator::Schedule ( scheduledTime , &OWDHost::Send, this, fastSocket, 0);
  }

  //! nb of packets sent = nb of probes + packet on slow path
  m_inflight = m_probesInARound + 1;

//  if (m_inflight < m_count)
//    {
//      m_sendEvent = Simulator::Schedule (m_interval, &OWDHost::Send, this);
//    }
}




RoundStats
OWdClientBasic::FinishRound()
{
    //! TODO compute estimations and return them

    /**  TODO need now to update OWD estimation
    */

    /* If all probes arrived afterreception of packet on slow path, then it means
    we waited too long before sending probes, ie we overestimated the ForwardDeltaOWD
    */
    if(m_arrivalPositionLastProbeBeforeSlowPath < 0)
    {
      /*
      Same as for the interval choice between probes, the reduction logic is critical to the algorithm
      performance

      As a rule of thumb (should improve it) we decrease by 10% the value of the estimated forward deltaOwd.

      As usua, Time values should not be too small to match real hardware clocks and network pacing values
      */
      NS_LOG_INFO("All probes arrived after packet on slow path");
      // TODO m_estimatedForwardDeltaOwd*0.1
      m_estimatedForwardDeltaOwd -= GetProbeDelay(0);

      // in this case we can't deduce the ReverseDeltaOWD so we copy the one from previous sampling ?
    }
    /* this is the opposite here, all probes arrived before packet on slow path, ie we underestimated the DeltaOWD
    That should be sthg like this
    std::max(10ms, 0.1* currentValue)
    */
    else if(m_arrivalPositionFirstProbeAfterSlowPath < 0)
    {
      NS_LOG_INFO("All probes arrived before packet on slow path");
//      m_estimatedForwardDeltaOwd += 0.1*m_estimatedForwardDeltaOwd;
      m_estimatedForwardDeltaOwd += GetProbeDelay(m_probesInARound-1);
    }
    else
    {
      /* Packet on slow path arrived between 2 probes so we can correctly update the DeltaOWD */
      //! Only the arrival between probes interest us so we have to decrease by one position
      //! if packet on slow path arrived before
      if(m_arrivalPositionSlowPacket <m_arrivalPositionLastProbeBeforeSlowPath ){
        --m_arrivalPositionLastProbeBeforeSlowPath;
        m_currentRoundStats.ReverseFastSubflow = ForwardSlowSubflowId();
      }

      NS_LOG_INFO("Probing succeeded in cornering OWD ");

      //!
      m_estimatedForwardDeltaOwd = GetProbeDelay(m_arrivalPositionLastProbeBeforeSlowPath);
    }

        //! We finished a round => reset
    NS_LOG_INFO("Owd round " << GetRoundNo() << " finished  (out of " << m_owdMaxRounds << "). "
      << "Subflow [" << sockId << "] forward delay looks shorter: "
      );

    //! TODO we need to update values

    m_currentRoundStats.EstimatedForwardDeltaOWD = m_estimatedForwardDeltaOwd;
    // TODO forward fastsubflow could be a float between 0 and 1, an average of past values.
    // If > 0.5 then => 1 else 0 for instance. Does not work with several
    // Or compare a smoothed score, each path would have a score.
    m_currentRoundStats.ForwardFastSubflow = m_estimatedForwardDeltaOwd;

    m_owdRoundStats.push_back(m_currentRoundStats);

    //! reset
    m_arrivalPositionLastProbeBeforeSlowPath = -1;
    m_arrivalPositionFirstProbeAfterSlowPath = -1;
    m_arrivalPositionSlowPacket = -1;
}

Time
OWdClientBasic::GetProbeDelay(uint8_t i) const
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

void
OWdClientBasic::NewPacketFromSlowPath(int position, Time arrivalTime,SeqTsHeader&)
{
  //! Only one packet was sent so update the RTT
}

void
OWdClientBasic::NewPacketFromFastPath(int position, Time arrivalTime,SeqTsHeader&)
{

}


bool
OWdClientBasic::ReachedConvergence() const
{
    //!
    return GetRoundNo() >= (int)m_owdMaxRounds;
}
