#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unordered_set>
#include <set>
#include "parser.hpp"
#include "packet.hpp"
using namespace std;
class multitask{
public:
	unsigned int target;
	queue<urbPacket> urbmsgs;
	string getTag(){
		long unsigned int size = urbmsgs.size();
		string ret = to_string(target);
		while(size > 0){
			urbPacket temp = urbmsgs.front();
			ret += temp.getTag();
			urbmsgs.pop();
			urbmsgs.push(temp);
			--size;
 		}
 		return ret;
	}
};
class deliver{
public:
	//udpPacket, size = 10byte
	urbPacket urbmsg;
	//udpPacket
	uint8_t realSenderID = 0;
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
	SafeQueue<deliver> pending;
	unsigned int maxSize = 10;
	uint8_t myID;//Process ID
	vector<myhost> hosts;//It stores the global information of processes
	string output;//log file address
	unordered_set<string> ack;//unorder_set
	mutex ack_mtx;
	string log;
	int s;
	bool stopflag = false;
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
	void UDPSend(int s, in_addr_t ip, unsigned short port, queue<urbPacket> u){
		if(s == -1){
			cout<<"could not create socket while sending";
			return;
		}
		char buffer[91];//1 + 10*9 = 91
		memset(&buffer, 0, 91);
		buffer[0]='0';
		//cout << "UDP----Send----------------" << endl;
		for(unsigned i = 0; i < maxSize ; i++){
			//copy msg-------9byte
			//uint8_t originalSenderID 1byte
			//int msg; 4byte
			//int seq; 4byte
			if(!u.empty()){
				//cout << "UDP packet:" << to_string(u.front().fifomsg.msg) << endl;
				memcpy(buffer + 1 + 9*i, &u.front().originalSenderID, sizeof(uint8_t));
				memcpy(buffer + 1 + sizeof(uint8_t)+ 9*i, &u.front().fifomsg.msg, sizeof(int));
				memcpy(buffer + 1 + sizeof(uint8_t) + sizeof(int) + 9*i, &u.front().fifomsg.seq, sizeof(int));
				u.pop();
			}
			else{
				break;
			}

		}

		
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


	void UDPReceive(){
		//specify address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    //!!!!!we just open the door
	    char buffer[91];
	    while(!stopflag){
	    	memset(&buffer, 0, 91);
		    ssize_t ret = recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
		    
		    if(ret == -1){
		    	cout << "errors in UDPReceive!--"<< strerror(errno) << endl;
		    }
		    uint8_t realSenderID = 0;
		   	for(unsigned int i = 0; i< this->hosts.size(); i++){
	    		//get senderID
		    	if(hosts[i].ip == addr.sin_addr.s_addr && hosts[i].port == addr.sin_port){
		    		realSenderID = hosts[i].id;
		    	}
	    	}

		    if(buffer[0] == '0'){
		    	buffer[0] = '1';
			    sendto(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
		    	//cout << "-----UDP--Receive--------" << endl;
		    	for(unsigned int i = 0; i < maxSize; i++){
			    	deliver d;
			    	d.realSenderID = realSenderID;
			    	memcpy(&d.urbmsg.originalSenderID, buffer + 1 + 9*i, sizeof(uint8_t));
					memcpy(&d.urbmsg.fifomsg.msg, buffer +1 +sizeof(uint8_t) + 9*i, sizeof(int));
					//cout<<"----------UDP--recv--msg-----" << to_string(d.urbmsg.fifomsg.msg)<< endl;
					memcpy(&d.urbmsg.fifomsg.seq, buffer +1 +sizeof(uint8_t) + sizeof(int) + 9*i, sizeof(int));
			    	//cout<<"----------UDP--recv--seq-----" << to_string(d.urbmsg.fifomsg.seq)<< endl;	
				    //cout << "UDP unpack:"<< to_string(d.urbmsg.fifomsg.msg) << endl;
				    if(d.urbmsg.originalSenderID != 0){
				    	pending.push(d);
				    }
				    else{
				    	break;
				    }
		    	}	    
		    }
		    else{//it is ack message
		    	multitask mt;
		    	mt.target = realSenderID;
		    	for(unsigned int i = 0; i < maxSize; i++){
			    	deliver d;
			    	d.realSenderID = realSenderID;
			    	memcpy(&d.urbmsg.originalSenderID, buffer + 1 + 9*i, sizeof(uint8_t));
					memcpy(&d.urbmsg.fifomsg.msg, buffer +1 +sizeof(uint8_t) + 9*i, sizeof(int));
					//cout<<"----------UDP--recv--msg-----" << to_string(d.urbmsg.fifomsg.msg)<< endl;
					memcpy(&d.urbmsg.fifomsg.seq, buffer +1 +sizeof(uint8_t) + sizeof(int) + 9*i, sizeof(int));
			    	//cout<<"----------UDP--recv--seq-----" << to_string(d.urbmsg.fifomsg.seq)<< endl;	
				    if(d.urbmsg.originalSenderID != 0){
				    	mt.urbmsgs.push(d.urbmsg);
				    }
				    else{
				    	break;
				    }
		    	}
		    	ack_mtx.lock();
		    	if(!ack.count(mt.getTag()))
		    		ack.insert(mt.getTag());
		    	ack_mtx.unlock();
		    	continue;
		    }		   				    
	    }
 	}
	void flp2pSend(unsigned int target, queue<urbPacket> msg){
		
		UDPSend(s, hosts[target - 1].ip, hosts[target - 1].port, msg);

	}
};