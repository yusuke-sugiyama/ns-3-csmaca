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

#ifndef CSMACA_CHANNEL_H
#define CSMACA_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/csmaca-phy.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/mobility-model.h"
#include <vector>

#include "csmaca-preamble.h"
#include "csmaca-channel.h"

namespace ns3 {
class CsmacaPhy;
class CsmacaNetDevice;
class PropagationLossModel;
class PropagationDelayModel;

class CsmacaChannel : public Channel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  CsmacaChannel ();

  void SetPropagationLossModel (Ptr<PropagationLossModel> loss);
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);
  void Add (Ptr<CsmacaPhy> phy);
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

  void Send (Ptr<Packet> packet, CsmacaPreamble preamble, double txPowerDbm, Ptr<CsmacaPhy> sender) const; 
  void Receive (Ptr<Packet> packet, CsmacaPreamble preamble, double rxPowerDbm, uint32_t i) const;

private:
  typedef std::vector<Ptr<CsmacaPhy> > PhyList;
  PhyList m_phyList;
  Ptr<PropagationLossModel> m_loss;
  Ptr<PropagationDelayModel> m_delay;
};

} // namespace ns3

#endif /* CSMACA_CHANNEL_H */
