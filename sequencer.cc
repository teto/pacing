#include "sequencer.h"
#include "ns3/log.h"
#include <set>
#include <algorithm>



NS_LOG_COMPONENT_DEFINE ("OwdSequencer");



OwdSequencer::OwdSequencer()
{
  NS_LOG_FUNCTION(this);

  m_highestInOrderReceivedSeqNumber = 0;

}


OwdSequencer::~OwdSequencer()
{
  NS_LOG_FUNCTION(this);


}


uint32_t
OwdSequencer::GetHighestRcvdInOrderSeqNb() const
{
  return m_highestInOrderReceivedSeqNumber;

}


void
OwdSequencer::AckSeqNb(uint32_t seq)
{
  NS_LOG_FUNCTION(this << "Received seq [" << seq << "]");

  // If already received this seq, do nothing
  if( GetHighestRcvdInOrderSeqNb() >= seq )
  {
    NS_LOG_INFO("Seq already received ");
    return;
  }

  // If seq is the expected in order seq
  if( GetHighestRcvdInOrderSeqNb() + 1 ==  seq )
  {
    NS_LOG_INFO("Seq received in order");
    ++m_highestInOrderReceivedSeqNumber;
    return;
  }

  // Means that it is out of order segement
  m_outOfOrder.insert(seq);

  // We check & clean the set in case it has become "in-order"

  for(std::set<uint32_t>::iterator it = m_outOfOrder.begin(); it != m_outOfOrder.end(); it++)
  {
    if( *it + 1 == GetHighestRcvdInOrderSeqNb() )
    {
      // returns void
      m_outOfOrder.erase(it);
      ++m_highestInOrderReceivedSeqNumber;
      NS_LOG_INFO("Out of order became in order");
      continue;
    }
    else
    {
      break;
    }


  }

//  int decal = 0;
//  for(std::set<uint32_t>::reverse_iterator it = m_outOfOrder.rbegin(); it != m_outOfOrder.rend(); it++)
//  {
//    //!
//    int dist = std::distance(m_outOfOrder.begin(), it);
//    if( *it == GetHighestRcvdInOrderSeqNb() + dist)
//    {
//      NS_LOG_INFO("Out of order became in order for [" << dist << "] bytes");
//      m_outOfOrder.erase( m_outOfOrder.begin(), it);
//
//      return;
//    }
//
//
//  }
//  if( GetHighestRcvdInOrderSeqNb() == )
//  if( GetHighestRcvdInOrderSeqNb() + 1 ==  seq )
//  {
//    NS_LOG_INFO("Seq received in order");
//    m_highestInOrderReceivedSeqNumber;
//  }

}
