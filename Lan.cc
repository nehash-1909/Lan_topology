#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LanScriptExample");

int 
main (int argc, char *argv[])
{

  uint32_t nCsma = 3; // 3 nodes in pure csma connection

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);


  cmd.Parse (argc,argv);


  NodeContainer p2pNodes; //creating 2 nodes
  p2pNodes.Create (2);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1)); //creating node container having 3 pure csma nodes and 1  additional node
  csmaNodes.Create (nCsma);

  PointToPointHelper pointToPoint; //creating point to point interface
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices; //installing links on interface
  p2pDevices = pointToPoint.Install (p2pNodes);

  CsmaHelper csma; //creating csma connection
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices; // creating csma links
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack; //installing stack on every node
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;  
  // setting ip address for point to point network
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

// setting ip address for point to point network
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  UdpEchoServerHelper echoServer (9); //setting port number of server as 9

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma)); //installing server on last node of csma link i.e n4
  serverApps.Start (Seconds (1.0)); //starting server connection
  serverApps.Stop (Seconds (10.0)); //stopping server connection

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9); //providing socket for client
  //setting client attributes
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1)); 
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0)); //setting client on first node of p2p i.e n0
  clientApps.Start (Seconds (2.0));  //starting client connection
  clientApps.Stop (Seconds (10.0));  //stopping client connection

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll ("lan"); //enabling point to point nodes pcap
  csma.EnablePcap ("lan", csmaDevices.Get (1), true); //enabling csma nodes pcap

 // Using NetAnim
	AnimationInterface::SetConstantPosition (p2pNodes.Get (0), 10.0, 20.0);
	AnimationInterface::SetConstantPosition (p2pNodes.Get (1), 20.0, 20.0);
        AnimationInterface::SetConstantPosition (csmaNodes.Get (1), 30.0, 20.0);
	AnimationInterface::SetConstantPosition (csmaNodes.Get (2), 40.0, 20.0);
	AnimationInterface::SetConstantPosition (csmaNodes.Get (3), 50.0, 20.0);
	AnimationInterface anim ("lan.xml"); 


  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
