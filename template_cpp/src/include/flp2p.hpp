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
	bool connected = false;//for sending process 
	unsigned long myID;//Process ID
	vector<myhost> hosts;//It stores the global information of processes
	string output;//log file address
	set<string> ack;
	mutex ack_mtx;
	string log;
	int s;
	bool stop = false;
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

		//prepare messages
		char* sendmsg = new char[msg.size()+2];
		for(unsigned int i = 0; i<msg.size(); i++){
			sendmsg[i] = msg[i];
		}
		sendmsg[msg.size()]='0';      
		sendmsg[msg.size()+1]='\0';		

		//prepare address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    addr.sin_family = AF_INET; // Use IPV4
	    addr.sin_port = port;
	    addr.sin_addr.s_addr = ip;

	    //send messages
	    ssize_t ret = 0;
	    ret = sendto(s, sendmsg, sizeof(sendmsg), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
	    cout << "UDP Send:"<< sendmsg << endl;
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
	    
	    char buffer[12];
	    ssize_t ret = recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
	    cout << "udp receive:" << buffer << endl;
	    if(ret == -1){
	    	cout << "errors in UDPReceive!--"<< strerror(errno) << endl;
	    }
	    size_t msglen = strlen(buffer); 
	    char ackflag = buffer[msglen-1];

	    string msg = "";
	    for(size_t i = 0; i< msglen-1; i++){
	    	msg += buffer[i]; 
	    }
    	
	    for(unsigned int i = 0; i< this->hosts.size(); i++){
	    	//get senderID
	    	if(hosts[i].ip == addr.sin_addr.s_addr && hosts[i].port == addr.sin_port){
	    		senderID = hosts[i].id;
	    	}
	    }
	    if(ackflag == '0'){
	    	// it is normal message
		    buffer[msglen-1] = '1';
		    sendto(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);	    	
	    }
	    else{
	    	//it is ack message
	    	ack_mtx.lock();
	    	string msgVal = to_string(senderID) + msg;

	    	if(!ack.count(msgVal))
	    		ack.insert(msgVal);
	    	ack_mtx.unlock();
	    }
		//prepare deliver and return	    
	    deliver d;
		d.msg = msg;
		d.senderID = senderID;
		d.ackflag = ackflag;

		return d;

 	}
	void flp2pSend(myhost target, string msg){
		
		UDPSend(s, target.ip, target.port, msg);

	}
	deliver flp2pDeliver(){
		
		return UDPReceive();

	}
};