#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <mutex>
#include <set>
#include "parser.hpp"
using namespace std;
struct deliver{
	string msg;
	unsigned long senderID;
	string ackflag;
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
struct udpMsg{
	string ackflag;
	string udpmsg;
};
class flp2p {
/*
This class is for fair-loss link, 
which is the most basic link 
that is implemented by using UDP in this project 

*/
public:
	bool connected = false;//for sending process 
	unsigned long myID;//Process ID
	vector<myhost> hosts;//It stores the global information of processes
	string output;//log file address
	set<string> ack;
	mutex ack_mtx;
	string log;
	int s;
	bool stopflag = false;
	void logfuction(){
		ofstream out;
		out.open(this->output);
		if(log.length()>0){
			log.erase(log.end()-1);
		}
		out << log;
		out.flush();
		out.close();
		cout << "I finish logging!"<< endl;
	}
	string serializeMsg(udpMsg m){
		string msg =  m.ackflag + ","+ m.udpmsg;
		return msg;
	}
	udpMsg deserializeMsg(string m){
		udpMsg msg;
		size_t split = m.find(",");
		msg.ackflag = m.substr(0, split);
		msg.udpmsg = m.substr(split + 1, m.size()- 1 -split);
		return msg;
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
		
	}
	~flp2p(){

	}
	void UDPSend(int s, in_addr_t ip, unsigned short port, string msg){
		if(s == -1){
			cout<<"could not create socket while sending";
			return;
		}
		udpMsg m;
		m.ackflag = "0";
		m.udpmsg = msg;
		string ready_msg = serializeMsg(m);
		//prepare messages
		char* sendmsg = new char[ready_msg.size()+1];
		for(unsigned int i = 0; i<ready_msg.size(); i++){
			sendmsg[i] = ready_msg[i];
		}
		sendmsg[ready_msg.size()]='\0';      
		//sendmsg[msg.size()+1]='\0';		 
		//prepare address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    addr.sin_family = AF_INET; // Use IPV4
	    addr.sin_port = port;
	    addr.sin_addr.s_addr = ip;

	    //send messages
	    ssize_t ret = 0;
	    //cout << "UDP send:" << sendmsg << endl;
	    //cout << "UDP send size:" << to_string(ready_msg.size()+1) << endl;
	    ret = sendto(s, sendmsg, ready_msg.size()+1, 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
    	if(ret == -1){
    		cout << "UDPSend fail!--"<< strerror(errno)<< endl;
    		return;
    	}
	}


	deliver UDPReceive(){

		unsigned long senderID = 0;
		//specify address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    //!!!!!we just open the door
	    
	    char buffer[20];
	    ssize_t ret = recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
	    if(ret == -1){
	    	cout << "errors in UDPReceive!--"<< strerror(errno) << endl;
	    }
	    //cout << "UDP receive:"<< buffer <<endl;
	    //size_t msglen = strlen(buffer); 
	    //char ackflag = buffer[msglen-1];
	    string temp(buffer);
	    udpMsg recvmsg = deserializeMsg(temp);
	    /*
	    string msg = "";
	    for(size_t i = 0; i< msglen-1; i++){
	    	msg += buffer[i]; 
	    }*/
    	
	    for(unsigned int i = 0; i< this->hosts.size(); i++){
	    	//get senderID
	    	if(hosts[i].ip == addr.sin_addr.s_addr && hosts[i].port == addr.sin_port){
	    		senderID = hosts[i].id;
	    	}
	    }
	    if(recvmsg.ackflag == "0"){//ackflag == '0'
	    	// it is normal message
		    //buffer[msglen-1] = '1';
		    recvmsg.ackflag == "1";
			string temp = serializeMsg(recvmsg);
		    sendto(s, const_cast<char *>(temp.c_str()), sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);	    	
	    }
	    else{
	    	//it is ack message
	    	ack_mtx.lock();
	    	string msgVal = to_string(senderID) + recvmsg.udpmsg;

	    	if(!ack.count(msgVal))
	    		ack.insert(msgVal);
	    	ack_mtx.unlock();
	    }
		//prepare deliver and return	    
	    deliver d;
		d.msg = recvmsg.udpmsg;
		d.senderID = senderID;
		d.ackflag = recvmsg.ackflag;
		return d;

 	}
	void flp2pSend(myhost target, string msg){
		
		UDPSend(s, target.ip, target.port, msg);

	}
	deliver flp2pDeliver(){
		
		return UDPReceive();

	}
};