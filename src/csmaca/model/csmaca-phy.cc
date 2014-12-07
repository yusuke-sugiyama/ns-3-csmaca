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

#include "ns3/core-module.h"
#include "ns3/simulator.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"

#include "csmaca-phy.h"
#include "csmaca-preamble.h"

NS_LOG_COMPONENT_DEFINE ("CsmacaPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmacaPhy);

CsmacaPhy::CsmacaPhy ()
  : m_edThresholdW (DbmToW (-96.0)),
    m_ccaMode1ThresholdW (DbmToW (-99.0)),
    m_txGainDb (0),
    m_rxGainDb (0),
    m_txPowerDbm (20),
    m_endRxEvent ()
{
  NS_LOG_FUNCTION (this);
  m_channel = CreateObject<CsmacaChannel>();
  m_state = CreateObject<CsmacaPhyStateHelper>();
  m_random = CreateObject<UniformRandomVariable> ();

  m_rxNoiseFigureDb = 7;
  m_interference.SetNoiseFigure (DbToRatio (m_rxNoiseFigureDb));

}
CsmacaPhy::~CsmacaPhy ()
{
  NS_LOG_FUNCTION (this);
}
void
CsmacaPhy::DoDispose (){
  m_channel = 0;
  m_state = 0;
}

TypeId
CsmacaPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmacaPhy")
    .SetParent<Object> ()
    .AddTraceSource ("StartTx", "Start transmission",
                     MakeTraceSourceAccessor (&CsmacaPhy::m_txTrace))
    ;
  return tid;
}
void
CsmacaPhy::SetMobility (Ptr<Object> mobility)
{
  m_mobility = mobility;
}

void
CsmacaPhy::SetDevice (Ptr<Object> device)
{
  m_device = device;
}

Ptr<Object>
CsmacaPhy::GetMobility ()
{
  return m_mobility;
}

Ptr<CsmacaPhyStateHelper>
CsmacaPhy::GetPhyStateHelper () const
{
  return m_state;
}

Ptr<CsmacaChannel>
CsmacaPhy::GetChannel () const
{
  return m_channel;
}

Ptr<Object>
CsmacaPhy::GetDevice (void) const
{
  return m_device;
}

int64_t
CsmacaPhy::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}

void
CsmacaPhy::StartSend (Ptr<Packet> packet, CsmacaPreamble preamble)
{
  NS_LOG_FUNCTION (this);
  if (m_state->IsStateRx ())
    {
      m_endRxEvent.Cancel ();
      m_interference.NotifyRxEnd ();
    }
  m_txTrace (packet);
  Time txDuration = Seconds((double)packet->GetSize () / preamble.GetRate ()) + preamble.GetDuration ();
  m_state->SwitchToTx (txDuration);
  m_channel->Send (packet, preamble, m_txPowerDbm + m_txGainDb, this);
}

void
CsmacaPhy::StartReceive (Ptr<Packet> packet, CsmacaPreamble preamble, double rxPowerDbm)
{
  NS_LOG_FUNCTION (this << rxPowerDbm + m_rxGainDb);
  double rxPowerW = DbmToW (rxPowerDbm + m_rxGainDb);
  Time rxDuration = Seconds((double)packet->GetSize () / preamble.GetRate ()) + preamble.GetDuration ();
  Ptr<CsmacaInterferenceHelper::Event> event;
  event = m_interference.Add (packet->GetSize (), rxDuration, rxPowerW, preamble);
  switch (m_state->GetState ())
    {
    case CsmacaPhyState::RX:
      NS_LOG_DEBUG ("Can not receive because state is RX");
      goto maybeCcaBusy;
      break;
    case CsmacaPhyState::TX:
      NS_LOG_DEBUG ("Can not receive because state is TX");
      goto maybeCcaBusy;
    break;
    case CsmacaPhyState::CCA_BUSY:
    case CsmacaPhyState::IDLE:
      if (rxPowerW > m_edThresholdW)
	{
	  m_interference.NotifyRxStart ();
	  m_state->SwitchToRx (rxDuration);
	  m_endRxEvent = Simulator::Schedule (rxDuration,
					      &CsmacaPhy::EndReceive,
					      this,
					      packet,
					      event);
	}
      else
	{
	  NS_LOG_DEBUG ("Can not receive because rxPower is small: " << rxPowerDbm + m_rxGainDb);
	  goto maybeCcaBusy;
	}
      break;
    }
  return;

maybeCcaBusy:
  Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaMode1ThresholdW);
  if (!delayUntilCcaEnd.IsZero ())
    {
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
    }
}

void
CsmacaPhy::EndReceive (Ptr<Packet> packet,  Ptr<CsmacaInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << packet);
  NS_ASSERT (event->GetEndTime () == Simulator::Now ());

  struct CsmacaInterferenceHelper::SnrPer snrPer;
  snrPer = m_interference.CalculateSnrPer (event);
  m_interference.NotifyRxEnd ();

  NS_LOG_DEBUG ("rate=" << (event->GetPreamble ().GetRate ()) <<
                ", snr=" << snrPer.snr << ", per=" << snrPer.per << ", size=" << packet->GetSize ());

  if (m_random->GetValue () > snrPer.per)
    {
      m_state->EndReceiveOk (packet);
    }
  else
    {
      m_state->EndReceiveError (packet);
    }
}

double
CsmacaPhy::DbToRatio (double dB) const
{
  double ratio = std::pow (10.0, dB / 10.0);
  return ratio;
}

double
CsmacaPhy::DbmToW (double dBm) const
{
  double mW = std::pow (10.0, dBm / 10.0);
  return mW / 1000.0;
}

double
CsmacaPhy::RatioToDb (double ratio) const
{
  return 10.0 * std::log10 (ratio);
}
} // namespace ns3
