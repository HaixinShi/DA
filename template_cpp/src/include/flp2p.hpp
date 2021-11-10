#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
//#include <thread>
//#include <mutex>
#include <unordered_set>
#include <set>
#include "parser.hpp"
#include "packet.hpp"
using namespace std;
class deliver{
public:
	//udpPacket, size = 10byte
	char ackflag;
	urbPacket urbmsg;
	//udpPacket
	uint8_t realSenderID;
};
struct myhost{
	uint8_t id;
	in_addr_t ip;
	unsigned short port;
};
class flp2p {
/*
This class is for fair-loss link, 
which is the most basic link 
that is implemented by using UDP in this project 

*/
public:
	bool connected = false;//for sending process 
	uint8_t myID;//Process ID
	vector<myhost> hosts;//It stores the global information of processes
	string output;//log file address
	unordered_set<string> ack;//unorder_set
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
	//creator funtion of flp2p class
	flp2p(uint8_t myID, vector<myhost>* hosts, const char* output){
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
	void UDPSend(int s, in_addr_t ip, unsigned short port, urbPacket u){
		if(s == -1){
			cout<<"could not create socket while sending";
			return;
		}
		char buffer[10];
		buffer[0]='0';

		//copy msg-------9byte
		//uint8_t originalSenderID 1byte
		//int msg; 4byte
		//int seq; 4byte
		memcpy(buffer + 1, &u.originalSenderID, sizeof(uint8_t));
		memcpy(buffer + 1 +sizeof(uint8_t), &u.fifomsg.msg, sizeof(int));
		memcpy(buffer + 1 + sizeof(uint8_t) + sizeof(int), &u.fifomsg.seq, sizeof(int));
		//cout<<"----------UDP--send--msg-----" << to_string(u.fifomsg.msg)<< endl;
	    //cout<<"----------UDP--send--seq-----" << to_string(u.fifomsg.seq)<< endl;
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
	    ret = sendto(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
    	
    	if(ret == -1){
    		cout << "UDPSend fail!--"<< strerror(errno)<< endl;
    		return;
    	}
	}


	deliver UDPReceive(){
		//specify address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    //!!!!!we just open the door
	    
	    char buffer[10];
	    ssize_t ret = recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
	    
	    if(ret == -1){
	    	cout << "errors in UDPReceive!--"<< strerror(errno) << endl;
	    }
	   	deliver d;
	    for(unsigned int i = 0; i< this->hosts.size(); i++){
	    	//get senderID
	    	if(hosts[i].ip == addr.sin_addr.s_addr && hosts[i].port == addr.sin_port){
	    		d.realSenderID = hosts[i].id;
	    	}
	    }
		//copy msg-------9byte
		//uint8_t originalSenderID 1byte
		//int msg; 4byte
		//int seq; 4byte			    
	    if(buffer[0] == '0'){
	    	memcpy(&d.urbmsg.originalSenderID, buffer + 1, sizeof(uint8_t));
			memcpy(&d.urbmsg.fifomsg.msg, buffer +1 +sizeof(uint8_t), sizeof(int));
			//cout<<"----------UDP--recv--msg-----" << to_string(d.urbmsg.fifomsg.msg)<< endl;
			memcpy(&d.urbmsg.fifomsg.seq, buffer +1 +sizeof(uint8_t) + sizeof(int), sizeof(int));
	    	//cout<<"----------UDP--recv--seq-----" << to_string(d.urbmsg.fifomsg.seq)<< endl;
	    	d.ackflag = '0';
	    	buffer[0] = '1';
		    sendto(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);	    	
	    }
	    else{//it is ack message
	    	d.ackflag = '1';
	    	string msgVal = getID(d.realSenderID) + d.urbmsg.getTag();
	    	ack_mtx.lock();
	    	if(!ack.count(msgVal))
	    		ack.insert(msgVal);
	    	ack_mtx.unlock();
	    }
		return d;

 	}
	void flp2pSend(myhost target, urbPacket msg){
		
		UDPSend(s, target.ip, target.port, msg);

	}
	deliver flp2pDeliver(){
		
		return UDPReceive();

	}
};