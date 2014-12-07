/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "csmaca-interference-helper.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>
#include "ns3/math.h"

NS_LOG_COMPONENT_DEFINE ("CsmacaInterferenceHelper");

namespace ns3 {

/****************************************************************
 *       Phy event class
 ****************************************************************/

CsmacaInterferenceHelper::Event::Event (uint32_t size, Time duration, double rxPower, CsmacaPreamble preamble)
  : m_size (size),
    m_startTime (Simulator::Now ()),
    m_endTime (m_startTime + duration),
    m_rxPowerW (rxPower),
    m_preamble (preamble)
    
{
}
CsmacaInterferenceHelper::Event::~Event ()
{
}

Time
CsmacaInterferenceHelper::Event::GetDuration (void) const
{
  return m_endTime - m_startTime;
}
Time
CsmacaInterferenceHelper::Event::GetStartTime (void) const
{
  return m_startTime;
}
Time
CsmacaInterferenceHelper::Event::GetEndTime (void) const
{
  return m_endTime;
}
double
CsmacaInterferenceHelper::Event::GetRxPowerW (void) const
{
  return m_rxPowerW;
}
uint32_t
CsmacaInterferenceHelper::Event::GetSize (void) const
{
  return m_size;
}
CsmacaPreamble
CsmacaInterferenceHelper::Event::GetPreamble (void) const
{
  return m_preamble;
}

/****************************************************************
 *       Class which records SNIR change events for a
 *       short period of time.
 ****************************************************************/

CsmacaInterferenceHelper::NiChange::NiChange (Time time, double delta)
  : m_time (time),
    m_delta (delta)
{
}
Time
CsmacaInterferenceHelper::NiChange::GetTime (void) const
{
  return m_time;
}
double
CsmacaInterferenceHelper::NiChange::GetDelta (void) const
{
  return m_delta;
}
bool
CsmacaInterferenceHelper::NiChange::operator < (const CsmacaInterferenceHelper::NiChange& o) const
{
  return (m_time < o.m_time);
}

/****************************************************************
 *       The actual CsmacaInterferenceHelper
 ****************************************************************/

CsmacaInterferenceHelper::CsmacaInterferenceHelper ()
  : m_firstPower (0.0),
    m_rxing (false)
{
}
CsmacaInterferenceHelper::~CsmacaInterferenceHelper ()
{
  EraseEvents ();
}

Ptr<CsmacaInterferenceHelper::Event>
CsmacaInterferenceHelper::Add (uint32_t size, Time duration, double rxPowerW, CsmacaPreamble preamble)
{
  Ptr<CsmacaInterferenceHelper::Event> event;

  event = Create<CsmacaInterferenceHelper::Event> (size,
                                             duration,
                                             rxPowerW,
                                             preamble);
  AppendEvent (event);
  return event;
}


void
CsmacaInterferenceHelper::SetNoiseFigure (double value)
{
  m_noiseFigure = value;
}

double
CsmacaInterferenceHelper::GetNoiseFigure (void) const
{
  return m_noiseFigure;
}

Time
CsmacaInterferenceHelper::GetEnergyDuration (double energyW)
{
  Time now = Simulator::Now ();
  double noiseInterferenceW = 0.0;
  Time end = now;
  noiseInterferenceW = m_firstPower;
  for (NiChanges::const_iterator i = m_niChanges.begin (); i != m_niChanges.end (); i++)
    {
      noiseInterferenceW += i->GetDelta ();
      end = i->GetTime ();
      if (end < now)
        {
          continue;
        }
      if (noiseInterferenceW < energyW)
        {
          break;
        }
    }
  return end > now ? end - now : MicroSeconds (0);
}

void
CsmacaInterferenceHelper::AppendEvent (Ptr<CsmacaInterferenceHelper::Event> event)
{
  Time now = Simulator::Now ();
  CsmacaPreamble pre = event->GetPreamble();
  if (!m_rxing)
    {
      NiChanges::iterator nowIterator = GetPosition (now);
      for (NiChanges::iterator i = m_niChanges.begin (); i != nowIterator; i++)
        {
          m_firstPower += i->GetDelta ();
        }
      m_niChanges.erase (m_niChanges.begin (), nowIterator);
      m_niChanges.insert (m_niChanges.begin (), NiChange (event->GetStartTime (), event->GetRxPowerW ()));
    }
  else
    {
      AddNiChangeEvent (NiChange (event->GetStartTime (), event->GetRxPowerW ()));
    }
  AddNiChangeEvent (NiChange (event->GetEndTime (), -event->GetRxPowerW ()));

}


double
CsmacaInterferenceHelper::CalculateSnr (double signal, double noiseInterference, CsmacaPreamble preamble) const
{
  // thermal noise at 290K in J/s = W
  static const double BOLTZMANN = 1.3803e-23;
  // Nt is the power of thermal noise in W
  double Nt = BOLTZMANN * 290.0 * preamble.GetBandwidth ();
  // receiver noise Floor (W) which accounts for thermal noise and non-idealities of the receiver
  double noiseFloor = m_noiseFigure * Nt;
  double noise = noiseFloor + noiseInterference;
  double snr = signal / noise;
  return snr;
}

double
CsmacaInterferenceHelper::CalculateNoiseInterferenceW (Ptr<CsmacaInterferenceHelper::Event> event, NiChanges *ni) const
{
  double noiseInterference = m_firstPower;
  NS_ASSERT (m_rxing);
  for (NiChanges::const_iterator i = m_niChanges.begin () + 1; i != m_niChanges.end (); i++)
    {
      if ((event->GetEndTime () == i->GetTime ()) && event->GetRxPowerW () == -i->GetDelta ())
        {
          break;
        }
      ni->push_back (*i);
    }
  ni->insert (ni->begin (), NiChange (event->GetStartTime (), noiseInterference));
  ni->push_back (NiChange (event->GetEndTime (), 0));
  return noiseInterference;
}

bool
CsmacaInterferenceHelper::CheckChunkShannonCapacity (double snir, Time duration, CsmacaPreamble preamble) const
{
  if (duration == NanoSeconds (0))
    {
      return true;
    }

  uint32_t rate = preamble.GetRate ();
  uint64_t nbits = (uint64_t)(rate * duration.GetSeconds ());
  uint64_t shannonBits = preamble.GetBandwidth () * log2 (1 + snir);
  shannonBits /= 8;
  shannonBits = shannonBits * duration.GetSeconds ();
  //  std::cout << "[Slimit]: " << shannonBits << ", [Bit]:" << nbits << ", [SNIR]:" << snir << " , [D]:" << duration << std::endl;
  NS_LOG_DEBUG ("[Slimit]: " << shannonBits << ", [Bit]:" << nbits << ", [SNIR]:" << snir << " , [D]:" << duration);
    
  if (shannonBits >= nbits)
    {
      return true;
    }
  else
    {
      return false;
    }
}

double
CsmacaInterferenceHelper::CalculatePer (Ptr<const CsmacaInterferenceHelper::Event> event, NiChanges *ni) const
{
  CsmacaPreamble preambleHdr;
  double snr;

  NiChanges::iterator j = ni->begin ();
  Time previous = (*j).GetTime ();

  Time preambleStart = (*j).GetTime ();
  Time payloadStart  = (*j).GetTime () + event->GetPreamble ().GetDuration ();
  double noiseInterferenceW = (*j).GetDelta ();
  double powerW = event->GetRxPowerW ();

  j++;

  while (ni->end () != j)
    {
      Time current = (*j).GetTime ();
      if (payloadStart > previous && payloadStart < current)
        {
          
          // Header
          snr = CalculateSnr (powerW, noiseInterferenceW, preambleHdr);
          if (!CheckChunkShannonCapacity (snr, payloadStart - previous, preambleHdr))
            {
              return 1;
            }
                                        
          // Payload
          snr = CalculateSnr (powerW, noiseInterferenceW, event->GetPreamble ());
          if (!CheckChunkShannonCapacity (snr, current - payloadStart, event->GetPreamble ()))
            {
              return 1;
            }
        }
      else if (payloadStart >= current)
        {
          // Header
          snr = CalculateSnr (powerW, noiseInterferenceW, preambleHdr);
          if (!CheckChunkShannonCapacity (snr , current - previous, preambleHdr))
            {
              return 1;
            }
        }
      else if (payloadStart < current)
        {
          // Payload
          snr = CalculateSnr (powerW, noiseInterferenceW, event->GetPreamble ());
          if (!CheckChunkShannonCapacity (snr, current - previous, event->GetPreamble ()))
            {
              return 1;
            }
        }
      noiseInterferenceW += (*j).GetDelta ();
      previous = (*j).GetTime ();
      j++;
    }

  return 0;
}


struct CsmacaInterferenceHelper::SnrPer
CsmacaInterferenceHelper::CalculateSnrPer (Ptr<CsmacaInterferenceHelper::Event> event)
{
  NiChanges ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni);
  double snr = CalculateSnr (event->GetRxPowerW (),
                             noiseInterferenceW,
                             event->GetPreamble ());

  /* calculate the SNIR at the start of the packet and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculatePer (event, &ni);

  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  return snrPer;
}

void
CsmacaInterferenceHelper::EraseEvents (void)
{
  m_niChanges.clear ();
  m_rxing = false;
  m_firstPower = 0.0;
}
CsmacaInterferenceHelper::NiChanges::iterator
CsmacaInterferenceHelper::GetPosition (Time moment)
{
  return std::upper_bound (m_niChanges.begin (), m_niChanges.end (), NiChange (moment, 0));
}
void
CsmacaInterferenceHelper::AddNiChangeEvent (NiChange change)
{
  m_niChanges.insert (GetPosition (change.GetTime ()), change);
}
void
CsmacaInterferenceHelper::NotifyRxStart ()
{
  m_rxing = true;
}
void
CsmacaInterferenceHelper::NotifyRxEnd ()
{
  m_rxing = false;
}
} // namespace ns3
