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

#include "csmaca-phy-state-helper.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("CsmacaPhyStateHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmacaPhyStateHelper);

CsmacaPhyListener::~CsmacaPhyListener ()
{
}

CsmacaPhyStateHelper::CsmacaPhyStateHelper ()
  : m_startTx (0),
    m_startRx (0),
    m_startCcaBusy (0),
    m_endTx (0),
    m_endRx (0),
    m_endCcaBusy (0),
    m_listeners (0),
    m_rxing (false)
{
  NS_LOG_FUNCTION (this);
}

void
CsmacaPhyStateHelper::NotifyMaybeCcaBusyStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyMaybeCcaBusyStart (duration);
    }
}
void
CsmacaPhyStateHelper::NotifyTxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyTxStart (duration);
    }
}
void
CsmacaPhyStateHelper::NotifyRxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxStart (duration);
    }
}
bool
CsmacaPhyStateHelper::IsStateIdle (void)
{
  return (GetState () == CsmacaPhyState::IDLE);
}
bool
CsmacaPhyStateHelper::IsStateBusy (void)
{
  return (GetState () != CsmacaPhyState::IDLE);
}
bool
CsmacaPhyStateHelper::IsStateCcaBusy (void)
{
  return (GetState () == CsmacaPhyState::CCA_BUSY);
}
bool
CsmacaPhyStateHelper::IsStateRx (void)
{
  return (GetState () == CsmacaPhyState::RX);
}
bool
CsmacaPhyStateHelper::IsStateTx (void)
{
  return (GetState () == CsmacaPhyState::TX);
}

enum CsmacaPhyState::State
CsmacaPhyStateHelper::GetState (void)
{
  if (m_endTx > Simulator::Now ())
    {
      return CsmacaPhyState::TX;
    }
  else if (m_rxing)
    {
      return CsmacaPhyState::RX;
    }
  else if (m_endCcaBusy > Simulator::Now ())
    {
      return CsmacaPhyState::CCA_BUSY;
    }
  else
    {
      return CsmacaPhyState::IDLE;
    }
}

void
CsmacaPhyStateHelper::EndReceiveOk (Ptr<Packet> packet)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndOk (packet);
    }

  m_rxing = false;
}

void
CsmacaPhyStateHelper::EndReceiveError (Ptr<Packet> packet)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndError (packet);
    }

  m_rxing = false;
}

void
CsmacaPhyStateHelper::RegisterListener (CsmacaPhyListener *listener)
{
  m_listeners.push_back (listener);
}

void
CsmacaPhyStateHelper::SwitchMaybeToCcaBusy (Time duration)
{
  NS_LOG_FUNCTION (this << duration << duration + Simulator::Now ());
  NotifyMaybeCcaBusyStart (duration);
  Time now = Simulator::Now ();

  if (GetState () != CsmacaPhyState::CCA_BUSY)
    {
      m_startCcaBusy = now;
    }
  m_endCcaBusy = std::max (m_endCcaBusy, now + duration);
}

void
CsmacaPhyStateHelper::SwitchToTx (Time duration)
{
  NS_LOG_FUNCTION (this << duration << duration + Simulator::Now ());
  NotifyTxStart (duration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case CsmacaPhyState::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_rxing = false;
      m_endRx = now;
      break;
    case CsmacaPhyState::IDLE:
    case CsmacaPhyState::TX:
    case CsmacaPhyState::CCA_BUSY:
      break;
    default:
      NS_FATAL_ERROR ("Invalid CsmacaPhyState state.");
      break;
    }
  m_endTx = now + duration;
  m_startTx = now;
}
void
CsmacaPhyStateHelper::SwitchToRx (Time duration)
{
  NS_LOG_FUNCTION (this << duration << duration + Simulator::Now ());
  NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
  NS_ASSERT (!m_rxing);
  NotifyRxStart (duration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case CsmacaPhyState::RX:
    case CsmacaPhyState::TX:
      NS_FATAL_ERROR ("Invalid CsmacaPhyState state.");
      break;
    case CsmacaPhyState::IDLE:
    case CsmacaPhyState::CCA_BUSY:
      break;
    }
  m_rxing = true;
  m_startRx = now;
  m_endRx = now + duration;
  NS_ASSERT (IsStateRx ());
}
} // namespace ns3
