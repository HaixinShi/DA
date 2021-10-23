#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <set>
#include "parser.hpp"
using namespace std;
struct deliver{
	string msg;
	unsigned long senderID;
	char ackflag;
};
struct myhost{
	unsigned long id;
	in_addr_t ip;
	unsigned short port;
};
struct task{
	myhost target;
	string msg;
};

class flp2p {
/*
This class is for fair-loss link, 
which is the most basic link 
that is implemented by using UDP in this project 

*/
public:
	bool connected = false;
	bool sendProcess = false;//this property is specific for Perfect Links
	int count = 0;//the number of messages
	unsigned long myID;//Process ID
	vector<myhost> hosts;//It stores the global information of processes
	string output;//log file address
	int send_seq=1;
	set<string> ack;
	mutex ack_mtx;
	string log;
	int s;
	bool stop = false;
	void logfuction(){
		ofstream out;
		out.open(this->output);
		out << log;
		out.flush();
		out.close();
		cout << "I finish logging!"<< endl;
	}
	//creator funtion of flp2p class
	flp2p(unsigned long myID, vector<myhost>* hosts, const char* output){
		cout << "enter creator" << endl;
		this->myID = myID;
		this->hosts = *hosts;
		this->output = output;
		//this->log.open(this->output);

		//bind the listening socket
		this->s = socket(AF_INET, SOCK_DGRAM, 0);
		for(unsigned int i = 0; i < this->hosts.size(); i++){
			if(this->hosts[i].id == this->myID) {
				struct sockaddr_in addr;
			    socklen_t addr_len = sizeof(addr);
			    memset(&addr, 0, sizeof(addr));
			    addr.sin_family = AF_INET; // Use IPV4
			    addr.sin_port = this->hosts[i].port;
			    //inet_aton(ip, &myaddr.sin_addr.s_addr);
			    addr.sin_addr.s_addr = this->hosts[i].ip;
				if( bind(this->s , reinterpret_cast<struct sockaddr *>(&addr), addr_len ) == -1)
				{
					cout<<"bind error!" << endl;
				}
				else {
					cout << "bind success!" <<endl;
				}				
			}
				
		}
		//unblocked for stop immediately
		/*
		struct timeval t;
		t.tv_sec = 3;
		t.tv_usec = 0;
		if (setsockopt(this->s, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1){
			cout << "setsockopt fail!"<< endl;
		}*/		
		
		/*		
		cout << "sending thread init start" << endl;
		thread sendthread(flp2pSend, this);
		cout << "sending thread init finish" << endl;

		cout << "receive thread init start" << endl;
		vector<thread> recvthreads;
		for(unsigned int i=0; i<this->hosts.size(); i++){
			if(this->hosts[i].id != this->myID) 
				recvthreads.push_back(thread(flp2pDeliver, this, this->hosts[i]));
		}
		cout << "receive thread init finish" << endl;		
		
		//join the threads
		sendthread.join();
		for(unsigned int i=0; i<recvthreads.size(); i++){
			recvthreads[i].join();
		}
		//testing
		*/
		/*
		this->s = socket(AF_INET, SOCK_DGRAM, 0);
		if(myID == 1){
			cout << "enter Process 1" << endl;
			thread th1(flp2pSend, this);
			th1.join();
		}
		if(myID == 2){
			cout << "enter Process 2" << endl;
			struct sockaddr_in addr;
		    socklen_t addr_len = sizeof(addr);
		    memset(&addr, 0, sizeof(addr));
		    addr.sin_family = AF_INET; // Use IPV4
		    addr.sin_port = this->hosts[1].port;
		    //inet_aton(ip, &myaddr.sin_addr.s_addr);
		    addr.sin_addr.s_addr = this->hosts[1].ip;
			if( bind(this->s , reinterpret_cast<struct sockaddr *>(&addr), addr_len ) == -1)
			{
				cout<<"bind error!" << endl;
			}
			thread th2(flp2pDeliver, this, this->hosts[0]);
			th2.join();
		}*/
		
	}
	~flp2p(){

	}
	void UDPSend(int s, in_addr_t ip, unsigned short port, string msg){
		cout << "enter UDPSend" << endl;
		if(s == -1){
			cout<<"could not create socket while sending";
			return;
		}
		//for send process, apply connect function to set target process for sending messages
		//so in this case, we can improve performance
		
		if(!connected) {
			//specify address
			struct sockaddr_in addr;
		    socklen_t addr_len = sizeof(addr);
		    memset(&addr, 0, sizeof(addr));
		    addr.sin_family = AF_INET; // Use IPV4
		    addr.sin_port = port;
		    addr.sin_addr.s_addr = ip;
			connect(s, reinterpret_cast<struct sockaddr *>(&addr), addr_len);			
			connected = true;
		}	

	    //send the message to the address
	    //ssize_t ret = 0;
    	//ret = sendto(s, message, sizeof(message), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
    	//ret = send(s, message, sizeof(message), 0);
    	/*
    	stringstream ss;
    	ss<<message;
    	char ackflag = '0';
    	ss<< ackflag;
    	char* packet = new char(sizeof(message));
    	ss >> packet;*/
    	/*
    	char ackflag ='0';
    	char* msg_ack = new char[sizeof(message)+1];
    	cout << "msg_ack size: "<< to_string(sizeof(message)+1);
    	memcpy(msg_ack, message, sizeof(message));
    	memcpy(msg_ack + sizeof(message), &ackflag, sizeof(ackflag));
    	*/  
		char* sendmsg = new char[msg.size()+2];
		for(unsigned int i = 0; i<msg.size(); i++){
			sendmsg[i] = msg[i];
		}
		sendmsg[msg.size()]='0';      
		sendmsg[msg.size()+1]='\0';     

    	send(s, sendmsg, sizeof(sendmsg), 0);
    	
    	/*
    	if(ret == -1){
    		cout << "UDPSend fail!"<< endl;
    		return;
    	}*/
    	cout << "leave UDPSend" << endl;
	}


	deliver UDPReceive(){
		cout << "enter UDPReceive" <<endl;
		unsigned long senderID = 0;
		//specify address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    //!!!!!we just open the door
	    
	    //addr.sin_family = AF_INET; // Use IPV4
	    //addr.sin_port = port;
	    //inet_aton(ip, &myaddr.sin_addr.s_addr);
	    //addr.sin_addr.s_addr = ip;
	    //receive the message from the address
	    //char recvinfo[8];
	    //char buffer[9];
	    char buffer[12];
	    ssize_t ret = recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
	    if(ret == -1){
	    	cout << "errors in UDPReceive!" << endl;
	    }
	    size_t msglen = strlen(buffer); 
	    char ackflag = buffer[msglen-1];

	    string msg = "";
	    for(size_t i = 0; i< msglen-1; i++){
	    	msg += buffer[i]; 
	    }
    	
    	cout << "msg: "<< msg << endl;
    	cout <<"ackflag: " << ackflag << endl;
	    /*
	    stringstream ss;
	    ss << buffer;
	    char ackflag;
	    ss >> recvinfo;
	    cout << "recvinfo" << recvinfo << endl;
	    ss >> ackflag;
	    cout << "ackflag" << ackflag << endl;
    	*/
    	//it is not ack messages
    	
	    for(unsigned int i = 0; i< this->hosts.size(); i++){
	    	if(hosts[i].ip == addr.sin_addr.s_addr && hosts[i].port == addr.sin_port){
	    		senderID = hosts[i].id;
	    		cout << "senderID: "<< to_string(senderID) << endl;
	    	}
	    }
	    if(ackflag == '0'){

		    buffer[msglen-1] = '1';
		    sendto(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);	    	
	    }
	    else{
	    	//string msg(recvinfo);
	    	ack_mtx.lock();
	    	string msgVal = to_string(senderID) + msg;

	    	if(!ack.count(msgVal))
	    		ack.insert(msgVal);
	    	ack_mtx.unlock();
	    }
	    /*
	   	if(sendProcess){
	   		//I want to find out ack
	   		stringstream ss;
	   		ss << buffer;
	   		int seq;
	   		ss >> seq;
	   		mtx.lock();
	   		ack[seq - 1] = true;
	   		mtx.unlock();
	   		cout << "I receive ack "<< to_string(seq) << endl;
	    }
	    else{
	    	//I want to send ack to the sender
	    	sendto(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
	    	cout << "I send ack" << buffer << endl;
	    }

	    for(unsigned int i = 0; i< this->hosts.size(); i++){
	    	if(hosts[i].ip == addr.sin_addr.s_addr && hosts[i].port == addr.sin_port){
	    		senderID = hosts[i].id;
	    	}
	    }
	    */
	    
	    deliver d;
		d.msg = msg;
		d.senderID = senderID;
		d.ackflag = ackflag;
		cout << "leave UDPReceive" <<endl;
		return d;

 	}
	void flp2pSend(myhost target, string msg){
		
		UDPSend(s, target.ip, target.port, msg);
		/*
		for (auto &host : thiz->hosts){
			if(host.id != thiz->myID){
				//message only include current Process ID
				string seq = to_string(thiz->send_seq);
				char* sendmsg = new char[seq.size()+1];
				for(unsigned int i = 0; i<seq.size(); i++){
					sendmsg[i] = seq[i];
				}
				sendmsg[seq.size()]='\0';		
				thiz->UDPSend(thiz->s, host.ip, host.port,sendmsg);
				//log this send event
				string tag = "b ";
				string loginfo = tag + to_string(thiz->send_seq);
				thiz->log << loginfo << endl;
				thiz->send_seq++;
			}
		}*/
		//message only include current Process ID
		
		/*
		if(!thiz -> stop)
			if(retransmit == false){
				thiz-> send_seq = 1;
				for(int i = 0; i < m; i++){
					string seq = to_string(thiz->send_seq);
					char* sendmsg = new char[seq.size()+1];
					for(unsigned int i = 0; i<seq.size(); i++){
						sendmsg[i] = seq[i];
					}
					sendmsg[seq.size()]='\0';
			
					thiz->UDPSend(thiz->s, target.ip, target.port, sendmsg);
					
					thiz -> sended.push_back(sendmsg);
					thiz->count++;
					//log this send event
					thiz->log += "b " + seq + "\n";

					thiz->send_seq++;
				}
			}
			else{
				for(unsigned int i = 0; i < thiz -> sended.size(); i++){
					mtx.lock();					
					if(!thiz -> ack[i]){
						thiz->UDPSend(thiz->s, target.ip, target.port, thiz -> sended[i]);	
					}
					mtx.unlock();
				}
			}
		else{

		}*/

	}
	deliver flp2pDeliver(){
		/*
		while(1){//single thread or multi threads
			char recvinfo[10];
			//message only include send Process ID 			
			unsigned long senderID = thiz->UDPReceive(thiz->s, recvinfo);
				
			//log this receive event
			string tag = "d ";
			string loginfo = tag + to_string(senderID) +" "+ recvinfo;
			thiz->log << loginfo << endl;
		}*/
		//char recvinfo[8];
		//message only include send Process ID 			
		return UDPReceive();
		/*	
		//log this receive event
		string tag = "d ";
		string loginfo = tag + to_string(senderID) +" "+ recvinfo;
		thiz->log << loginfo << endl;
		*/
	}
};