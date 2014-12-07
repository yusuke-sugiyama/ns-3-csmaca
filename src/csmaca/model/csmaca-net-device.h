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
#ifndef CSMACA_NET_DEVICE_H
#define CSMACA_NET_DEVICE_H

#include <stdint.h>
#include <string>
#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "csmaca-mac.h"
#include "csmaca-phy.h"

namespace ns3 {

class CsmacaMac;
class CsmacaPhy;
class Node;

/**
 * \ingroup netdevice
 *
 * This device does not have a helper and assumes 48-bit mac addressing;
 * the default address assigned to each device is zero, so you must 
 * assign a real address to use it.  There is also the possibility to
 * add an ErrorModel if you want to force losses on the device.
 * 
 * \brief csmaca net device for csmaca things and testing
 */
class CsmacaNetDevice : public NetDevice
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  CsmacaNetDevice ();
  void SetPhy (Ptr<CsmacaPhy> phy);
  void SetMac (Ptr<CsmacaMac> mac);
  Ptr<CsmacaPhy> GetPhy ();
  Ptr<CsmacaMac> GetMac ();

  void Receive (Ptr<Packet> packet, Mac48Address to, Mac48Address from);
  

  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

protected:
  virtual void DoDispose (void);
private:
  NetDevice::ReceiveCallback m_rxCallback; //!< Receive callback
  NetDevice::PromiscReceiveCallback m_promiscCallback; //!< Promiscuous receive callback
  Ptr<Node> m_node; //!< Node this netDevice is associated to
  Ptr<CsmacaMac> m_mac; //!< Mac this netDevice is associated to
  Ptr<CsmacaPhy> m_phy; //!< Mac this netDevice is associated to
  uint16_t m_mtu;   //!< MTU
  uint32_t m_ifIndex; //!< Interface index
  Mac48Address m_address; //!< MAC address

  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;
};

} // namespace ns3

#endif /* CSMACA_NET_DEVICE_H */
