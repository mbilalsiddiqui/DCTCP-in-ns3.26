**--Implementation of DCTCP in ns3 and Generation of DCTCP results for Fat-Tree Datacenter Topology--**

Following changes are made in four ns3 source code files:
--> Main file is tcp-socket-base.cc which contains ECN at TCP and DCTCP implementation. (In ns3 it is present in src/internet/model/tcp-socket-base.cc)
--> ipv4-queue-disc-item.cc contains Mark method and red-queue-disc.cc uses that method for switch.
--> "tcp-socket.h" contains definition of ECN states.


  
Associated .h files are not included because they only contain declarations for other files. Only tcp-socket.h is important.
  
To run DCTCP and ECN in RED queue and TCP. Only these lines have to be added in the application:
1)	To set "K" parameter, set K = threshold
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (threhold));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (threhold));
2)	To Enable ECN in RED Queue:
Config::SetDefault ("ns3::TcpSocketBase::UseEcn", BooleanValue (true));
Config::SetDefault ("ns3::RedQueueDisc::UseEcn", BooleanValue (true));
3)	To Enable DCTCP and Set Weight Factor:      
 Config::SetDefault ("ns3::TcpSocketBase::useDCTCP", BooleanValue(true));
 Config::SetDefault ("ns3::TcpSocketBase::DctcpGainFactor", DoubleValue (1.0 / 16));
 
 


--> The four tcp experiments correspond to the four experiments mentioned in the presentation

		1) Throughput and Queue Length experiment 
		2) Incast performance experiment
		3) Queue buildup performacnce experiment
		4) Buffer pressure experiment

--> The one dctcp experiment is equivalent to TCP experiment with the relevant modules and libraries added
		for dctcp to function, all tcp experiments can be made dctcp experiments in the similar fashion.

--> The 64-server-fat-tree builds a larger fat-tree topology in ns3.

Note: Each test can be run individually. There is a lot of overlapping code in those test files.
