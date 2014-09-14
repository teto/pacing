#pragma once

#include "ns3/sequence-number.h"

#include <set>

/**
Contrary to Tcp(Rx|Tx)Buffer, we only consider the sender point of view.

**/
class OwdSequencer
{
public:
  OwdSequencer();
  ~OwdSequencer();


//  uint32_t GetHighestSentSeqNb() const;

  uint32_t GetHighestRcvdInOrderSeqNb() const;
//  uint32_t GetExpectedSeqNb() const;

  /**
  Say that
  **/
  void AckSeqNb(uint32_t seq);


protected:
  std::set<uint32_t> m_outOfOrder; //!< sorted & unique values
//  SequenceNumber32
  uint32_t m_highestInOrderReceivedSeqNumber;

};
