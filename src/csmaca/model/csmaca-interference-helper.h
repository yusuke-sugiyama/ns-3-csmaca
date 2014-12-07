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
#ifndef SIMPLE_INTERFERENCE_HELPER_H
#define SIMPLE_INTERFERENCE_HELPER_H

#include <stdint.h>
#include <vector>
#include <list>
#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"
#include "csmaca-preamble.h"

namespace ns3 {

/**
 * \ingroup wifi
 * \brief handles interference calculations
 */
class CsmacaInterferenceHelper
{
public:
  /**
   * Signal event for a packet.
   */
  class Event : public SimpleRefCount<CsmacaInterferenceHelper::Event>
  {
public:

    Event (uint32_t size, Time duration, double rxPower, CsmacaPreamble preamble);
    ~Event ();

    Time GetDuration (void) const;
    Time GetStartTime (void) const;
    Time GetEndTime (void) const;
    double GetRxPowerW (void) const;
    uint32_t GetSize (void) const;
    CsmacaPreamble GetPreamble (void) const;

private:
    uint32_t m_size;
    Time m_startTime;
    Time m_endTime;
    double m_rxPowerW;
    CsmacaPreamble m_preamble;
  };

  struct SnrPer
  {
    double snr;
    double per;
  };

  CsmacaInterferenceHelper ();
  ~CsmacaInterferenceHelper ();

  void SetNoiseFigure (double value);
  double GetNoiseFigure (void) const;

  Time GetEnergyDuration (double energyW);

  Ptr<CsmacaInterferenceHelper::Event> Add (uint32_t size, Time duration, double rxPower, CsmacaPreamble preamble);

  struct CsmacaInterferenceHelper::SnrPer CalculateSnrPer (Ptr<CsmacaInterferenceHelper::Event> event);

  void NotifyRxStart ();
  void NotifyRxEnd ();
  void EraseEvents (void);

private:

  class NiChange
  {
public:

    NiChange (Time time, double delta);

    Time GetTime (void) const;
    double GetDelta (void) const;
    bool operator < (const NiChange& o) const;

private:
    Time m_time;
    double m_delta;
  };

  typedef std::vector <NiChange> NiChanges;
  typedef std::list<Ptr<Event> > Events;

  void AppendEvent (Ptr<Event> event);

  double CalculateNoiseInterferenceW (Ptr<Event> event, NiChanges *ni) const;
  double CalculateSnr (double signal, double noiseInterference, CsmacaPreamble preamble) const;
  bool CheckChunkShannonCapacity (double snir, Time duration, CsmacaPreamble preamble) const;
  double CalculatePer (Ptr<const Event> event, NiChanges *ni) const;

  double m_noiseFigure; /**< noise figure (linear) */
  /// Experimental: needed for energy duration calculation
  NiChanges m_niChanges;
  double m_firstPower;
  bool m_rxing;
  /// Returns an iterator to the first nichange, which is later than moment
  NiChanges::iterator GetPosition (Time moment);
  /**
   * Add NiChange to the list at the appropriate position.
   *
   * \param change
   */
  void AddNiChangeEvent (NiChange change);
};

} // namespace ns3

#endif /* SIMPLE_IINTERFERENCE_HELPER_H */
