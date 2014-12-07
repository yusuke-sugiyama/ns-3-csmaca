/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Author: Mirko Banchi <mk.banchi@gmail.com>
 */
#ifndef CSMACA_MAC_HEADER_H
#define CSMACA_MAC_HEADER_H

#include "ns3/header.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include <stdint.h>

namespace ns3 {

/**
 * Combination of valid MAC header type/subtype.
 */
enum CsmacaMacType
{
  CSMACA_MAC_DATA = 0,
  CSMACA_MAC_ACK,
  CSMACA_MAC_RTS,
  CSMACA_MAC_CTS
};

/**
 * \ingroup wifi
 *
 * Implements the IEEE 802.11 MAC header
 */
class CsmacaMacHeader : public Header
{
public:

  /**
   * Address types.
   */
  enum AddressType
  {
    ADDR1,
    ADDR2
  };

  CsmacaMacHeader ();
  ~CsmacaMacHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  void SetTypeData (void);
  void SetAddr1 (Mac48Address address);
  void SetAddr2 (Mac48Address address);
  void SetType (enum CsmacaMacType type);
  void SetDuration (Time duration);

  Mac48Address GetAddr1 (void) const;
  Mac48Address GetAddr2 (void) const;
  enum CsmacaMacType GetType (void) const;
  Time GetDuration (void) const;
  uint16_t GetFrameControl (void) const;
  uint32_t GetSize (void) const;
  const char * GetTypeString (void) const;

private:
  void SetFrameControl (uint16_t ctrl);
  void SetSequenceControl (uint16_t seq);
  void PrintFrameControl (std::ostream &os) const;

  uint8_t m_ctrlType;
  uint16_t m_duration;
  Mac48Address m_addr1;
  Mac48Address m_addr2;
  uint16_t m_seqSeq;
};

} // namespace ns3



#endif /* CSMACA_MAC_HEADER_H */
