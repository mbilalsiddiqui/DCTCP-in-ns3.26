#include<iostream>
#include<fstream>
#include<string>
#include<cassert>
#include<malloc.h>

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

int queueSize = 0;
bool queueState = true;
bool throughput = false;

// Converting string to Integer
int convert(char *bin)
{
   int b, k, m, n;
   int len, sum = 0;

   len = strlen(bin) - 1;
   for(k = 0; k <= len; k++)
   {
       n = (bin[k] - '0'); // char to numeric value
      if ((n > 1) || (n < 0))
      {
          puts("\n\n ERROR! BINARY has only 1 and 0!\n");
          return (0);
       }
      for(b = 1, m = len; m > k; m--)
      {
            // 1 2 4 8 16 32 64 ... place-values, reversed here
                         b *= 2;
      }
      // sum it up
      sum = sum + n * b;
   }
  return sum;
}

char* construct_address(char *ip1, char *ip2)
{
    char *str1;
    char *str2;
    int ip3;
    str1 = (char*) malloc(sizeof(char) * 30);
    str2 = (char*) malloc(sizeof(char) * 30);
    cout << ip1 << " " << ip2 << endl; 
    strcpy(str1,"0000");
    strcat(str1,ip1);
    strcat(str1,ip2);
    ip3 = convert(str1);
    bzero(str1,30);
    strcpy(str1,"10.2.");
    snprintf(str2,10,"%d",ip3);
    strcat(str1,str2);
    strcat(str1,".0");
    return(str1);
}
char* construct_address_l1(char *ip1, char *ip2, char *ip3)
{
    char *str1;
    char *str2;
    char *str3;
    int ip3_nu;
    str1 = (char*) malloc(sizeof(char) * 30);
    str2 = (char*) malloc(sizeof(char) * 30);
    str3 = (char*) malloc(sizeof(char) * 30);
    cout << ip1 << " " << ip2 << " " << ip3 << endl; 
    strcpy(str1,"0000");
    strcat(str1,ip1);
    strcat(str1,ip2);
    ip3_nu = convert(str1);
    bzero(str1,30);
    strcpy(str1,"10.2.");
    snprintf(str2,10,"%d",ip3_nu);
    strcat(str1,str2);
    bzero(str2,30);
    bzero(str3,30);
    strcat(str2,"00");
    strcat(str2,ip3);
    strcat(str2,"000");
    ip3_nu = convert(str2);
    snprintf(str3,10,"%d",ip3_nu);
    strcat(str1,".");
    strcat(str1,str3);
    return(str1);
}

char* construct_address_l2(char *ip1, char *ip2, char *ip3)
{
    char *str1;
    char *str2;
    char *str3;
    int ip3_nu;
    str1 = (char*) malloc(sizeof(char) * 30);
    str2 = (char*) malloc(sizeof(char) * 30);
    str3 = (char*) malloc(sizeof(char) * 30);
    strcpy(str1,"0000");
    strcat(str1,ip1);
    strcat(str1,ip2);
    ip3_nu = convert(str1);
    bzero(str1,30);
    strcpy(str1,"10.2.");
    snprintf(str2,10,"%d",ip3_nu);
    strcat(str1,str2);
    bzero(str2,30);
    bzero(str3,30);
    strcat(str2,"0");
    strcat(str2,ip3);
    strcat(str2,"000");
    ip3_nu = convert(str2);
    snprintf(str3,10,"%d",ip3_nu);
    strcat(str1,".");
    strcat(str1,str3);
    return(str1);
}
// Increamenting Binary
char* incr_bin(char *bin)
{
    int len,k,n;
    len = strlen(bin) - 1;
    for(k = len; k >= 0; k--)
    {
          n = (bin[k] - '0'); // char to numeric value
          if ((n > 1) || (n < 0))
          {
                  puts("\n\n ERROR! BINARY has only 1 and 0!\n");
                  return (0);
          }
          if( n == 0)
          {
               bin[k] = '1';
               break;
          }
          else
               bin[k] = '0';
    }
    return bin;
}


 
void printRoutingTable(Ptr<Node> node)
{
    Ipv4StaticRoutingHelper helper;
    Ptr<Ipv4> stack = node->GetObject<Ipv4>();
    Ptr<Ipv4StaticRouting> staticRouting = helper.GetStaticRouting(stack);
    uint32_t numroutes = staticRouting->GetNRoutes();
    Ipv4RoutingTableEntry entry;
    std::cout << "Routing table for device: " << Names::FindName(node) << "\n";
    std::cout << "Destination\t Mask \t\t Gateway \t\t Iface \n";
    for(uint32_t i=0; i < numroutes; i++)
    {
         entry = staticRouting->GetRoute(i);
	 std::cout << entry.GetDestNetwork() << "\t" << entry.GetDestNetworkMask() << "\t" << entry.GetGateway() << "\t\t" << entry.GetInterface() << "\n";
    }
    return;
}

void print_stats(FlowMonitor::FlowStats st)
{
    cout << " Tx Bytes : " << st.txBytes << endl;
    cout << " Rx Bytes : " << st.rxBytes << endl;
    cout << " Tx Packets : " << st.txPackets << endl;
    cout << " Rx Packets : " << st.rxPackets << endl;
    cout << " Lost Packets : " << st.lostPackets << endl;

    if( st.rxPackets > 0) {
         cout << " Mean{Delay}: " << (st.delaySum.GetSeconds() / st.rxPackets);
	 cout << " Mean{jitter}: " << (st.jitterSum.GetSeconds() / (st.rxPackets - 1));
	 cout << " mean{Hop Count}: " << st.timesForwarded / st.rxPackets + 1;
    }
    if(true) {
           cout << "Delay Histogram :" << endl;
	   for(uint32_t i=0; i < st.delayHistogram.GetNBins (); i++)
	        cout << " " << i << "(" << st.delayHistogram.GetBinStart(i) << st.delayHistogram.GetBinEnd(i) << ")" << st.delayHistogram.GetBinCount(i) << endl;
	}
}

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,Gnuplot2dDataset DataSet)
	{
if (throughput)
{
		std::cout<<"Flow,Throughput(Gbps)"<<std::endl;
}
    double localThrou=0;
		std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
		Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
		{
			Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
	//		std::cout<<"Flow ID			: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
	//		std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
	//		std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
  //          std::cout<<"Duration		: "<<(stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())<<std::endl;
	//		std::cout<<"Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
if (throughput)
{
			std::cout<<stats->first<<","<< stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/50<<std::endl;
} 
           localThrou=(stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
			// updata gnuplot data
            DataSet.Add((double)Simulator::Now().GetSeconds(),(double) localThrou);
	//		std::cout<<"---------------------------------------------------------------------------"<<std::endl;
		}
			Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, flowMon,DataSet);
   //if(flowToXml)
      {
	flowMon->SerializeToXmlFile ("ThroughputMonitor.xml", true, true);
      }

	}

static void
Enqueue (Ptr<const Packet> p)
{
  //NS_LOG_UNCOND ("Enqueue at " << Simulator::Now ().GetSeconds ());
  queueSize = queueSize + 1;
	std::cout<<Simulator::Now ().GetSeconds ()<<","<< queueSize<<std::endl;
}

static void
Dequeue (Ptr<const Packet> p)
{
 // NS_LOG_UNCOND ("Dequeue at " << Simulator::Now ().GetSeconds ());
  queueSize = queueSize - 1;
	std::cout<<Simulator::Now ().GetSeconds ()<<","<< queueSize<<std::endl;  
}

static void
Drop (Ptr<const Packet> p)
{
//  NS_LOG_UNCOND ("Drop at " << Simulator::Now ().GetSeconds ());
  queueSize = queueSize - 1;
	std::cout<<Simulator::Now ().GetSeconds ()<<","<< queueSize<<std::endl;  
}


NS_LOG_COMPONENT_DEFINE("first");
int main(int argc, char *argv[])
{
      Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue(1200));
      Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue("1000Mb/s"));

       int i,j,k,l,temp;
      //int k=16,i,j,temp,l1_index,p;
      CommandLine cmd;
      bool enableMonitor = true;

      cmd.AddValue("EnableMonitor", "Enable Flow Monitor", enableMonitor);

      cmd.Parse(argc, argv);
      Ipv4AddressHelper address;

      char *str1, *str2, *str3;
      str1 = (char *) malloc(sizeof(char) * 30);
      str2 = (char *) malloc(sizeof(char) * 30);
      str3 = (char *) malloc(sizeof(char) * 30);

      //For BulkSend
      uint32_t maxBytes = 0;

      NS_LOG_INFO("Create Nodes");

      // Level-0 subnets, there are k*(k/2) subnets
      NodeContainer* c = new NodeContainer[16];
      NetDeviceContainer* d = new NetDeviceContainer[16];
      Ipv4InterfaceContainer* ip= new Ipv4InterfaceContainer[16];
      // Aggregate level-lower
      NodeContainer* l1 = new NodeContainer[4];
      // Aggregate level-upper 
      NodeContainer* l2 = new NodeContainer[4];
      // CsmaHelper for obtaining CSMA behavior in subnets 
      CsmaHelper csma;
      csma.SetChannelAttribute("DataRate", (DataRateValue(1000000000)));
      csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));
      csma.SetDeviceAttribute("Mtu", UintegerValue(1400));
			csma.SetQueue("ns3::DropTailQueue",  "MaxPackets",UintegerValue(250));
      // Point to Point Helper to interconnect the subnets
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
      p2p.SetChannelAttribute("Delay", StringValue("1ms"));
			p2p.SetQueue("ns3::DropTailQueue",  "MaxPackets",UintegerValue(250));
      // NetDeviceContainer for p2p connections
      NetDeviceContainer p2p_d; 
      Ipv4InterfaceContainer p2p_ip;
      InternetStackHelper internet;

      // First create four set of Level-0 subnets
      temp = 0;
      char *ip1, *ip2, *ip3;
      ip1 = (char *) malloc(8 * sizeof(char));
      ip2 = (char *) malloc(8 * sizeof(char));
      ip3 = (char *) malloc(8 * sizeof(char));

      strcpy(ip1,"00");
      strcpy(ip2,"00");
      for(i=0;i < 4; i++){
           l1[i].Create(4);
           internet.Install(l1[i]);
           temp = i * 4;
           for(j=0;j < 4; j++) {
                  c[temp+j].Create(4);
                  internet.Install(c[temp+j]);
                  c[temp+j].Add(l1[i].Get(j));
                  //printf("c[%d] is added to l1[%d].Get(%d)\n",temp+j,i,j);
                  d[temp+j] = csma.Install(c[temp+j]);
		  bzero(str1,30);
		  str1 = construct_address(ip1,ip2);
		 // printf("Assigned address is %s\n",str1);
		  address.SetBase(str1,"255.255.255.240");
		  ip[temp+j] = address.Assign(d[temp+j]);
                  ip2 = incr_bin(ip2);  
           }
	   ip1 = incr_bin(ip1);
   } // End of creating servers at level-0 and connecting it to first layer switch
  // Now connect first layer switch with second layer 
  bzero(ip1,8);
  bzero(ip2,8);
  bzero(ip3,8);
  strcpy(ip1,"00");
  strcpy(ip2,"00");
  strcpy(ip3,"010");

  for(i=0;i<4;i++){
	l2[i].Create(4);
        internet.Install(l2[i]);
        for(j=0;j<4;j++)
        {
               for(k=0;k < 4; k++){
                    p2p_d = p2p.Install(l1[i].Get(j), l2[i].Get(k));
		    bzero(str1,30);
		    str1 = construct_address_l1(ip1,ip2,ip3);
		    address.SetBase(str1,"255.255.255.248");
		    p2p_ip = address.Assign(p2p_d);
       //             cout << "--" << i << " of " << j  << " to -- " << i << "of " << k << endl;
		   // cout << "Assigned Address is " << str1 << endl;
		    ip3 = incr_bin(ip3);
               }
               bzero(ip3,8); 
               strcpy(ip3,"010");
	       ip2 = incr_bin(ip2);
        } 
	ip1 = incr_bin(ip1);
 } // End of creating first layer
 // Interconnect the second layer of four level-0 subnets
  bzero(ip1,8);
  bzero(ip2,8);
  bzero(ip3,8);
  strcpy(ip1,"00");
  strcpy(ip2,"00");
  strcpy(ip3,"0110");
 for(i=0;i<4;i++){
 for(j=i;j<4;j++){
 for(k=0;k<4;k++){
                   p2p_d = p2p.Install(l2[i].Get(k), l2[j].Get(k));
		   bzero(str1,30); 
		   str1 = construct_address_l2(ip1,ip2,ip3);
		   address.SetBase(str1,"255.255.255.248");
		   p2p_ip = address.Assign(p2p_d);
		  // cout << " -- " << i << " of " << k << " -- " << j << " of " << k << endl;
		  // cout << "Assigned address is " << str1 << endl;
		   ip3 = incr_bin(ip3);
         }
         bzero(ip3,8); 
         strcpy(ip3,"0110");
	 ip2 = incr_bin(ip2);
     }
     ip1 = incr_bin(ip1);   
}     // End of interconnecting that form mesh among four level-0 subnets
      
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

/****************** Data Transmission and receiving starts here *********************/
   
    int temp1;
    uint16_t port = 9;

		l = 1;

		temp1 = l;
		l = temp1;

		//Create a long TCP flow receiver at node 3
		PacketSinkHelper sink("ns3::TcpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
		ApplicationContainer sinkApps = sink.Install(c[3].Get(1));

		sinkApps.Start(Seconds(0.5));
		sinkApps.Stop(Seconds(20.0));

		//Create long TCP flow generators at nodes 1 and 2
		for(int sender = 1; sender < 3; sender++)
		{
			BulkSendHelper source("ns3::TcpSocketFactory",Address(InetSocketAddress(ip[3].GetAddress(1), port)));
            source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
			ApplicationContainer sourceApps = source.Install(c[sender]);

			sourceApps.Start(Seconds(0.5));
			sourceApps.Stop(Seconds(20.0));
		}
    	for(int sender = 4; sender <14; sender++)
		{
	
  			NS_LOG_INFO("Create Server Application.");
  			port = 7; // well-known echo port number.
  			TcpEchoServerHelper echoServer (port);
  			ApplicationContainer serverApps = echoServer.Install (c[sender].Get(1));
  			serverApps.Start (Seconds (0.5));
  			serverApps.Stop (Seconds (20.0));
		}
	
		for (int worker = 4; worker < 14; worker++)
		{
			// Create a TcpEchoClient application to send TCP packet to server.
  			NS_LOG_INFO("Create Client Application.");
 		    TcpEchoClientHelper echoClient (Address(ip[worker].GetAddress(1)), port);
  			echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
  			echoClient.SetAttribute ("Interval", TimeValue (MilliSeconds (2)));
  			echoClient.SetAttribute ("PacketSize", UintegerValue (2000));
  			ApplicationContainer clientApps = echoClient.Install (c[3]);
											
			//echoClient.SetFill (clientApps.Get (1), "Hello World");
											
  			clientApps.Start (Seconds (1.0));
  			clientApps.Stop (Seconds (20.0));
		}
    											

/*****************************************************************************************************/

	// FInally have some traces to see if the packet transfer are happenning or not
        csma.EnablePcap("s0", d[0].Get(1), false);
        csma.EnablePcap("s1", d[1].Get(1), false);
        csma.EnablePcap("s2", d[2].Get(1), false);
        csma.EnablePcap("s3", d[3].Get(1), false);
        csma.EnablePcap("s4", d[4].Get(1), false);
        csma.EnablePcap("s5", d[5].Get(1), false);
        csma.EnablePcap("s6", d[6].Get(1), false);
        csma.EnablePcap("s7", d[7].Get(1), false);
        csma.EnablePcap("s8", d[8].Get(1), false);
        csma.EnablePcap("s9", d[9].Get(1), false);
        csma.EnablePcap("s10", d[10].Get(1), false);
        csma.EnablePcap("s11", d[11].Get(1), false);
        csma.EnablePcap("s12", d[12].Get(1), false);
        csma.EnablePcap("s13", d[13].Get(1), false);
        csma.EnablePcap("s14", d[14].Get(1), false);
        csma.EnablePcap("s15", d[15].Get(1), false);

/****************************************Trace to Monitor Queue*****************************************/
if (queueState)
{	

	std::cout<<"Time,QueueSize"<<std::endl;
	Ptr<Queue> sourceQueue = d[1].Get(1)->GetObject<CsmaNetDevice> ()->GetQueue();
  sourceQueue->TraceConnectWithoutContext ("Enqueue", MakeCallback (&Enqueue));
  sourceQueue->TraceConnectWithoutContext ("Dequeue", MakeCallback (&Dequeue)); 
  sourceQueue->TraceConnectWithoutContext ("Drop", MakeCallback (&Drop));  
}

/*******************************************************************************************************/

       	
	// Code for flow monitor
        Ptr<FlowMonitor> flowmon;
        FlowMonitorHelper flowmonHelper;

        flowmon = flowmonHelper.InstallAll();
        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier()); 
       	
	NS_LOG_INFO("Run Simulation");
	Simulator::Stop(Seconds(20.0));
	Simulator::Run();

        if(enableMonitor){
          // printf("Entering the monitor\n");
	   				bzero(str1,30);
	   				bzero(str2,30);
	   				bzero(str3,30);

	   				strcpy(str1,"flowStats");
	   				snprintf(str2,10,"%d",10);
	   				strcat(str2,".flowmon");
	  			 	strcat(str1,str2);
	   				flowmon->SerializeToXmlFile(str1, false, false);
	   //std::map<FlowId, FlowMonitor::FlowStats> stats1= flowmon->GetFlowStats ();
           //print_stats(stats1);
	}
	bool Plot = true;
	if(Plot) {
			std::string fileNameWithNoExtension = "FlowVSThroughput_";
    	std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    	std::string plotFileName            = fileNameWithNoExtension + ".plt";
    	std::string plotTitle               = "Flow vs Throughput";
    	std::string dataTitle               = "Througput";
		
	    Gnuplot gnuplot(graphicsFileName);
			gnuplot.SetTitle (plotTitle);		
	
			gnuplot.SetTerminal ("png");

			gnuplot.SetLegend ("Flow", "Throughput");

	   Gnuplot2dDataset dataset;
			dataset.SetTitle (dataTitle);
	   dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);

		 ThroughputMonitor(&flowmonHelper, flowmon, dataset); 

	   gnuplot.AddDataset(dataset);
	    std::ofstream plotFile (plotFileName.c_str());
  		// Write the plot file.
  		gnuplot.GenerateOutput (plotFile);
  		// Close the plot file.
  		plotFile.close ();
	   //gnuplot.GenerateOutput(std::cout);
	}
	Simulator::Destroy();
	NS_LOG_INFO("Done");	

}// End of program         
