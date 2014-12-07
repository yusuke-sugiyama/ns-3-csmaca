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

#ifndef SIMPLE_PREAMBLE_H
#define SIMPLE_PREAMBLE_H

#include <stdint.h>
#include "ns3/nstime.h"


namespace ns3 {

class CsmacaPreamble
{
public:
  CsmacaPreamble ();
  ~CsmacaPreamble ();
  void SetRate (uint32_t rate);
  void SetBandwidth (uint32_t bandwidth);
  void SetDuration (Time duration);
  uint32_t GetRate ();
  uint32_t GetBandwidth ();
  Time GetDuration ();
private:
  uint32_t m_rate;
  uint32_t m_bandwidth;
  /*
    0. preamble + layer 1 header
      preamble      : 12 [symbols] 16 [us]
      layer 1 header:  1 [symbols]  4 [us]

    1. layer 1 header = |Rate|Symbols|Tail|
      Rate:     6 [bits]
      Symbols: 12 [bits]
      Tail:     6 [bits]
      Total:   24 [bits]
   */
  Time m_duration;
};
}

#endif /* SIMPLE_PREAMBLE_H */
