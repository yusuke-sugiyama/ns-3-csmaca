/* -*- Mode:C++; -*- */
/*
 * Copyright (c) 2014 Yusuke Sugiyama
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., Saruwatari Lab, Shizuoka University, Japan
 *
 * Author: Yusuke Sugiyama <sugiyama@aurum.cs.inf.shizuoka.ac.jp>
 */

#include "ns3/core-module.h"
#include "ns3/llc-snap-header.h"
#include "ns3/object.h"
#include "ns3/log.h"
#include "csmaca-mac-header.h"
#include "csmaca-mac-trailer.h"
#include "csmaca-mac.h"

NS_LOG_COMPONENT_DEFINE ("CsmacaMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmacaMac);

class PhyCsmacaMacListener : public ns3::CsmacaPhyListener
{
public:
  /**
   * Create a PhyMacLowListener for the given CsmacaMac.
   *
   * \param csmacaMac
   */
  PhyCsmacaMacListener (ns3::CsmacaMac *csmacaMac)
    : m_csmacaMac (csmacaMac)
  {
  }
  virtual ~PhyCsmacaMacListener ()
  {
  }
  virtual void NotifyMaybeCcaBusyStart (Time duration)
  {
    m_csmacaMac->NotifyMaybeCcaBusyStartNow (duration);
  }
  virtual void NotifyTxStart (Time duration)
  {
    m_csmacaMac->NotifyTxStartNow (duration);
  }
  virtual void NotifyRxStart (Time duration)
  {
    m_csmacaMac->NotifyRxStartNow (duration);
  }
  virtual void NotifyRxEndOk (Ptr<Packet> packet)
  {
    m_csmacaMac->ReceiveOk (packet);
  }
  virtual void NotifyRxEndError (Ptr<Packet> packet)
  {
    m_csmacaMac->ReceiveError (packet);
  }
private:
  ns3::CsmacaMac *m_csmacaMac;
};

CsmacaMac::CsmacaMac ()
  : m_phyCsmacaMacListener (0),
    m_rtsSendThreshold (1000),
    m_resendRtsNum (0),
    m_resendRtsMax (7),
    m_resendDataNum (0),
    m_resendDataMax (7),
    m_cwMin (15),
    m_cwMax (1023),
    m_cw (m_cwMin),
    m_backoffSlots (0),
    m_sifs (MicroSeconds (16)),
    m_difs (MicroSeconds (34)),
    m_slotTime (MicroSeconds (9)),
    m_backoffStart (0),
    m_lastRxStart (0),
    m_lastRxDuration (0),
    m_lastBusyStart (0),
    m_lastBusyDuration (0),
    m_lastTxStart (0),
    m_lastTxDuration (0),
    m_lastNavStart (0),
    m_lastNavDuration (0),
    m_lastAckTimeoutEnd (0),
    m_lastCtsTimeoutEnd (0),
    m_rxing (false),
    m_sendCtsAfterRtsEvent (),
    m_sendDataAfterCtsEvent (),
    m_sendAckAfterDataEvent (),
    m_ackTimeoutEvent (),
    m_ctsTimeoutEvent (),
    m_backoffTimeoutEvent (),
    m_backoffGrantStartEvent()
{
  NS_LOG_FUNCTION (this);
  CsmacaPreamble preamble;
  CsmacaMacHeader rts;
  rts.SetType (CSMACA_MAC_RTS);
  CsmacaMacHeader cts;
  cts.SetType (CSMACA_MAC_CTS);
  CsmacaMacHeader ack;
  ack.SetType (CSMACA_MAC_ACK);
  CsmacaMacTrailer fcs;

  Time rtsDuration = Seconds (double(rts.GetSize () + fcs.GetSize ()) / preamble.GetRate ()) + preamble.GetDuration ();
  Time ctsDuration = Seconds (double(cts.GetSize () + fcs.GetSize ()) / preamble.GetRate ()) + preamble.GetDuration ();
  Time ackDuration = Seconds (double(ack.GetSize () + fcs.GetSize ()) / preamble.GetRate ()) + preamble.GetDuration ();

  m_maxPropagationDelay = Seconds (1000.0 / 300000000.0);
  m_rtsSendAndSifsTime = rtsDuration + m_maxPropagationDelay + m_sifs;
  m_ctsSendAndSifsTime = ctsDuration + m_maxPropagationDelay + m_sifs;
  m_ackSendAndSifsTime = ackDuration + m_maxPropagationDelay + m_sifs;

  SetPhy (CreateObject<CsmacaPhy> ());
  SetQueue (CreateObject<CsmacaMacQueue> ());
  m_rng = new CsmacaRealRandomStream ();
  SetupPhyCsmacaMacListener (GetPhy ()->GetPhyStateHelper ());
}

TypeId
CsmacaMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmacaMac")
    .SetParent<Object> ()
    .AddAttribute ("Rate", "Rate for send.",
                   UintegerValue (6000000 / 8),
                   MakeUintegerAccessor (&CsmacaMac::m_rate),
                   MakeUintegerChecker<uint32_t>(0))
  ;

  return tid;
}

int64_t
CsmacaMac::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_rng->AssignStreams (stream);
  return 1;
}

CsmacaMac::~CsmacaMac ()
{
  NS_LOG_FUNCTION (this);
}

void
CsmacaMac::SetPhy (Ptr<CsmacaPhy> phy){
  m_phy = phy;
}

void
CsmacaMac::SetNetDevice (Ptr<CsmacaNetDevice> device){
  m_device = device;
}

void
CsmacaMac::SetQueue (Ptr<CsmacaMacQueue> queue){
  m_queue = queue;
}
void
CsmacaMac::SetAddress (Mac48Address address){
  m_address = address;
}
Ptr<CsmacaPhy>
CsmacaMac::GetPhy (){
  return m_phy;
}

Ptr<CsmacaMacQueue>
CsmacaMac::GetQueue (){
  return m_queue;
}

Mac48Address
CsmacaMac::GetAddress (){
  return m_address;
}

void
CsmacaMac::SetupPhyCsmacaMacListener (Ptr<CsmacaPhyStateHelper> state)
{
  m_phyCsmacaMacListener = new PhyCsmacaMacListener (this);
  state->RegisterListener (m_phyCsmacaMacListener);
}

void
CsmacaMac::NotifyMaybeCcaBusyStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  m_lastBusyStart = Simulator::Now ();
  m_lastBusyDuration = duration;
}

void
CsmacaMac::NotifyTxStartNow (Time duration)
{
  NS_LOG_FUNCTION (this);
  if (m_rxing)
    {
      NS_ASSERT (Simulator::Now () - m_lastRxStart <= m_sifs);
      m_lastRxDuration = Simulator::Now () - m_lastRxStart;
      m_rxing = false;
    }
  m_lastTxStart = Simulator::Now ();
  m_lastTxDuration = duration;
}

void
CsmacaMac::NotifyRxStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  m_lastRxStart = Simulator::Now ();
  m_lastRxDuration = duration;
  m_rxing = true;
}

Time
CsmacaMac::GetBackoffGrantStart (void) const
{
  NS_LOG_FUNCTION (this);

  Time rxBackoffStart = m_lastRxStart + m_lastRxDuration + m_difs;
  Time txBackoffStart = m_lastTxStart + m_lastTxDuration + m_difs;
  Time busyBackoffStart = m_lastBusyStart + m_lastBusyDuration + m_difs;
  Time navBackoffStart = m_lastNavStart + m_lastNavDuration + m_difs;
  Time ackTimeoutBackoffStart = m_lastAckTimeoutEnd + m_difs;
  Time ctsTimeoutBackoffStart = m_lastCtsTimeoutEnd + m_difs;
  Time backoffGrantedStart = Max (rxBackoffStart, txBackoffStart);
  backoffGrantedStart = Max (backoffGrantedStart, busyBackoffStart);
  backoffGrantedStart = Max (backoffGrantedStart, navBackoffStart);
  backoffGrantedStart = Max (backoffGrantedStart, ackTimeoutBackoffStart);
  backoffGrantedStart = Max (backoffGrantedStart, ctsTimeoutBackoffStart);
                                        
  NS_LOG_INFO ("access grant start="  << backoffGrantedStart <<
               ", rx start="   << rxBackoffStart   <<
               ", tx start="   << txBackoffStart    <<
               ", busy start=" << busyBackoffStart <<
               ", nav start="  << navBackoffStart  <<
               ", ack timeout start="  << ackTimeoutBackoffStart << 
               ", cts timeout start="  << ackTimeoutBackoffStart
	       );
  return backoffGrantedStart;
}

Time
CsmacaMac::GetSendGrantStart (void) const
{
  NS_LOG_FUNCTION (this);

  Time rxSendStart = m_lastRxStart + m_lastRxDuration;
  Time txSendStart = m_lastTxStart + m_lastTxDuration;
  Time busySendStart = m_lastBusyStart + m_lastBusyDuration;
  Time navSendStart = m_lastNavStart + m_lastNavDuration;
  Time ackTimeoutSendStart = m_lastAckTimeoutEnd;
  Time ctsTimeoutSendStart = m_lastCtsTimeoutEnd;
  Time sendGrantedStart = Max (rxSendStart, txSendStart);
  sendGrantedStart = Max (sendGrantedStart, busySendStart);
  sendGrantedStart = Max (sendGrantedStart, navSendStart);
  sendGrantedStart = Max (sendGrantedStart, ackTimeoutSendStart);
  sendGrantedStart = Max (sendGrantedStart, ctsTimeoutSendStart);
                                        
  NS_LOG_INFO ("access grant start="  << sendGrantedStart <<
               ", rx start="   << rxSendStart   <<
               ", tx start="   << txSendStart    <<
               ", busy start=" << busySendStart <<
               ", nav start="  << navSendStart  <<
               ", ack timeout start="  << ackTimeoutSendStart <<
               ", cts timeout start="  << ctsTimeoutSendStart
	       );
  return sendGrantedStart;
}

void
CsmacaMac::UpdateCw ()
{
  m_cw = std::min ( 2 * (m_cw + 1) - 1, m_cwMax);
  m_backoffSlots = m_rng->GetNext (0, m_cw);
  m_backoffStart = Simulator::Now ();
}

void
CsmacaMac::InitSend ()
{
  NS_LOG_FUNCTION (this);
  m_resendRtsNum = 0;
  m_resendDataNum = 0;
  m_currentPacket = 0;
  m_cw = m_cwMin;
}

Time
CsmacaMac::CalculateDataSendTime (CsmacaPreamble preamble)
{
  NS_LOG_FUNCTION (this);
  CsmacaMacTrailer fcs;
  uint32_t dataSize =
    m_currentPacket->GetSize () +
    m_currentHdr.GetSize () +
    fcs.GetSize ();
  Time txDuration =
    Seconds((double)dataSize / preamble.GetRate ()) +
    preamble.GetDuration () +
    m_maxPropagationDelay;
  return txDuration;
}

void
CsmacaMac::ReceiveOk (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  m_rxing = false;

  CsmacaMacHeader hdr;
  packet->RemoveHeader (hdr);
  NS_LOG_DEBUG (hdr);

  // Set Nav
  if (hdr.GetAddr1 () != GetAddress ())
    {
      SetNav (hdr.GetDuration ());
    }


  switch (hdr.GetType ())
    {
    case CSMACA_MAC_DATA:
      if (hdr.GetAddr1 () == GetAddress () && !hdr.GetAddr1 ().IsGroup ())
	{
	  m_sendAckAfterDataEvent = Simulator::Schedule (m_sifs,
							 &CsmacaMac::SendAckAfterData,
							 this,
							 hdr.GetAddr2 ());
	}
      m_device->Receive (packet, hdr.GetAddr1 (), hdr.GetAddr2 ());
      break;
      
    case CSMACA_MAC_ACK:
      if (hdr.GetAddr1 () == GetAddress ())
	{
	  m_ackTimeoutEvent.Cancel ();
	  m_lastAckTimeoutEnd = Simulator::Now ();
	  InitSend ();
	  StartBackoffIfNeeded ();
	}
      break;
    
    case CSMACA_MAC_RTS:
      if (hdr.GetAddr1 () == GetAddress ())
	{
	  m_sendCtsAfterRtsEvent = Simulator::Schedule (m_sifs,
							&CsmacaMac::SendCtsAfterRts,
							this,
							hdr.GetAddr2 (),
							hdr.GetDuration ());
	}
      break;

    case CSMACA_MAC_CTS:
      if (hdr.GetAddr1 () == GetAddress ())
	{
	  m_ctsTimeoutEvent.Cancel ();
	  m_lastCtsTimeoutEnd = Simulator::Now ();
	  m_sendDataAfterCtsEvent = Simulator::Schedule (m_sifs,
							 &CsmacaMac::SendDataAfterCts,
							 this);
	}
      break;

    }
}

void
CsmacaMac::ReceiveError (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  m_rxing = false;

  CsmacaMacHeader hdr;
  packet->RemoveHeader (hdr);
  NS_LOG_DEBUG (hdr);
}

void
CsmacaMac::SendRts ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_ctsTimeoutEvent.IsExpired ());

  CsmacaPreamble preamble;

  Time timerDelay = m_rtsSendAndSifsTime + m_ctsSendAndSifsTime;
  m_ctsTimeoutEvent = Simulator::Schedule (timerDelay, &CsmacaMac::CtsTimeout, this);
  m_lastCtsTimeoutEnd = Simulator::Now () + timerDelay;
  NS_LOG_DEBUG ("CTS Time out: " << m_lastCtsTimeoutEnd);

  Time txDuration = CalculateDataSendTime (preamble);

  CsmacaMacHeader rts;
  rts.SetType (CSMACA_MAC_RTS);
  rts.SetAddr1 (m_currentHdr.GetAddr1 ());
  rts.SetAddr2 (GetAddress ());
  rts.SetDuration (m_ctsSendAndSifsTime + txDuration + m_ackSendAndSifsTime);

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (rts);

  CsmacaMacTrailer fcs;
  packet->AddTrailer (fcs);

  m_phy->StartSend (packet, preamble); 
}

void
CsmacaMac::SendCtsAfterRts (Mac48Address source, Time duration)
{
  NS_LOG_FUNCTION (this);

  CsmacaPreamble preamble;

  CsmacaMacHeader cts;
  cts.SetType (CSMACA_MAC_CTS);
  cts.SetAddr1 (source);
  cts.SetDuration (duration - m_ctsSendAndSifsTime);

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (cts);

  CsmacaMacTrailer fcs;
  packet->AddTrailer (fcs);

  m_phy->StartSend (packet, preamble); 
}

void
CsmacaMac::SendDataNoAck ()
{
  NS_LOG_FUNCTION (this);

  CsmacaPreamble preamble;
  preamble.SetRate (m_rate);

  Ptr<Packet> packet = m_currentPacket->Copy ();
  m_currentHdr.SetDuration (Seconds (0));
  packet->AddHeader (m_currentHdr);

  CsmacaMacTrailer fcs;
  packet->AddTrailer (fcs);

  m_phy->StartSend (packet, preamble); 

  InitSend ();
  StartBackoffIfNeeded ();
}

void
CsmacaMac::SendDataAfterCts ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_ackTimeoutEvent.IsExpired ());

  CsmacaPreamble preamble;
  preamble.SetRate (m_rate);

  Time txDuration = CalculateDataSendTime (preamble);
  Time timerDelay = txDuration + m_ackSendAndSifsTime;

  m_ackTimeoutEvent = Simulator::Schedule (timerDelay, &CsmacaMac::AckTimeout, this);
  m_lastAckTimeoutEnd = Simulator::Now () + timerDelay;
  NS_LOG_DEBUG ("[ACK Time out] duration=" << timerDelay <<  ", end time=" << m_lastAckTimeoutEnd);

  Ptr<Packet> packet = m_currentPacket->Copy ();
  m_currentHdr.SetDuration (m_ackSendAndSifsTime);
  packet->AddHeader (m_currentHdr);
  
  CsmacaMacTrailer fcs;
  packet->AddTrailer (fcs);

  NS_LOG_DEBUG ("packet Size" << packet->GetSize());
  m_phy->StartSend (packet, preamble); 
}

void
CsmacaMac::SendAckAfterData (Mac48Address source)
{
  NS_LOG_FUNCTION (this);

  CsmacaPreamble preamble;

  CsmacaMacHeader ack;
  ack.SetType (CSMACA_MAC_ACK);
  ack.SetAddr1 (source);
  ack.SetDuration (Seconds (0));

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (ack);

  CsmacaMacTrailer fcs;
  packet->AddTrailer (fcs);

  m_phy->StartSend (packet, preamble); 
}

void
CsmacaMac::SetNav (Time duration)
{
  m_lastNavStart = Seconds (0);
  if (m_lastNavStart + m_lastNavDuration < Simulator::Now () + duration)
    {
      m_lastNavDuration = duration;
      m_lastNavStart = Simulator::Now ();
    }
}

void
CsmacaMac::Enqueue (Ptr<Packet const> packet, const CsmacaMacHeader &hdr)
{
  NS_LOG_FUNCTION (this);
  m_queue->Enqueue (packet, hdr);
  StartBackoffIfNeeded ();
}

void
CsmacaMac::StartBackoffIfNeeded ()
{
  NS_LOG_FUNCTION (this);
  if (m_currentPacket == 0 &&
      !m_queue->IsEmpty () &&
      m_backoffGrantStartEvent.IsExpired ())
    {
      m_currentPacket = m_queue->Dequeue (&m_currentHdr);
      BackoffGrantStart ();
    }
}

void
CsmacaMac::BackoffGrantStart ()
{
  NS_LOG_FUNCTION (this);
  Time backoffGrantStart = GetBackoffGrantStart ();
  if (backoffGrantStart <= Simulator::Now ())
    {
      StartBackoff ();
    }
  else
    {
      Time duration = backoffGrantStart - Simulator::Now ();
      m_backoffGrantStartEvent = Simulator::Schedule (duration, &CsmacaMac::BackoffGrantStart, this);
    }
}

void
CsmacaMac::StartBackoff ()
{
  NS_LOG_FUNCTION (this);
  m_backoffSlots = m_rng->GetNext (0, m_cw);
  m_backoffStart = Simulator::Now ();
  Time duration = m_backoffSlots * m_slotTime;
  NS_LOG_DEBUG ("slot: "   << m_backoffSlots <<
		", start: "<< m_backoffStart <<
		", end: "  << m_backoffSlots * m_slotTime + m_backoffStart);
  m_backoffTimeoutEvent = Simulator::Schedule (duration, &CsmacaMac::BackoffTimeout, this);
}

void
CsmacaMac::BackoffTimeout ()
{
  NS_LOG_FUNCTION (this);
  Time sendGrantStartTime = GetSendGrantStart ();
  Time backoffGrantStart = GetBackoffGrantStart ();
  if (sendGrantStartTime <= Simulator::Now ())
    {
      // Need RTS/CTS + ACK
      if (!m_currentHdr.GetAddr1 ().IsGroup ())
	{
	  if (m_rtsSendThreshold <= m_currentPacket->GetSize ())
	    {
	      SendRts ();
	    }
	  else
	    {
	      SendDataAfterCts ();
	    }
	}
      // Does not need  RTS/CTS + ACK
      else
	{
	  SendDataNoAck ();
	}
    }
  else
    {
      Time duration = backoffGrantStart - Simulator::Now ();
      m_backoffGrantStartEvent = Simulator::Schedule (duration, &CsmacaMac::BackoffGrantStart, this);
    }
}

void
CsmacaMac::CtsTimeout ()
{
  NS_LOG_FUNCTION (this << m_resendRtsNum);
  if (m_resendRtsMax > m_resendRtsNum)
    {
      m_resendRtsNum++;
      UpdateCw ();
      BackoffGrantStart ();
    }
  else
    {
      InitSend ();
      StartBackoffIfNeeded ();
    }
}

void
CsmacaMac::AckTimeout ()
{
  NS_LOG_FUNCTION (this << m_resendDataNum);
  if (m_resendDataMax > m_resendDataNum)
    {
      m_resendDataNum++;
      UpdateCw ();
      BackoffGrantStart ();
    }
  else
    {
      InitSend ();
      StartBackoffIfNeeded ();
    }
}

} // namespace ns3
