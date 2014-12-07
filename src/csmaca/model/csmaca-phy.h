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

#ifndef CSMACA_PHY_H
#define CSMACA_PHY_H


#include <stdint.h>
#include <string>
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/random-variable-stream.h"
#include "csmaca-random-stream.h"
#include "csmaca-channel.h"
#include "csmaca-mac.h"
#include "csmaca-phy-state.h"
#include "csmaca-phy-state-helper.h"
#include "csmaca-preamble.h"
#include "csmaca-interference-helper.h"

namespace ns3 {

class CsmacaPhyStateHelper;
class CsmacaChannel;

class CsmacaPhy: public Object
{
public:

  CsmacaPhy ();
  ~CsmacaPhy ();

  static TypeId GetTypeId (void);

  typedef enum State
  {
    IDLE,
    TX,
    RX,
    CCA_BUSY
  }State;

  void SetMobility (Ptr<Object> mobility);
  void SetDevice (Ptr<Object> device);
  Ptr<Object> GetMobility ();
  Ptr<CsmacaPhyStateHelper> GetPhyStateHelper () const;
  Ptr<CsmacaChannel> GetChannel () const;
  Ptr<Object> GetDevice () const;
  int64_t AssignStreams (int64_t stream);

  void StartSend (Ptr<Packet> pacekt, CsmacaPreamble preamble);
  void StartReceive (Ptr<Packet> packet, CsmacaPreamble preamble, double rxPowerDbm);
  void EndReceive (Ptr<Packet> packet, Ptr<CsmacaInterferenceHelper::Event> event);
  double DbToRatio (double dB) const;
  double DbmToW (double dBm) const;
  double RatioToDb (double ratio) const;

protected:
  virtual void DoDispose (void);
private:
  Ptr<CsmacaChannel> m_channel;
  Ptr<Object> m_mobility;
  Ptr<CsmacaPhyStateHelper> m_state;
  Ptr<Object> m_device;
  Ptr<UniformRandomVariable> m_random;

  CsmacaInterferenceHelper m_interference;

  double m_edThresholdW;
  double m_ccaMode1ThresholdW;
  double m_txGainDb;
  double m_rxGainDb;
  double m_txPowerDbm;
  double m_rxNoiseFigureDb;

  EventId m_endRxEvent;
  TracedCallback<Ptr<Packet> > m_txTrace;
};

} // namespace ns3

#endif /* CSMACA_PHY_H */
