
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "owd-client-basic.h"




NS_LOG_COMPONENT_DEFINE ("OwdClientBasic");

OWdClientBasic::OWdClientBasic() :
    m_probesInARound(3)
      ,m_owdMaxRounds(20)
{
//  m_estimatedForwardDeltaOwd = 0;
//  m_arrivalPositionSlowPacket = -1;
}

OWdClientBasic::~OWdClientBasic()
{

}



void
OWdClientBasic::StartNewOwdEstimationRound()
{
  NS_LOG_UNCOND("\n\n");
  NS_LOG_FUNCTION ( this << "New round " << GetRoundNo() << "");
  NS_ASSERT( m_currentMode == OWDEstimation);
  NS_ASSERT( m_inflight == 0);

//  Time ForwardDeltaOwd = 0;

  //// Reset a few values
  ////////////////////////////
  m_arrivalTimeLastProbeBeforeSlowPath = MilliSeconds(0);
  m_arrivalTimeFirstProbeAfterSlowPath      = MilliSeconds(0);
  m_arrivalTimeOnSlowPath               = MilliSeconds(0);
//  m_estimatedForwardDeltaOwd           = MilliSeconds(0);
  m_timeOfFirstSentPacket               = MilliSeconds(0);
//  m_probeNo = 0;
  m_currentRoundStats = RoundStats();
  m_probes.clear();
  Time t;
  m_slowPacket = std::make_pair(t,t);



  if( GetRoundNo() == 0)
  {
    // If first round, we decide upon the RTTsampling phase
    NS_ASSERT( !m_rttRoundStats.empty() );
//    if(m_rttRoundStats.back().rtt[0] > m_rttRoundStats.back().rtt[1])
//    {
//      m_forwardFastSubflow =
//    }
    m_forwardFastSubflow = m_rttRoundStats.back().ForwardFastSubflow;
  }
  else {
    m_forwardFastSubflow = m_owdRoundStats.back().ForwardFastSubflow;
  }

  NS_LOG_INFO("starting with fast froward subflow " << ForwardFastSubflowId() );
  // First we should estimate the OWD from the past samples
//  NS_ASSERT( ForwardSlowSubflowId() >= 0 && ForwardSlowSubflowId() < (int)m_sockets.size() );
  NS_ASSERT( ForwardFastSubflowId() >= 0 && ForwardFastSubflowId() < (int)m_sockets.size() );

  Ptr<Socket> slowSocket = m_sockets[ForwardSlowSubflowId()];
  Ptr<Socket> fastSocket = m_sockets[ForwardFastSubflowId()];

  //! Send seq nb 2 on slow socket
  m_timeOfFirstSentPacket = Simulator::Now();
  Send(slowSocket, m_lowestRoundSeq + 1, m_timeOfFirstSentPacket.GetTimeStep() );
//  Simulator::Now()

  //! Then we send the probes on the fast path
//  for(int i= -m_probesInARound/2; i < m_probesInARound/2+m_probesInARound%2; i++)
  for(uint8_t i= 0; i < m_probesInARound; i++)
  {
    //! sent on forward fast socket seq nb "1"
//    Time scheduledTime = m_estimatedForwardDeltaOwd + i*delayBetweenProbes;
    // this must be always positive, I can't wait a negative time
    Time probeDelay = GetProbeDelay(i) ;
    NS_ASSERT(probeDelay >= 0);
    NS_LOG_INFO("Scheduling send on fast forward socket of seqnb [ " <<  m_lowestRoundSeq << "] in "<< probeDelay );
//    m_probes[scheduledTime] = i;

//    PacketStats placeholder;
//    m_probes[scheduledTime] = placeholder;

    Simulator::Schedule ( probeDelay , &OWdClientBasic::Send, this, fastSocket, m_lowestRoundSeq, (m_timeOfFirstSentPacket + probeDelay).GetTimeStep());
  }

  //! nb of packets sent = nb of probes + packet on slow path
  m_inflight = m_probesInARound + 1;
//  m_inflight = m_probes.size() ;

//  if (m_inflight < m_count)
//    {
//      m_sendEvent = Simulator::Schedule (m_interval, &OWDHost::Send, this);
//    }
}


int
OWdClientBasic::GetNumberOfProbesThisRound()
{
  return m_probesInARound;
}


Time
OWdClientBasic::GetProbeDelay(uint8_t i) const
{
  NS_ASSERT_MSG( (i >= 0) && (i < m_probesInARound),"There is no probe with such an id");
  NS_ASSERT( m_currentMode == OWDEstimation);

  /**
  The choice of the delay between probe is really critical for the performance of the algorithm
  (speed of convergence, overhead etc...).
  Here we choose a constant delay for the sake of simplicity but we can imagine many schemes depending
  on the variance of the deltaOWD, the number of available probes (depend on the size of the global/local windows etc...)
  */

  Time delayBetweenProbes = MilliSeconds(2);
  Time estimatedForwardDeltaOwd;
  Time probeDelay;

  if( GetRoundNo() == 0)
  {
    // If first round, we decide upon the RTTsampling phase
    NS_ASSERT( !m_rttRoundStats.empty() );

//    m_forwardFastSubflow = m_rttRoundStats.back().ForwardFastSubflow;

    // Rtt slow -  abs(DeltaRTT)/2
//    m_estimatedForwardDeltaOwd = MicroSeconds(std::abs( (m_rttRoundStats.back().rtt[0]- m_rttRoundStats.back().rtt[1]).GetMicroSeconds() ) );
//    estimatedForwardDeltaOwd = ns3::Abs( (m_rttRoundStats.back().rtt[0]- m_rttRoundStats.back().rtt[1]) );
//    estimatedForwardDeltaOwd = MicroSeconds( m_estimatedForwardDeltaOwd.GetMicroSeconds()/2);

//    estimatedForwardDeltaOwd = Time();

//    NS_LOG_INFO("First OWD estimation round. Setting fast forward path to [" << m_forwardFastSubflow <<"]"
//                << " and setting estimatedForward delay to " << m_estimatedForwardDeltaOwd
//                );
  }
  else
  {
    NS_ASSERT( !m_owdRoundStats.empty() );
    //! TODO We should average over the past results
//    m_forwardFastSubflow = m_owdRoundStats.back().ForwardFastSubflow;
    estimatedForwardDeltaOwd = m_owdRoundStats.back().EstimatedForwardDeltaOWD;
    NS_LOG_INFO("Setting estimatedForward delay to " << estimatedForwardDeltaOwd);
  }

  NS_ASSERT(estimatedForwardDeltaOwd.IsPositive() );
  //! need to go into signed mode
//  int j = i;
  //! so as to shift j, since we want probes to be centered around DeltaOWD.
  int startIdx = 0;
  startIdx =  -((int)m_probesInARound/2);
//  NS_LOG_INFO("startIdx [" << startIdx << "]");

  probeDelay = estimatedForwardDeltaOwd;
  probeDelay +=   MilliSeconds(delayBetweenProbes.GetMilliSeconds()*startIdx);


  while( probeDelay.IsStrictlyNegative() ){
    startIdx++;
    probeDelay += delayBetweenProbes;
  }


//  NS_LOG_INFO("startIdx [" << startIdx << "]");
  return (estimatedForwardDeltaOwd +  MilliSeconds(delayBetweenProbes.GetMilliSeconds()*(startIdx + (int)i) ));
}



void
OWdClientBasic::NewPacketFromSlowPath(int position, Time arrivalTime, const SeqTsHeader& seqTs)
{
  //! Only one packet was sent so update the RTT
  //! les positions on s'en fiche en fait
  NS_LOG_INFO("Packet on slow path arrived !");
  m_arrivalTimeOnSlowPath = arrivalTime;

  // can update RTT & stuff
  int sockId = ForwardSlowSubflowId();

  m_slowPacket = std::make_pair(Simulator::Now(),seqTs.GetReceiverTs());
  m_currentRoundStats.rtt[sockId]             = Simulator::Now() - seqTs.GetReceiverTs();

  m_currentRoundStats.RealForwardDelay[sockId] = seqTs.GetSenderTs() - seqTs.GetReceiverTs();
  m_currentRoundStats.RealReverseDelay[sockId] = Simulator::Now() - seqTs.GetSenderTs();


  /* Imply that all probes will arrive after reception of packet on slow path, then it means
  we waited too long before sending probes, ie we overestimated the ForwardDeltaOWD
  */
//  if( ts.GetSeq() == m_lowestRoundSeq )
//  {
//    //
//
//  }


}


void
OWdClientBasic::NewPacketFromFastPath(int position, Time arrivalTime,const SeqTsHeader& seqTs)
{
//  m_probeNo++;

  PacketStats stats;
  stats.RealForwardDelay = seqTs.GetSenderTs() - seqTs.GetReceiverTs();
  stats.RealReverseDelay = Simulator::Now() - seqTs.GetSenderTs();
  stats.DepartureTime = seqTs.GetReceiverTs();

  m_probes[ arrivalTime ] = stats;

  // if packet on slow path has not arrived
//  if( m_arrivalTimeOnSlowPath.IsZero() )




  // Means the packet arrived at the remote host before packet on slow path
  if( (seqTs.GetSeq() == m_lowestRoundSeq)  )
  {

    // Update time . May be done several times
    NS_LOG_INFO(" >> Probe arrived before packet on slow path");
    m_arrivalTimeLastProbeBeforeSlowPath = arrivalTime;
    return;
  }
  // if probe arrived after packet on slow path at the remote host
  // if this is the **first** probe to arrive after at the remote host
  else if(m_arrivalTimeFirstProbeAfterSlowPath.IsZero() )
  {
//    NS_ASSERT(seqTs.GetSeq()== m_lowestRoundSeq  +1);

      NS_LOG_INFO(" >> Probe arrived JUST after packet on slow path");
      m_arrivalTimeFirstProbeAfterSlowPath = arrivalTime;



    return;
  }
  else {

    NS_LOG_INFO("Probe arrived after packet on slow path");
  }




//  NS_FATAL_ERROR("How can we arrive here ?");
}


//! TODO compute estimations and return them

/**
Need to determine the FastReversePath
**/
RoundStats
OWdClientBasic::FinishRound()
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(m_inflight ==0, "There are still packets inflight");

  //! Previous round only used 2 seq nb
  m_lowestRoundSeq += 2;

  //! TODO average OWD using m_probes packet stats
  int id = ForwardFastSubflowId();
  NS_LOG_INFO(this << "DEBUG: forward fast sublfow id" << ForwardFastSubflowId() );
  for(PacketList::iterator i = m_probes.begin(); i != m_probes.end(); ++i)
  {
    //!
//     m_currentRoundStats.RealForwardDelay[id] += i->second.RealForwardDelay;
//     m_currentRoundStats.RealReverseDelay[id] += i->second.RealReverseDelay;

    m_currentRoundStats.RealForwardDelay[id] = i->second.RealForwardDelay;
     m_currentRoundStats.RealReverseDelay[id] = i->second.RealReverseDelay;
  }
//  m_currentRoundStats.RealForwardDelay[id] = TimeStep(m_currentRoundStats.RealForwardDelay[id].GetTimeStep() / m_probes.size());
//  m_currentRoundStats.RealReverseDelay[id] = TimeStep(m_currentRoundStats.RealForwardDelay[id].GetTimeStep() / m_probes.size());
//  m_currentRoundStats.[id] /= m_probes.size();

  m_currentRoundStats.rtt[id] = m_currentRoundStats.RealForwardDelay[id] + m_currentRoundStats.RealReverseDelay[id];

  // computed by default (computation improved in a subsequent part of the algorithm)
//  m_currentRoundStats.EstimatedForwardDelay[ForwardSlowSubflowId()] = (m_currentRoundStats.rtt[ForwardSlowSubflowId()] )/(uint8_t)2;
//  m_currentRoundStats.EstimatedReverseDelay[ForwardSlowSubflowId()] = m_currentRoundStats.EstimatedForwardDelay[ForwardSlowSubflowId()];
//
//  // forward delays - GetProbeDelay(0)
//  m_currentRoundStats.EstimatedForwardDelay[ForwardFastSubflowId()] = (m_currentRoundStats.rtt[ForwardFastSubflowId()]  )/(uint8_t)2;
//  m_currentRoundStats.EstimatedReverseDelay[ForwardFastSubflowId()] = m_currentRoundStats.EstimatedForwardDelay[ForwardFastSubflowId()];


  /* If all probes arrived afterreception of packet on slow path, then it means
  we waited too long before sending probes, ie we underestimated the ForwardDeltaOWD
  */
  if( m_arrivalTimeFirstProbeAfterSlowPath.IsZero())
  {
    NS_LOG_INFO("All probes arrived before packet on slow path");

    m_currentRoundStats.EstimatedForwardDeltaOWD = GetProbeDelay(m_probesInARound-1);
    /*
    Same as for the interval choice between probes, the reduction logic is critical to the algorithm
    performance

    As a rule of thumb (should improve it) we decrease by 10% the value of the estimated forward deltaOwd.

    As usual, Time values should not be too small to match real hardware clocks and network pacing values
    */


  }
  /* this is the opposite here, all probes arrived before packet on slow path,
  ie we overestimated the DeltaOWD
  That should be sthg like this
  std::max(10ms, 0.1* currentValue)
  */
  else if(m_arrivalTimeLastProbeBeforeSlowPath.IsZero() )
  {

      /* Packet on slow path arrived between 2 probes so we can correctly update the DeltaOWD */
      //! Only the arrival between probes interest us so we have to decrease by one position
      //! if packet on slow path arrived before
  // We have to decide what are the fast paths (only possible if packet on slow path comprised)
  // between probes else
    NS_LOG_INFO("All probes arrived after packet on slow path");
    m_currentRoundStats.EstimatedForwardDeltaOWD = GetProbeDelay(0);
  }
  // Probes were successful in framing slow packet
  else
  {
    Time departureDelayBetweenFramingProbes = m_probes[m_arrivalTimeFirstProbeAfterSlowPath].DepartureTime - m_probes[m_arrivalTimeLastProbeBeforeSlowPath].DepartureTime;
    Time arrivalDelayBetweenFramingProbes = m_arrivalTimeFirstProbeAfterSlowPath -m_arrivalTimeLastProbeBeforeSlowPath;

    Time reverseDeltaOWD = m_slowPacket.first - m_arrivalTimeFirstProbeAfterSlowPath;

    if(reverseDeltaOWD.IsNegative() )
    {
      m_currentRoundStats.ReverseFastSubflow = ForwardSlowSubflowId();
    }
    else
    {
      m_currentRoundStats.ReverseFastSubflow = ForwardFastSubflowId();
    }

    // TODO faire plusieurs cas selon si le FFP == BFP
    if(m_currentRoundStats.ReverseFastSubflow == m_currentRoundStats.ForwardFastSubflow)
    {
      NS_LOG_INFO("FFP == BFP");

    }
    else
    {
      NS_LOG_INFO("FFP != BFP");
    }

    NS_LOG_INFO("Probing successfully cornered OWD values. Can now compute fast backward delay");

      //! Update estimate delta OWD
      // We take Departure time of last probe bfore slow packet +  1/3 * delay between this probe and the next one
      m_currentRoundStats.EstimatedForwardDeltaOWD =
        (m_probes[m_arrivalTimeLastProbeBeforeSlowPath].DepartureTime-
         //m_timeOfFirstSentPacket
         m_slowPacket.second
         ) +
          MilliSeconds(
                      0.3* (
//                            m_probes[m_arrivalTimeFirstProbeAfterSlowPath].DepartureTime - m_probes[m_arrivalTimeLastProbeBeforeSlowPath].DepartureTime
                          departureDelayBetweenFramingProbes
                          ).GetMilliSeconds() );

      NS_LOG_INFO("Estimating ForwardOwdDelta to " << m_currentRoundStats.EstimatedForwardDeltaOWD);
      //! Update estimated backward delay
//      m_currentRoundStats.EstimatedForwardDeltaOWD
//      if(m_timeOfFirstSentPacket




      NS_LOG_INFO("Estimating Reverse Fast Subflow to [" << m_currentRoundStats.ReverseFastSubflow << "]");
      m_currentRoundStats.EstimatedReverseDeltaOWD = Abs(reverseDeltaOWD) ;// Abs
      NS_LOG_INFO("Estimating ReverseOwdDelta to " << m_currentRoundStats.EstimatedReverseDeltaOWD);




      // TODO now need to update the OWD estimations
      // TO do that we estimate the arrival time at the remote host
      // to do that we find the fastest round trip time without considering
      // RTT/2

      // take the time arrival of the first packet that came back after slow packet arrived at remote host
      Time halfSmallestRtt = Min( m_slowPacket.first, m_arrivalTimeFirstProbeAfterSlowPath+ arrivalDelayBetweenFramingProbes/(uint8_t)2 )
        // minus the time of the first packet sent on forward fast path that arrived
        // after packet on slow path
          -(m_probes[m_arrivalTimeFirstProbeAfterSlowPath].DepartureTime + departureDelayBetweenFramingProbes/(uint8_t)2);

      // Finally we do RTT/2
      //.GetMilliSeconds()
      halfSmallestRtt =  halfSmallestRtt / (uint8_t)2 ;

      m_currentRoundStats.EstimatedForwardDelay[ForwardFastSubflowId()] = halfSmallestRtt ;
      m_currentRoundStats.EstimatedReverseDelay[ForwardFastSubflowId()] = m_currentRoundStats.rtt[ForwardFastSubflowId()] - m_currentRoundStats.EstimatedForwardDelay[ForwardFastSubflowId()];

//      NS_LOG_INFO("Estimating Forward delay to " << m_currentRoundStats.EstimatedForwardDeltaOWD);

      //Prendre la moitie du temps entre 2 probes
      // Bug c pas forcément la probe 0
      //
      m_currentRoundStats.EstimatedForwardDelay[ForwardSlowSubflowId()] = m_currentRoundStats.EstimatedForwardDeltaOWD + m_currentRoundStats.EstimatedForwardDelay[ForwardFastSubflowId()];

      #if 0
      m_currentRoundStats.EstimatedForwardDelay[ForwardSlowSubflowId()] =
        // Delay  probe before slow packet
        (m_probes[m_arrivalTimeLastProbeBeforeSlowPath].DepartureTime - m_slowPacket.second )
        +  (m_probes[m_arrivalTimeFirstProbeAfterSlowPath].DepartureTime - m_probes[m_arrivalTimeLastProbeBeforeSlowPath].DepartureTime ) / (uint8_t)2   // choose average between corenering packets
        + m_currentRoundStats.EstimatedForwardDelay[ForwardFastSubflowId()];  // + estiamted forward time for fast path
//      GetProbeDelay(0)
//      m_currentRoundStats.EstimatedForwardDelay[ForwardSlowSubflowId()] = m_probes[m_arrivalTimeLastProbeBeforeSlowPath].DepartureTime  + m_currentRoundStats.EstimatedForwardDelay[ForwardFastSubflowId()];
      #endif
      m_currentRoundStats.EstimatedReverseDelay[ForwardSlowSubflowId()] = m_currentRoundStats.rtt[ForwardSlowSubflowId()] -m_currentRoundStats.EstimatedForwardDelay[ForwardSlowSubflowId()];
//      return m_currentRoundStats;
  }

  // for now, we never change the forwardfast subflow
  m_currentRoundStats.ForwardFastSubflow = ForwardFastSubflowId();

  NS_LOG_INFO(this << "DEBUG2: forward fast sublfow id" << ForwardFastSubflowId() );

  return m_currentRoundStats;
}





bool
OWdClientBasic::ReachedConvergence() const
{
    //!
    return GetRoundNo() >= (int)m_owdMaxRounds;
}
