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

#include "csmaca-channel.h"
#include "csmaca-net-device.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("CsmacaChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmacaChannel);

TypeId 
CsmacaChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmacaChannel")
    .SetParent<Channel> ()
    .AddConstructor<CsmacaChannel> ()
    ;
  return tid;
}
  
CsmacaChannel::CsmacaChannel ()
{
  NS_LOG_FUNCTION (this);

  ObjectFactory factoryLoss;
  factoryLoss.SetTypeId ("ns3::LogDistancePropagationLossModel");
  Ptr<PropagationLossModel> loss = factoryLoss.Create<PropagationLossModel> ();
  SetPropagationLossModel (loss);

  ObjectFactory factoryDelay;
  factoryDelay.SetTypeId ("ns3::ConstantSpeedPropagationDelayModel");
  Ptr<PropagationDelayModel> delay = factoryDelay.Create<PropagationDelayModel> ();
  SetPropagationDelayModel (delay);
}

void
CsmacaChannel::SetPropagationLossModel (Ptr<PropagationLossModel> loss)
{
  m_loss = loss;
}

void
CsmacaChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  m_delay = delay;
}

uint32_t
CsmacaChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyList.size ();
}

Ptr<NetDevice>
CsmacaChannel::GetDevice (uint32_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
CsmacaChannel::Add (Ptr<CsmacaPhy> phy)
{
  m_phyList.push_back (phy);
}
  
void
CsmacaChannel::Send (Ptr<Packet> packet, CsmacaPreamble preamble, double txPowerDbm, Ptr<CsmacaPhy> sender) const
{
  NS_LOG_FUNCTION (this);
  Ptr<MobilityModel> senderMobility = sender->GetMobility ()->GetObject<MobilityModel> ();
  uint32_t j = 0;
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++, j++)
    {
      if (sender == (*i))
	{
	  continue;
	}
      Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
      Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
      double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
      Ptr<Packet> copy = packet->Copy ();
      Ptr<Object> dstNetDevice = m_phyList[j]->GetDevice ();
      uint32_t dstNode;
      if (dstNetDevice == 0)
	{
	  dstNode = 0xffffffff;
	}
      else
	{
	  dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode ()->GetId ();
	}
      NS_LOG_DEBUG ("rxPower=" << rxPowerDbm << ", delay=" << delay);
      Simulator::ScheduleWithContext (dstNode,
				      delay,
				      &CsmacaChannel::Receive,
				      this,
				      copy,
				      preamble,
				      rxPowerDbm,
				      j);
    }
}

void
CsmacaChannel::Receive (Ptr<Packet> packet, CsmacaPreamble preamble, double rxPowerDbm, uint32_t i) const
{
  NS_LOG_FUNCTION (this);
  m_phyList[i]->StartReceive (packet, preamble, rxPowerDbm);
}

} // namespace ns3
