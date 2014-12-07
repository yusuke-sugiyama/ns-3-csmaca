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

#ifndef CSMACA_MAC_H
#define CSMACA_MAC_H

#include <stdint.h>
#include <string>
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/ptr.h"
#include "csmaca-random-stream.h"
#include "csmaca-preamble.h"
#include "csmaca-mac-queue.h"
#include "csmaca-phy.h"
#include "csmaca-phy-state-helper.h"
#include "csmaca-net-device.h"

namespace ns3 {

class CsmacaRandomStream;
class CsmacaPhy;
class CsmacaPhyStateHelper;

class CsmacaMac: public Object
{
public:
  CsmacaMac ();
  ~CsmacaMac ();

  static TypeId GetTypeId (void);

  int64_t AssignStreams (int64_t stream);

  Ptr<CsmacaPhy> GetPhy ();
  Ptr<CsmacaMacQueue> GetQueue ();
  Mac48Address GetAddress ();
  void SetPhy (Ptr<CsmacaPhy> phy);
  void SetQueue (Ptr<CsmacaMacQueue> queue);
  void SetAddress (Mac48Address);
  void SetNetDevice (Ptr<CsmacaNetDevice> device);

  void ReceiveOk (Ptr<Packet> packet);
  void ReceiveError (Ptr<Packet> packet);

  void NotifyMaybeCcaBusyStartNow (Time duration);
  void NotifyTxStartNow (Time duration);
  void NotifyRxStartNow (Time duration);
  void SetupPhyCsmacaMacListener (Ptr<CsmacaPhyStateHelper> state);
  void Enqueue (Ptr<Packet const> packet, const CsmacaMacHeader &hdr);

  void StartBackoffIfNeeded ();
  void StartBackoff ();
  Time GetBackoffGrantStart (void) const;
  Time GetSendGrantStart (void) const;

  void UpdateCw ();
  void InitSend ();
  void SetNav (Time duration);
  Time CalculateDataSendTime (CsmacaPreamble preamble);

  void SendRts ();
  void SendCtsAfterRts (Mac48Address source, Time duration);
  void SendDataNoAck ();
  void SendDataAfterCts ();
  void SendAckAfterData (Mac48Address source);

  void BackoffGrantStart ();
  void BackoffTimeout ();
  void AckTimeout ();
  void CtsTimeout ();

private:
  class PhyCsmacaMacListener *m_phyCsmacaMacListener;
  Ptr<CsmacaPhy> m_phy;
  Ptr<CsmacaMacQueue> m_queue;
  Ptr<CsmacaNetDevice> m_device;
  CsmacaRandomStream *m_rng;

  Mac48Address m_address;
  Ptr<Packet const> m_currentPacket;
  CsmacaMacHeader m_currentHdr;

  uint32_t m_rtsSendThreshold;

  Time m_maxPropagationDelay;
  Time m_rtsSendAndSifsTime;
  Time m_ctsSendAndSifsTime;
  Time m_ackSendAndSifsTime;
  Time m_rtsNavDuration;

  uint16_t m_resendRtsNum;
  uint16_t m_resendRtsMax;
  uint16_t m_resendDataNum;
  uint16_t m_resendDataMax;

  uint32_t m_cwMin;
  uint32_t m_cwMax;
  uint32_t m_cw;
  uint32_t m_backoffSlots;
  Time m_sifs;
  Time m_difs;
  Time m_slotTime;
  Time m_backoffStart;

  Time m_lastRxStart;
  Time m_lastRxDuration;
  Time m_lastBusyStart;
  Time m_lastBusyDuration;
  Time m_lastTxStart;
  Time m_lastTxDuration;
  Time m_lastNavStart;
  Time m_lastNavDuration;
  Time m_lastAckTimeoutEnd;
  Time m_lastCtsTimeoutEnd;
  bool m_rxing;

  uint32_t m_rate;

  EventId m_sendCtsAfterRtsEvent;
  EventId m_sendDataAfterCtsEvent;
  EventId m_sendAckAfterDataEvent;
  EventId m_ackTimeoutEvent;
  EventId m_ctsTimeoutEvent;
  EventId m_backoffTimeoutEvent;
  EventId m_backoffGrantStartEvent;


};

} // namespace ns3

#endif /* CSMACA_MAC_H */
