/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "csmaca-mac-trailer.h"
#include "ns3/assert.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmacaMacTrailer);

CsmacaMacTrailer::CsmacaMacTrailer ()
{
}

CsmacaMacTrailer::~CsmacaMacTrailer ()
{
}

TypeId
CsmacaMacTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmacaMacTrailer")
    .SetParent<Trailer> ()
    .AddConstructor<CsmacaMacTrailer> ()
  ;
  return tid;
}
TypeId
CsmacaMacTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CsmacaMacTrailer::Print (std::ostream &os) const
{
}
uint32_t
CsmacaMacTrailer::GetSerializedSize (void) const
{
  return CSMACA_MAC_FCS_LENGTH;
}
uint32_t
CsmacaMacTrailer::GetSize (void) const
{
  return CSMACA_MAC_FCS_LENGTH;
}
void
CsmacaMacTrailer::Serialize (Buffer::Iterator start) const
{
  start.Prev (CSMACA_MAC_FCS_LENGTH);
  start.WriteU32 (0);
}
uint32_t
CsmacaMacTrailer::Deserialize (Buffer::Iterator start)
{
  return CSMACA_MAC_FCS_LENGTH;
}

} // namespace ns3
