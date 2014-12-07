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

#include "csmaca-preamble.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("CsmacaPreamble");

namespace ns3 {

CsmacaPreamble::CsmacaPreamble ()
  : m_rate (6000000 / 8),
    m_bandwidth (20000000),
    m_duration (MicroSeconds (20))
{
}

CsmacaPreamble::~CsmacaPreamble ()
{
}

void
CsmacaPreamble::SetRate (uint32_t rate){
  m_rate = rate;
}

void
CsmacaPreamble::SetBandwidth (uint32_t bandwidth){
  m_bandwidth = bandwidth;
}

void
CsmacaPreamble::SetDuration (Time duration){
  m_duration = duration;
}

uint32_t
CsmacaPreamble::GetRate (){
  return m_rate;
}

uint32_t
CsmacaPreamble::GetBandwidth (){
  return m_bandwidth;
}

Time
CsmacaPreamble::GetDuration (){
  return m_duration;
}
}
