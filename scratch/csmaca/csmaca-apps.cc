#include <ostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"

#include "csmaca-apps.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmacaApps");

TypeId
CsmacaSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("CsmacaSender")
    .SetParent<Application> ()
    .AddConstructor<CsmacaSender> ()
    .AddAttribute ("PacketSize", "The size of packets transmitted.",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&CsmacaSender::m_pktSize),
                   MakeUintegerChecker<uint32_t>(1))
    .AddAttribute ("Destination", "Target host address.",
                   Ipv4AddressValue ("255.255.255.255"),
                   MakeIpv4AddressAccessor (&CsmacaSender::m_destAddr),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("Port", "Destination app port.",
                   UintegerValue (1603),
                   MakeUintegerAccessor (&CsmacaSender::m_destPort),
                   MakeUintegerChecker<uint32_t>(0))
    .AddAttribute ("Interval", "Delay between transmissions.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1]"),
                   MakePointerAccessor (&CsmacaSender::m_interval),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("Stream", "Random Stream.",
                   StringValue ("ns3::UniformRandomVariable[Stream=-1]"),
                   MakePointerAccessor (&CsmacaSender::m_random),
                   MakePointerChecker <RandomVariableStream>())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&CsmacaSender::m_txTrace))
  ;
  return tid;
}


CsmacaSender::CsmacaSender()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_interval = CreateObject<ConstantRandomVariable> ();
  m_random = CreateObject<UniformRandomVariable> ();
  m_socket = 0;
}

CsmacaSender::~CsmacaSender()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
CsmacaSender::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

void CsmacaSender::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0) {
      Ptr<SocketFactory> socketFactory = GetNode ()->GetObject<SocketFactory>
          (UdpSocketFactory::GetTypeId ());
      m_socket = socketFactory->CreateSocket ();
      m_socket->Bind ();
    }
  Simulator::Cancel (m_sendEvent);
  m_sendEvent = Simulator::ScheduleNow (&CsmacaSender::SendPacket, this);

  // end CsmacaSender::StartApplication
}

void CsmacaSender::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
  // end CsmacaSender::StopApplication
}

void CsmacaSender::SendPacket ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_INFO ("Sending packet at " << Simulator::Now () << " to " <<
               m_destAddr);

  Ptr<Packet> packet = Create<Packet>(m_pktSize);
  
  // Could connect the socket since the address never changes; using SendTo
  // here simply because all of the standard apps do not.
  m_socket->SendTo (packet, 0, InetSocketAddress (m_destAddr, m_destPort));

  // Report the event to the trace.
  m_txTrace (packet);
  double interval = m_interval->GetValue ();
  double logval = -log(m_random->GetValue());
  Time nextTxTime = Seconds (interval * logval);
  NS_LOG_INFO("nextTime:" << nextTxTime << " in:" << interval << " log:" << logval);

  m_sendEvent = Simulator::Schedule (nextTxTime, &CsmacaSender::SendPacket, this);
  m_numSendPkts++;
}




//----------------------------------------------------------------------
//-- Receiver
//------------------------------------------------------
TypeId
CsmacaReceiver::GetTypeId (void)
{
  static TypeId tid = TypeId ("CsmacaReceiver")
    .SetParent<Application> ()
    .AddConstructor<CsmacaReceiver> ()
    .AddAttribute ("Port", "Listening port.",
                   UintegerValue (1603),
                   MakeUintegerAccessor (&CsmacaReceiver::m_port),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("NumPackets", "Total number of packets to recv.",                   UintegerValue (0),
                   MakeUintegerAccessor (&CsmacaReceiver::m_numPkts),
                   MakeUintegerChecker<uint32_t>(0))
    .AddTraceSource ("Rx", "CsmacaReceiver data packet",
                     MakeTraceSourceAccessor (&CsmacaReceiver::m_rxTrace))
  ;
  return tid;
}

CsmacaReceiver::CsmacaReceiver()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

CsmacaReceiver::~CsmacaReceiver()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
CsmacaReceiver::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

void
CsmacaReceiver::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0) {
      Ptr<SocketFactory> socketFactory = GetNode ()->GetObject<SocketFactory>
          (UdpSocketFactory::GetTypeId ());
      m_socket = socketFactory->CreateSocket ();
      InetSocketAddress local = 
        InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
    }

  m_socket->SetRecvCallback (MakeCallback (&CsmacaReceiver::Receive, this));
  // end CsmacaReceiver::StartApplication
}

void
CsmacaReceiver::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket != 0) {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }

  // end CsmacaReceiver::StopApplication
}

void
CsmacaReceiver::Receive (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from))) {
    if (InetSocketAddress::IsMatchingType (from)) {
      InetSocketAddress::ConvertFrom (from).GetIpv4 ();
      m_rxTrace (m_numPkts, packet);
    }
  }
  
  // end CsmacaReceiver::CsmacaReceiver
}
