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

#ifndef CSMACA_PHY_STATE_HELPER_H
#define CSMACA_PHY_STATE_HELPER_H

#include "csmaca-phy-state.h"
#include <stdint.h>
#include <string>
#include "ns3/csmaca-phy.h"
#include "ns3/traced-callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"

namespace ns3 {
class CsmacaPhyListener
{
public:
  virtual ~CsmacaPhyListener ();
  virtual void NotifyRxEndOk (Ptr<Packet> packet) = 0;
  virtual void NotifyRxEndError (Ptr<Packet> packet) = 0;
  virtual void NotifyMaybeCcaBusyStart (Time duration) = 0;
  virtual void NotifyTxStart (Time duration) = 0;
  virtual void NotifyRxStart (Time duration) = 0;
};

class CsmacaPhyStateHelper: public Object
{
public:

  CsmacaPhyStateHelper ();

  bool IsStateIdle (void);
  bool IsStateBusy (void);
  bool IsStateCcaBusy (void);
  bool IsStateRx (void);
  bool IsStateTx (void);
  enum CsmacaPhyState::State GetState (void);

  void NotifyMaybeCcaBusyStart (Time duration);
  void NotifyTxStart (Time duration);
  void NotifyRxStart (Time duration);

  void SwitchMaybeToCcaBusy (Time duration);
  void SwitchToTx (Time duration);
  void SwitchToRx (Time duration);

  void EndReceiveOk (Ptr<Packet> packet);
  void EndReceiveError (Ptr<Packet> packet);
  void RegisterListener (CsmacaPhyListener *listener);

private:
  typedef std::vector<CsmacaPhyListener *> Listeners;
  Time m_startTx;
  Time m_startRx;
  Time m_startCcaBusy;
  Time m_endTx;
  Time m_endRx;
  Time m_endCcaBusy;
  Listeners m_listeners;
  bool m_rxing;
};

} // namespace ns3

#endif /* CSMACA_PHY_STATE_HELPER_H */
