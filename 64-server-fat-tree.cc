#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/gnuplot.h"

using namespace ns3;
using namespace std;

void printRoutingTable (Ptr<Node> node) 
{
        Ipv4StaticRoutingHelper helper;
	Ptr<Ipv4> stack = node -> GetObject<Ipv4>();
	Ptr<Ipv4StaticRouting> staticrouting = helper.GetStaticRouting(stack);
        uint32_t numroutes=staticrouting->GetNRoutes();
        Ipv4RoutingTableEntry entry;
        std::cout << "Routing table for device: " << Names::FindName(node) <<"\n";
        std::cout << "Destination\tMask\t\tGateway\t\tIface\n";
        for (uint32_t i =0 ; i<numroutes;i++) {
        entry =staticrouting->GetRoute(i);
        std::cout << entry.GetDestNetwork()  << "\t" << entry.GetDestNetworkMask() << "\t" << entry.GetGateway() << "\t\t" << entry.GetInterface() << "\n";
	}
	  return;
} 

void print_stats ( FlowMonitor::FlowStats st)
{
	cout << " Tx Bytes : " << st.txBytes << endl;
        cout << " Rx Bytes : " << st.rxBytes << endl;
        cout << " Tx Packets : " << st.txPackets << endl;
        cout << " Rx Packets : " << st.rxPackets << endl;
        cout << " Lost Packets : " << st.lostPackets << endl;

        if( st.rxPackets > 0) {
            cout << " Mean{Delay}: " << (st.delaySum.GetSeconds() / st.rxPackets);
            cout << " Mean{jitter}: " << (st.jitterSum.GetSeconds() / (st.rxPackets -1));
            cout << " Mean{Hop Count }: " << st.timesForwarded / st.rxPackets + 1;
        }
        if (true) {
           cout << "Delay Histogram : " << endl;
           for(uint32_t i = 0; i < st.delayHistogram.GetNBins (); i++)
              cout << " " << i << "(" << st.delayHistogram.GetBinStart (i) << st.delayHistogram.GetBinEnd(i) << "):" << st.delayHistogram.GetBinCount (i) << endl;

        }

}

NS_LOG_COMPONENT_DEFINE ("SimpleGlobalRoutingExample");

int 
main(int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below line suggets how to do this
#if 0
  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif
  // Set up some default values for the simulation. Use the
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue(210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

  // DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);
  // Allow the user to override any of the default and the above
  // DafaultValue::Bind ()s at run-time, via command-line arguments
  int k=4,i,j,temp,l1_index,p;
  CommandLine cmd;
  bool enableMonitor = true;
  std::string animFile = "fat-animation.xml";;
  cmd.AddValue("EnableMonitor", "Enable Flow Monitor", enableMonitor);
  cmd.AddValue("animFile", "Filename for animation output", animFile);
  cmd.Parse(argc, argv);
  Ipv4AddressHelper address;
  char str1[30],str2[30],str3[30];

  // For BulkSend
  uint32_t maxBytes = 52428800; // 50 MB of data


  NS_LOG_INFO ("Create Nodes");
  // Leaf Level subnets, there are k*(k/2) subnets
  NodeContainer* c = new NodeContainer[16];
  // Aggregate routers, there are k groups of routers , and in each group ther are k routers numbered from 0..k-1
  NodeContainer* l1 = new NodeContainer[4];
  // Core routrer, there are (k/2) ^ 2 in numbers.
  NodeContainer l2;
  // CsmaHelper for obtaining CSMA behavior in subnet
  CsmaHelper csma;
  // Point to Point links are used to interconnect the PODS
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  
  // NetDeviceContainer to aggregate the subnet with the aggregate router, equal to number of subnets
  NetDeviceContainer* d = new NetDeviceContainer[16]; 
  // Also have Ipv4InterfaceContainer,offcourse equal to number of NetDeviceContainer
  Ipv4InterfaceContainer* ip = new Ipv4InterfaceContainer[16];
  // Have point-to-point connection between two set of aggregate routers.
  NetDeviceContainer* p2p_d = new NetDeviceContainer[1024];
  NetDeviceContainer inter_level1;
  // Seperate Ipv4InterfaceContainer for point to point links
  Ipv4InterfaceContainer* ip_for_p2p = new Ipv4InterfaceContainer[1024]; 
  // Have k/2 aggregate router

  // Core level NetDeviceContainers for interconnecting aggregate routers with core routers
  NetDeviceContainer* core_d = new NetDeviceContainer[1024];
  // Core Ipv4InterfaceContainer
  Ipv4InterfaceContainer* ip_for_core = new Ipv4InterfaceContainer[1024];

  // Set starting time
  clock_t start = clock();
  InternetStackHelper internet;
  Ipv4NixVectorHelper nixRouting; 
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (nixRouting, 10);
  internet.SetRoutingHelper(list);

  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));

  std::pair<Ptr<Ipv4>, uint32_t> pairx; 
  // First create core router
  l2.Create(4);
  internet.Install(l2);
   int subnet_index = 0;
  l1_index = 0;
  
  // For animation
  float pos_x = 60;
  float pos_y = 20;
  float save;
  float save_x;
  // First place the core routers
  for( int x = 0; x < k ; x++){
                Ptr<Node> n1 = l2.Get(x);
                Ptr<ConstantPositionMobilityModel> loc1 = n1->GetObject<ConstantPositionMobilityModel> ();
                if(loc1 == 0) {
                        loc1 = CreateObject<ConstantPositionMobilityModel> ();
                        Vector vec1(pos_x,pos_y,0);
                        loc1->SetPosition(vec1);
                        n1->AggregateObject(loc1);
                }
                pos_x = pos_x + 30;
  }
  printf("Cores are places\n");
  // Placing core ends here
   pos_x = 10;
   pos_y = 70;

  // Constructing k pods 
  for(i=0; i<k; i++) {
        cout << " *******************Pod " << i <<" ***********************" << endl;
        l1[i].Create(k);
        save = pos_x;
        pos_y = 70;
        for( int x = k/2; x < k; x++){
                Ptr<Node> n1 = l1[i].Get(x);
                Ptr<ConstantPositionMobilityModel> loc1 = n1->GetObject<ConstantPositionMobilityModel> ();
                if(loc1 == 0) {
                        loc1 = CreateObject<ConstantPositionMobilityModel> ();
                        Vector vec1(pos_x,pos_y,0);
                        loc1->SetPosition(vec1);
                        n1->AggregateObject(loc1);
                        cout << "Pod: " << i << " Subnet: " << subnet_index << "Node:0 " << pos_x << " " << pos_y << endl;
                }
                pos_x = pos_x + 30;
        }
         save_x = save;
        pos_y = 110;
        for( int x = 0; x < k/2 ; x++){
                Ptr<Node> n1 = l1[i].Get(x);
                Ptr<ConstantPositionMobilityModel> loc1 = n1->GetObject<ConstantPositionMobilityModel> ();
                if(loc1 == 0) {
                        loc1 = CreateObject<ConstantPositionMobilityModel> ();
                        Vector vec1(save_x,pos_y,0);
                        loc1->SetPosition(vec1);
                        n1->AggregateObject(loc1);
                        cout << "Pod: " << i << " Subnet: " << subnet_index << "Node:0 " << pos_x << " " << pos_y << endl;
                }
                save_x = save_x + 30;
         }
        printf("Aggr placed\n");
	internet.Install(l1[i]);
        save_x = save;
        pos_y = 150;
        for ( j=0; j < k/2 ; j++)
	{
	   temp = (k/2) * i + j;
	   cout << "Subnet Number :" << temp << endl;
	   c[subnet_index].Create(k/2);
           // Place the nodes at suitable position for Animation
           Ptr<Node> n1 = c[subnet_index].Get(0);
           Ptr<ConstantPositionMobilityModel> loc1 = n1->GetObject<ConstantPositionMobilityModel> ();
           if(loc1 == 0) {
              loc1 = CreateObject<ConstantPositionMobilityModel> ();
              Vector vec1(save_x - 10,pos_y,0);
              loc1->SetPosition(vec1);
              n1->AggregateObject(loc1);
              cout << "n1: " <<  pos_x << " " << pos_y << endl;
           }
						save_x = save_x + 20;
            // Place the nodes at suitable position for Animation
           Ptr<Node> n2 = c[subnet_index].Get(1);
           Ptr<ConstantPositionMobilityModel> loc2 = n2->GetObject<ConstantPositionMobilityModel> ();
           if(loc2 == 0) {
              loc2 = CreateObject<ConstantPositionMobilityModel> ();
              Vector vec2(save_x + 10,pos_y ,0);
              loc2->SetPosition(vec2);
              n2->AggregateObject(loc2);
              cout << "n2: " <<  pos_x << " " << pos_y << endl;
           }
           printf("Pod placed\n");
	   internet.Install(c[subnet_index]);

	   // Connect jth subnet with jth router of the POD, there are k/2 PODS.
	   d[temp] = csma.Install (NodeContainer (l1[i].Get(j), c[subnet_index]));
           bzero(str1,30);
           bzero(str3,30);
           bzero(str2,30);
           strcpy(str1,"10.");
           snprintf(str2,10,"%d",i);
           strcat(str2,".");
           snprintf(str3,10,"%d",j);
	   strcat(str3,".0");
           strcat(str2,str3);
           strcat(str1,str2);
           cout << "Subnet Address : " << str1 << endl;
           address.SetBase (str1, "255.255.255.0");
           ip[temp] = address.Assign(d[temp]);
	   cout << "Temp : " << temp << "Subnet_index : " << subnet_index << endl;
	   subnet_index++;
        }
	//l1_index =(i * k/2 );  
	// Interconnect two rows of the aggregate routers.
	// variale to hold the second position
	int sec_ip = k/2;
	int third_ip = 0;
	for(j=k/2; j < k; j++)
	{
	      cout << " Combined  Pod " << i << " th " << j << "Router\n"; 
	      for ( p = 0; p < k/2; p++)
	      {  
                 p2p_d[l1_index] = p2p.Install(NodeContainer(l1[i].Get(j),l1[i].Get(p)));
		 //p2p_d[l1_index].Add (csma.Install(NodeContainer ( l1[i].Get (p)))); 
	         cout << " Router added are  Pod " << i << " th " << p << "Router\n"; 
	         bzero(str1,30);
                 bzero(str3,30);
                 bzero(str2,30);
                 strcpy(str1,"10.");
                 snprintf(str2,10,"%d",i);
                 strcat(str2,".");
		 strcat(str1,str2);
                 snprintf(str3,10,"%d",sec_ip);
		 strcat(str1,str3);
		 strcat(str1,".");
		 bzero(str3,30);
		 snprintf(str3,10,"%d",third_ip);
                 strcat(str1,str3);
                 cout << "Level: " << i  << " Router: " << j <<" -- IP Address : " << str1 << endl;
                 address.SetBase (str1, "255.255.255.252");
	         ip_for_p2p[l1_index] = address.Assign ( p2p_d[l1_index]);
                 pairx = ip_for_p2p[l1_index].Get (0);
	         cout << "++++++++++++++l1_index +++++++++++++++++ :" << l1_index << endl;
                 cout << "Router addresses 1 :" << pairx.first -> GetAddress(1,0) << endl; 
                 //cout << "Router addresses 2 :" << pairx.first -> GetAddress(2,0) << endl; 
                 //cout << "Router addresses 2 :" << pairx.first -> GetAddress(3,0) << endl; 
	         l1_index++;
		 third_ip = third_ip + 4;
		 if( third_ip == 252){
		      third_ip = 0;
		      sec_ip++;
		 }     
	      }
	}
}

// There are (k/2) ^ 2 core routers i.e for k = 8 , 16 core switches
int l2_no = pow ( (k/2), 2);
// These (k/2) ^ 2 core switches are partitioned into sets each containing (k/2) switches
int groups = l2_no / (k/2);
int l2_index = 0;
// Have ip1 as counter for third octet
int ip1 = 1;
// Have ip2 as counter for fourth octet 
int ip2 = 0;
for( i = 0 ; i < groups ; i++)
{
    for ( j = 0; j < k/2 ; j++)
    {
    	for ( p = 0; p < k ; p++)
    	{
    	         core_d[l2_index] = p2p.Install(NodeContainer(l2.Get((k/2)*i+j),l1[p].Get((k/2) + j)));
	         cout << "Core****" << k/2*i+j << "****Combined with \n"; 
        	 //core_d[l2_index].Add (p2p.Install(NodeContainer ( l1[p].Get ((k/2) + j))));
		 cout << "Pod " << p << " Switch " << (k/2) + j << endl;
    	         bzero(str1,30);
   	         bzero(str3,30);
   	         bzero(str2,30);
    	         strcpy(str1,"10.");
    	         snprintf(str2,10,"%d",k);
		 strcat(str1,str2);
    	         strcat(str1,".");
		 bzero(str2,30);
		 snprintf(str2,10,"%d",ip1);
		 strcat(str1,str2);
		 strcat(str1,".");
                 bzero(str2,30);
		 snprintf(str2,10,"%d",ip2);
		 strcat(str1,str2);
    	         cout << "Group:  " << i  << " Router: " << j <<" -- IP Address : " << str1 << endl;
    	         address.SetBase (str1, "255.255.255.252");
    	         ip_for_core[l2_index] = address.Assign ( core_d[l2_index]);
	         uint32_t nNodes = ip_for_core[l2_index].GetN ();
	         cout << "Number of Nodes at core :" << l2_index << "   is-_-_-_-_ " << nNodes << endl; 
    	         l2_index++;
		 ip2 = ip2 + 4;
		 if(ip2 == 252) {
		    ip2 = 0;
		    ip1++;
		 }   
    	}
    }
}   
  
  std::pair<Ptr<Ipv4>, uint32_t> pair0 = ip_for_p2p[0].Get (0);
  cout << "Router0 addresses 1 :" << pair0.first -> GetAddress(1,0) << endl; 
  cout << "Router0 addresses 2 :" << pair0.first -> GetAddress(2,0) << endl; 
  //cout << "Router0 addresses 3 :" << pair0.first -> GetAddress(3,0) << endl; 

  std::pair<Ptr<Ipv4>, uint32_t> pair1 = ip_for_p2p[0].Get (1);
  cout << "Router1 addresses 1 :" << pair1.first -> GetAddress(1,0) << endl; 
  cout << "Router1 addresses 2 :" << pair1.first -> GetAddress(2,0) << endl; 
  cout << "Router1 addresses 3 :" << pair1.first -> GetAddress(3,0) << endl; 

  std::pair<Ptr<Ipv4>, uint32_t> pair2 = ip_for_p2p[1].Get (1);
  cout << "Router2 addresses 1 :" << pair2.first -> GetAddress(1,0) << endl; 
  cout << "Router2 addresses 2 :" << pair2.first -> GetAddress(2,0) << endl; 
  cout << "Router2 addresses 3 :" << pair2.first -> GetAddress(3,0) << endl;

  std::pair<Ptr<Ipv4>, uint32_t> pair3 = ip_for_p2p[1].Get (0);
  cout << "Router3 addresses 1 :" << pair3.first -> GetAddress(1,0) << endl; 
  //cout << "Router3 addresses 2 :" << pair3.first -> GetAddress(2,0) << endl; 
  //cout << "Router3 addresses 3 :" << pair3.first -> GetAddress(3,0) << endl; 
  //cout << "Router2 addresses 4 :" << pair2.first -> GetAddress(4,0) << endl; 
  std::pair<Ptr<Ipv4>, uint32_t> pair4 = ip_for_core[3].Get (0);
  cout << "Core addresses 1 :" << pair4.first -> GetAddress(1,0) << endl; 
  std::pair<Ptr<Ipv4>, uint32_t> pair5 = ip_for_core[3].Get (1);
  cout << "Core addresses 1 :" << pair5.first -> GetAddress(1,0) << endl; 
  std::pair<Ptr<Ipv4>, uint32_t> pair6 = ip_for_core[3].Get (1);
  cout << "Core addresses 1 :" << pair6.first -> GetAddress(1,0) << endl; 
//  cout << "Core addresses 2 :" << pair3.first -> GetAddress(2,0) << endl; 
 // cout << "Core addresses 3 :" << pair3.first -> GetAddress(3,0) << endl; 
  //cout << "Core addresses 4 :" << pair3.first -> GetAddress(4,0) << endl; 
  //cout << "Core addresses 5 :" << pair3.first -> GetAddress(5,0) << endl; 





  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
   //Print total time taken
	
  cout << "Total time for setting up a route : " << (double) ( clock() - start) / CLOCKS_PER_SEC;
  cout << "Aggregate routers, each pod second level, 3rd router" << endl;
  printRoutingTable(l1[0].Get(2));
  printRoutingTable(l1[1].Get(2));
  printRoutingTable(l1[2].Get(2));
  printRoutingTable(l1[3].Get(2));
  cout << "Aggregate routers, each pod first level, 3rd router" << endl;
  printRoutingTable(l1[0].Get(0));
  printRoutingTable(l1[1].Get(0));
  printRoutingTable(l1[2].Get(0));
  printRoutingTable(l1[3].Get(0));
  cout << "Core routers routing table " << endl;
  printRoutingTable(l2.Get(0));
  printRoutingTable(l2.Get(1));
  printRoutingTable(l2.Get(2));
  printRoutingTable(l2.Get(3));

   //
  // Create one udpServer applications on node one.
  //
   int temp1;
   uint16_t port = 9;
   ApplicationContainer sourceApps;
   ApplicationContainer sinkApps;

   BulkSendHelper source ("ns3::TcpSocketFactory",Address(InetSocketAddress (ip[0].GetAddress (1), port)));
   source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
   PacketSinkHelper sink ("ns3::TcpSocketFactory",Address(InetSocketAddress (Ipv4Address::GetAny (), port)));

   for( int i = 0; i < k ; i++)
   {
       temp = i * k/2;
       for ( int j= 0; j < k; j++){
           if( i != j) {
              temp1 = j * k/2;
              for( int l = 0; l < k/2 ; l++){
                   printf( " Pod %d server %d sedning data to Pod %d server %d \n", temp+l,l,temp1+l,l);
                   source.SetAttribute ("Remote", AddressValue (InetSocketAddress (ip[temp1+l].GetAddress (l+1), port)));
                   sourceApps = source.Install (c[temp + l].Get (l));

                   sourceApps.Start (Seconds (0.5));
                   sourceApps.Stop (Seconds (10.0));

                   sink.SetAttribute ("Local",AddressValue(InetSocketAddress(Ipv4Address::GetAny (), port)));
                   sinkApps = sink.Install (c[temp1+l].Get (l));

                   sinkApps.Start (Seconds (0.5));
                   sinkApps.Stop (Seconds (10.0));
                   port++;
               }
           }
        }
   }
    csma.EnablePcap ("s0", d[0].Get(1), false);
    csma.EnablePcap ("s1", d[1].Get(1), false);
    csma.EnablePcap ("s2", d[2].Get(1), false);
    csma.EnablePcap ("s3", d[3].Get(1), false);

    csma.EnablePcap ("c0", d[0].Get(2), false);
    csma.EnablePcap ("c1", d[1].Get(2), false);
    csma.EnablePcap ("c2", d[2].Get(2), false);
    csma.EnablePcap ("c3", d[3].Get(2), false);
  
  // Code for flowmonitor
     Ptr<FlowMonitor> flowmon;

       FlowMonitorHelper flowmonHelper;
       flowmonHelper.SetMonitorAttribute("DelayBinWidth",ns3::DoubleValue(10));
       flowmonHelper.SetMonitorAttribute("JitterBinWidth",ns3::DoubleValue(10));

       flowmon = flowmonHelper.InstallAll();
       Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier());

  // End of flow monitor

  //
  // Now, do the actual simulation.
  //
     NS_LOG_INFO ("Run Simulation.");
     Simulator::Stop (Seconds (20.0));

		 AnimationInterface anim(animFile);
	
     Simulator::Run ();
     
     if(enableMonitor){
        printf("Entering the monitor\n");
        bzero(str1,30);
        bzero(str2,30);
        bzero(str3,30);
        strcpy(str1,"fat-tree");
        snprintf(str2,10,"%d",10);
        strcat(str2,".flowmon");
        strcat(str1,str2);
        flowmon->SerializeToXmlFile(str1, false, false);

     }
     bool Plot=true;
     if(Plot)
         {
               Gnuplot gnuplot("DELAYSbyFLOW.png");
                 Gnuplot2dDataset dataset;
                 dataset.SetStyle(Gnuplot2dDataset::HISTEPS);
                 std::map< FlowId, FlowMonitor::FlowStats> stats = flowmon->GetFlowStats ();
                 for (std::map<FlowId, FlowMonitor::FlowStats>::iterator flow=stats.begin(); flow!=stats.end();flow++)

                 {
                         Ipv4FlowClassifier::FiveTuple tuple = classifier->FindFlow(flow->first);

                         if(tuple.protocol == 17 && tuple.sourcePort == 698)
                                 continue;
                         dataset.Add((double)flow->first, (double)flow->second.delaySum.GetSeconds() / (double)flow->second.rxPackets);

                 }
                 gnuplot.AddDataset(dataset);
                 gnuplot.GenerateOutput(std::cout);
     }

     Simulator::Destroy ();
     Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get(0));
     cout << "Total Bytes received: " << sink1->GetTotalRx () << endl;
		 anim.StopAnimation ();
     NS_LOG_INFO ("Done.");
  //

}
