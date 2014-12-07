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

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "csmaca-mac-header.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmacaMacHeader);

enum
{
  TYPE_DATA = 0,
  TYPE_ACK  = 1,
  TYPE_RTS  = 2,
  TYPE_CTS  = 3,
};

CsmacaMacHeader::CsmacaMacHeader ()
{
}
CsmacaMacHeader::~CsmacaMacHeader ()
{
}

void
CsmacaMacHeader::SetAddr1 (Mac48Address address)
{
  m_addr1 = address;
}
void
CsmacaMacHeader::SetAddr2 (Mac48Address address)
{
  m_addr2 = address;
}

void
CsmacaMacHeader::SetType (enum CsmacaMacType type)
{
  switch (type)
    {
    case CSMACA_MAC_DATA:
      m_ctrlType = TYPE_DATA;
      break;
    case CSMACA_MAC_ACK:
      m_ctrlType = TYPE_ACK;
      break;
    case CSMACA_MAC_RTS:
      m_ctrlType = TYPE_RTS;
      break;
    case CSMACA_MAC_CTS:
      m_ctrlType = TYPE_CTS;
      break;
    }
}

void
CsmacaMacHeader::SetDuration (Time duration)
{
  int64_t duration_us = duration.GetMicroSeconds ();
  NS_ASSERT (duration_us >= 0 && duration_us <= 0x7fff);
  m_duration = static_cast<uint16_t> (duration_us);
}


Mac48Address
CsmacaMacHeader::GetAddr1 (void) const
{
  return m_addr1;
}
Mac48Address
CsmacaMacHeader::GetAddr2 (void) const
{
  return m_addr2;
}


enum CsmacaMacType
CsmacaMacHeader::GetType (void) const
{
  switch (m_ctrlType)
    {
    case TYPE_DATA:
      return CSMACA_MAC_DATA;
      break;
    case TYPE_ACK:
      return CSMACA_MAC_ACK;
      break;
    case TYPE_RTS:
      return CSMACA_MAC_RTS;
      break;
    case TYPE_CTS:
      return CSMACA_MAC_CTS;
      break;
    }
  NS_ASSERT (false);
  return (enum CsmacaMacType)-1;
}


Time
CsmacaMacHeader::GetDuration (void) const
{
  return MicroSeconds (m_duration);
}

uint32_t
CsmacaMacHeader::GetSize (void) const
{
  uint32_t size = 0;
  switch (m_ctrlType)
    {
    case TYPE_DATA:
      size = 2 + 2 + 6 + 6;
      break;
    case TYPE_ACK:
      size = 2 + 2 + 6;
      break;
    case TYPE_RTS:
      size = 2 + 2 + 6 + 6;
      break;
    case TYPE_CTS:
      size = 2 + 2 + 6;
      break;
    }
  return size;
}

TypeId
CsmacaMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmacaMacHeader")
    .SetParent<Header> ()
    .AddConstructor<CsmacaMacHeader> ()
  ;
  return tid;
}

TypeId
CsmacaMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CsmacaMacHeader::Print (std::ostream &os) const
{
  os << "type=" << GetType ();
  os << ", Duration/ID=" << m_duration << "us";
  switch (GetType ())
    {
    case TYPE_DATA:
      os << ", DA=" << m_addr1 << ", SA=" << m_addr2;
      break;
    case TYPE_ACK:
      os << "DA=" << m_addr1;
      break;
    case TYPE_RTS:
      os << ", DA=" << m_addr1 << ", SA=" << m_addr2;
      break;
    case TYPE_CTS:
      os << "DA=" << m_addr1;
      break;
    }
}
uint16_t
CsmacaMacHeader::GetFrameControl (void) const
{
  uint16_t val = 0;
  val |= m_ctrlType & 0x3;
  return val;
}
void
CsmacaMacHeader::SetFrameControl (uint16_t ctrl)
{
  m_ctrlType = ctrl & 0x03;
}
uint32_t
CsmacaMacHeader::GetSerializedSize (void) const
{
  return GetSize ();
}
void
CsmacaMacHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteHtolsbU16 (GetFrameControl ());
  i.WriteHtolsbU16 (m_duration);
  WriteTo (i, m_addr1);
  switch (m_ctrlType)
    {
    case TYPE_DATA:
      WriteTo (i, m_addr2);
      break;
    case TYPE_ACK:
      // do nothing
      break;
    case TYPE_RTS:
      WriteTo (i, m_addr2);
      break;
    case TYPE_CTS:
      // do nothing
      break;
    }
}

uint32_t
CsmacaMacHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint16_t frame_control = i.ReadLsbtohU16 ();
  SetFrameControl (frame_control);
  m_duration = i.ReadLsbtohU16 ();
  ReadFrom (i, m_addr1);
  switch (m_ctrlType)
    {
    case TYPE_DATA:
      ReadFrom (i, m_addr2);
      break;
    case TYPE_ACK:
      // do nothing
      break;
    case TYPE_RTS:
      ReadFrom (i, m_addr2);
      break;
    case TYPE_CTS:
      // do nothing
      break;
    }
  return i.GetDistanceFrom (start);
}

} // namespace ns3
