/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/llc-snap-header.h"
#include "csmaca-net-device.h"

NS_LOG_COMPONENT_DEFINE ("CsmacaNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmacaNetDevice);

TypeId 
CsmacaNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmacaNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<CsmacaNetDevice> ()
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&CsmacaNetDevice::m_phyRxDropTrace))
  ;
  return tid;
}

CsmacaNetDevice::CsmacaNetDevice ()
  : m_node (0),
    m_mtu (0xffff),
    m_ifIndex (0)
{
  NS_LOG_FUNCTION (this);
  SetMac (CreateObject<CsmacaMac>());
  GetMac ()->SetNetDevice (this);
  SetPhy (GetMac()->GetPhy());
}
void
CsmacaNetDevice::SetPhy (Ptr<CsmacaPhy> phy)
{
  m_phy = phy;
}
void
CsmacaNetDevice::SetMac (Ptr<CsmacaMac> mac)
{
  m_mac = mac;
}
Ptr<CsmacaPhy>
CsmacaNetDevice::GetPhy ()
{
  return m_phy;
}
Ptr<CsmacaMac>
CsmacaNetDevice::GetMac ()
{
  return m_mac;
}

void
CsmacaNetDevice::Receive (Ptr<Packet> packet, Mac48Address to, Mac48Address from)
{
  NS_LOG_FUNCTION (this << packet << to << from);
  NetDevice::PacketType packetType;
  LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  NS_LOG_DEBUG (llc.GetType ());
  
  if (to == m_address)
    {
      packetType = NetDevice::PACKET_HOST;
    }
  else if (to.IsBroadcast ())
    {
      packetType = NetDevice::PACKET_HOST;
    }
  else if (to.IsGroup ())
    {
      packetType = NetDevice::PACKET_MULTICAST;
    }
  else 
    {
      packetType = NetDevice::PACKET_OTHERHOST;
    }
  m_rxCallback (this, packet, llc.GetType (), from);
  if (!m_promiscCallback.IsNull ())
    {
      m_promiscCallback (this, packet, llc.GetType (), from, to, packetType);
    }
}

void 
CsmacaNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}
uint32_t 
CsmacaNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}
Ptr<Channel>
CsmacaNetDevice::GetChannel (void) const
{
  return m_phy->GetChannel ();
}
void
CsmacaNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
  m_mac->SetAddress (m_address);
}
Address 
CsmacaNetDevice::GetAddress (void) const
{
  //
  // Implicit conversion from Mac48Address to Address
  //
  NS_LOG_FUNCTION (this);
  return m_address;
}
bool 
CsmacaNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}
uint16_t 
CsmacaNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}
bool 
CsmacaNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void 
CsmacaNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
 NS_LOG_FUNCTION (this << &callback);
}
bool 
CsmacaNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
Address
CsmacaNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}
bool 
CsmacaNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
Address 
CsmacaNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address CsmacaNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

bool 
CsmacaNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool 
CsmacaNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool 
CsmacaNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);

  CsmacaMacHeader hdr;
  Mac48Address to = Mac48Address::ConvertFrom (dest);
  hdr.SetType (CSMACA_MAC_DATA);
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (m_mac->GetAddress ());
  m_mac->Enqueue (packet, hdr);
  return true;
}
bool 
CsmacaNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
  //  Mac48Address to = Mac48Address::ConvertFrom (dest);
  //  Mac48Address from = Mac48Address::ConvertFrom (source);
  //  m_mac->Send (packet, protocolNumber, to, from);
  return true;
}

Ptr<Node> 
CsmacaNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}
void 
CsmacaNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}
bool 
CsmacaNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void 
CsmacaNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

void
CsmacaNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_mac = 0;
  NetDevice::DoDispose ();
}


void
CsmacaNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscCallback = cb;
}

bool
CsmacaNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

} // namespace ns3
