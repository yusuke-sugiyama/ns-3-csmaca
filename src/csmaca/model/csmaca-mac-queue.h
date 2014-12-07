/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005, 2009 INRIA
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

#ifndef CSMACA_MAC_QUEUE_H
#define CSMACA_MAC_QUEUE_H

#include <list>
#include <utility>
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "csmaca-mac-header.h"

namespace ns3 {

class CsmacaMacQueue : public Object
{
public:
  static TypeId GetTypeId (void);
  CsmacaMacQueue ();
  ~CsmacaMacQueue ();

  /**
   * Set the maximum queue size.
   *
   * \param maxSize the maximum queue size
   */
  void SetMaxSize (uint32_t maxSize);
  /**
   * Return the maximum queue size.
   *
   * \return the maximum queue size
   */
  uint32_t GetMaxSize (void) const;

  /**
   * Enqueue the given packet and its corresponding CsmacaMacHeader at the <i>end</i> of the queue.
   *
   * \param packet the packet to be euqueued at the end
   * \param hdr the header of the given packet
   */
  void Enqueue (Ptr<const Packet> packet, const CsmacaMacHeader &hdr);
  /**
   * Enqueue the given packet and its corresponding CsmacaMacHeader at the <i>front</i> of the queue.
   *
   * \param packet the packet to be euqueued at the end
   * \param hdr the header of the given packet
   */
  void PushFront (Ptr<const Packet> packet, const CsmacaMacHeader &hdr);
  /**
   * Dequeue the packet in the front of the queue.
   *
   * \param hdr the CsmacaMacHeader of the packet
   * \return the packet
   */
  Ptr<const Packet> Dequeue (CsmacaMacHeader *hdr);
  /**
   * Peek the packet in the front of the queue. The packet is not removed.
   *
   * \param hdr the CsmacaMacHeader of the packet
   * \return the packet
   */
  Ptr<const Packet> Peek (CsmacaMacHeader *hdr);

  /**
   * If exists, removes <i>packet</i> from queue and returns true. Otherwise it
   * takes no effects and return false. Deletion of the packet is
   * performed in linear time (O(n)).
   *
   * \param packet the packet to be removed
   * \return true if the packet was removed, false otherwise
   */
  bool Remove (Ptr<const Packet> packet);

  /**
   * Flush the queue.
   */
  void Flush (void);

  /**
   * Return if the queue is empty.
   *
   * \return true if the queue is empty, false otherwise
   */
  bool IsEmpty (void);
  /**
   * Return the current queue size.
   *
   * \return the current queue size
   */
  uint32_t GetSize (void);
protected:

  struct Item;

  /**
   * typedef for packet (struct Item) queue.
   */
  typedef std::list<struct Item> PacketQueue;
  /**
   * typedef for packet (struct Item) queue reverse iterator.
   */
  typedef std::list<struct Item>::reverse_iterator PacketQueueRI;
  /**
   * typedef for packet (struct Item) queue iterator.
   */
  typedef std::list<struct Item>::iterator PacketQueueI;

  /**
   * A struct that holds information about a packet for putting
   * in a packet queue.
   */
  struct Item
  {
    /**
     * Create a struct with the given parameters.
     *
     * \param packet
     * \param hdr
     * \param tstamp
     */
    Item (Ptr<const Packet> packet,
          const CsmacaMacHeader &hdr,
          Time tstamp);
    Ptr<const Packet> packet; //!< Actual packet
    CsmacaMacHeader hdr; //!< Csmaca MAC header associated with the packet
    Time tstamp; //!< timestamp when the packet arrived at the queue
  };

  PacketQueue m_queue; //!< Packet (struct Item) queue
  uint32_t m_size; //!< Current queue size
  uint32_t m_maxSize; //!< Queue capacity
};

} // namespace ns3

#endif /* CSMACA_MAC_QUEUE_H */
