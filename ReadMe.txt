/******************-------------------------------Bilal's Code---------------------------------------------------------******************

Following changes are made in four ns3 source code files:
• Main file is “tcp-socket-base.cc” which contains ECN at TCP and DCTCP implementation. (In ns3 it is present in src/internet/model/tcp-socket-base.cc)
• “ipv4-queue-disc-item.cc” contains Mark method and “red-queue-disc.cc” uses that method for switch.
• "tcp-socket.h" contains definition of ECN states.

I have also commented the portions of code. Main Changes in the source files are present in starred blocks of this type.  e.g. In tcp-socket.h file, the changes are: (Because it’s from a “.h” file so formatting will be destroyed).  Similarly, all files contain modified code in these type of blocks so it is easy to scan those portions.
/***********************************************************************************/   
  // ECN state related attributes which are used in ns-2. But were not in ns3 before. 
  typedef enum {
    NO_ECN        = 0x0,     //  ECN is not ON
    ECN_CONN      = 0x1,     //  ECN Connection established
    ECN_TX_ECHO   = 0x2,     //  Receiver is sending ECN Echo to transmitter
    ECN_RX_ECHO   = 0x4,     //  Sender Received ECN Echo
    ECN_SEND_CWR  = 0x8      //  In response to ECN Echo, Sender Reduces window and sends Congestion Window Reduced flag
    
  } EcnStates_t;
/***********************************************************************************/
  
Associated .h files are not included because they only contain declarations for other files. Only tcp-socket.h is important.
  
To run DCTCP and ECN in RED queue and TCP. Only these lines have to be added in the application:
1)	To set “K” parameter, set K = threshold
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (threhold));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (threhold));
2)	To Enable ECN in RED Queue:
Config::SetDefault ("ns3::TcpSocketBase::UseEcn", BooleanValue (true));
Config::SetDefault ("ns3::RedQueueDisc::UseEcn", BooleanValue (true));
3)	To Enable DCTCP and Set Weight Factor:      
 Config::SetDefault ("ns3::TcpSocketBase::useDCTCP", BooleanValue(true));
 Config::SetDefault ("ns3::TcpSocketBase::DctcpGainFactor", DoubleValue (1.0 / 16));
 
 
        
/*****----------------------------------------------------Yahya's Code------------------------------------******/

--> The four tcp experiments correspond to the four experiments mentioned in the presentation

		1) Throughput and Queue Length experiment 
		2) Incast performance experiment
		3) Queue buildup performacnce experiment
		4) Buffer pressure experiment

--> The one dctcp experiment is equivalent to TCP experiment with the relevant modules and libraries added
		for dctcp to function, all tcp experiments can be made dctcp experiments in the similar fashion.

--> The 64-server-fat-tree builds a larger fat-tree topology in ns3.
