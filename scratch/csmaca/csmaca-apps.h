#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/application.h"

#include "ns3/stats-module.h"

using namespace ns3;

//----------------------------------------------------------------------
//------------------------------------------------------
class CsmacaSender : public Application {
 public:
  static TypeId GetTypeId (void);
  CsmacaSender();
  virtual ~CsmacaSender();
  
 protected:
  virtual void DoDispose (void);
  
 private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  
  void SendPacket ();
  
  uint32_t        m_pktSize;
  uint32_t        m_numSendPkts;
  Ipv4Address     m_destAddr;
  uint32_t        m_destPort;
  Ptr<UniformRandomVariable> m_random;
  Ptr<ConstantRandomVariable> m_interval;
  
  Ptr<Socket>     m_socket;
  EventId         m_sendEvent;
  
  TracedCallback<Ptr<const Packet> > m_txTrace;
  
  // end class CsmacaSender
};




//------------------------------------------------------
class CsmacaReceiver: public Application {
 public:
  static TypeId GetTypeId (void);
  CsmacaReceiver();
  virtual ~CsmacaReceiver();
  
  void SetCounter (Ptr<CounterCalculator<> > calc);
  
 protected:
  virtual void DoDispose (void);
  
 private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  
  void Receive (Ptr<Socket> socket);

  uint32_t        m_numPkts;  
  Ptr<Socket>     m_socket;
  uint32_t        m_port;
  
  TracedCallback<uint32_t, Ptr<Packet> > m_rxTrace;
  // end class CsmacaReceiver
};
