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
	unsigned int seq;
	queue<urbPacket> urbmsgs;
};
class deliver{
public:
	urbPacket urbmsg;
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
	long unsigned int maxSize;
	uint8_t myID;//Process ID
	vector<myhost> hosts;//It stores the global information of processes
	string output;//log file address
	//unordered_set<string> ack;//unorder_set
	set<unsigned int> ack;
	mutex ack_mtx;
	string log;
	int s;
	bool stopflag = false;
	
	set<string> delivers;//in order to avoid duplication of messages
	//call back

	void (*callurb) (deliver);

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
		maxSize = (1472 - sizeof(unsigned int) - sizeof(char))/(sizeof(int)*(this->hosts.size()+1) + sizeof(uint8_t));
	}
	~flp2p(){

	}
	void UDPSend(int s, in_addr_t ip, unsigned short port, queue<urbPacket> u, unsigned int seq){
		if(s == -1){
			cout<<"could not create socket while sending";
			return;
		}
		char buffer[1472];
		memset(&buffer, 0, 1472);
		unsigned long int offset = 0;
		buffer[0]='0';
		offset += sizeof(char);
		memcpy(buffer + offset, &seq, sizeof(unsigned int));
		offset += sizeof(unsigned int);
		//cout << "UDP----Send---------------" << endl;
		//cout << "max_size:" << to_string(maxSize) << endl;
		for(unsigned i = 0; i < maxSize ; i++){
			//cout << to_string(i) << endl;
			//put messages
			//ack_flag 1byte
			//seq unsigned int
			//[
			//uint8_t originalSenderID 1byte
			//vector<int> host_size*4 bytes
			//int msg 4bytes
			//]
			if(!u.empty()){
				memcpy(buffer + offset, &u.front().originalSenderID, sizeof(uint8_t));
				offset += sizeof(uint8_t);

				for(unsigned int j = 0; j < this->hosts.size(); j++){
					memcpy(buffer + offset, &u.front().lcbmsg.V[j], sizeof(int));
					offset += sizeof(int); 
				}
				//cout << "UDP send msg offset:"<<to_string(offset)<< endl;
				memcpy(buffer + offset, &u.front().lcbmsg.msg, sizeof(int));
				offset += sizeof(int);
				//cout << "UDP send"<<u.front().getTag() << endl;
				u.pop();
			}
			else{
				break;
			}

		}
		//cout << "UDP----Send---------------2" << endl;
		
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
	    //cout << "UDP----Send---------------3" << endl;
	    ret = sendto(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
    	//cout << buffer << endl;
    	//cout << to_string(sizeof(buffer)) << endl;
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
	    char buffer[1472];
	    while(!stopflag){
	    	memset(&buffer, 0, 1472);
	    	//cout << "enter UDP receive:"<< endl;
		    ssize_t ret = recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
		    //cout << "ack flag:" <<buffer[0] << endl;
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
	    	unsigned long int offset = sizeof(char);
			unsigned int seq;
			memcpy(&seq, buffer + sizeof(char), sizeof(unsigned int));
		    if(buffer[0] == '0'){
		    	buffer[0] = '1';
		    	char ackbuffer[sizeof(char)+sizeof(unsigned int)];
		    	offset += sizeof(unsigned int);
		    	memcpy(ackbuffer, buffer, sizeof(char)+sizeof(unsigned int));
			    sendto(s, ackbuffer, sizeof(ackbuffer), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
		    	//cout << "-----UDP--Receive----not---ack----" << endl;
			    string tag = getID(realSenderID)+ to_string(seq);
			    if(!delivers.count(tag)){
			    	delivers.insert(tag);
			    	for(unsigned int i = 0; i < maxSize; i++){
			    		//get messages
						//vector<int> host_size*4 bytes
						//uint8_t originalSenderID 1byte
						//int msg 4bytes
				    	deliver d;
				    	d.realSenderID = realSenderID;
				    	memcpy(&d.urbmsg.originalSenderID, buffer + offset, sizeof(uint8_t));
						if(d.urbmsg.originalSenderID == 0){
							//cout << "originalSenderID is zero" << endl;
					    	break;
					    }				
						offset += sizeof(uint8_t);				
						for(unsigned int j = 0; j < this->hosts.size(); j++){
							d.urbmsg.lcbmsg.V.push_back(0);
							memcpy(&d.urbmsg.lcbmsg.V[j], buffer + offset, sizeof(int));
							offset += sizeof(int); 
						}	  
						memcpy(&d.urbmsg.lcbmsg.msg, buffer + offset, sizeof(int)); 
						offset += sizeof(int);
						callurb(d); 	
			    	}
			    }	    
		    }
		    else{//it is ack message
		    	//cout << "-----UDP--Receive------ack----" << endl;
				ack_mtx.lock();
				ack.erase(seq);
		    	ack_mtx.unlock();		    	
		    }		   				    
	    }
 	}
	void flp2pSend(unsigned int target, queue<urbPacket> msg, unsigned int seq){
		//cout << "enter flp2pSend---" << endl;
		UDPSend(s, hosts[target - 1].ip, hosts[target - 1].port, msg, seq);
	}
};