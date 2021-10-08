#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include "parser.hpp"
using namespace std;

struct myhost{
	unsigned long id;
	in_addr_t ip;
	unsigned short port;
};

class flp2p {
/*
This class is for fair-loss link, 
which is the most basic link 
that is implemented by using UDP in this project 

*/
private:
	unsigned long myID;//Process ID
	vector<myhost> hosts;//It stores the global information of processes
	string output;//log file address
	string config;//determine the behaviours of process
	int send_seq=1;
	int recv_seq=1;
	ofstream log;
public:
	//creator funtion of flp2p class
	flp2p(unsigned long myID, vector<myhost>* hosts, const char* output, const char* config){
		cout << "enter creator" << endl;
		this->myID = myID;
		this->hosts = *hosts;
		this->output = output;
		this->config = config;
		this->log.open(this->output);
		cout << "send thread init start" << endl;
		thread sendthread(flp2pSend, this);
		cout << "send thread init finish" << endl;

		cout << "receive thread init start" << endl;
		vector<thread> recvthreads;
		for(unsigned int i=0; i<this->hosts.size(); i++){
			if(this->hosts[i].id != this->myID)
				recvthreads.push_back(thread(flp2pDeliver, this, this->hosts[i]));
		}
		cout << "receive thread init finish" << endl;		
		sendthread.join();
		for(unsigned int i=0; i<this->hosts.size(); i++){
			recvthreads[i].join();
		}
		
	}
	~flp2p(){
		log.close();
	}
	void UDPSend(int s, in_addr_t ip, unsigned short port, char* message){

		if(s == -1){
			cout<<"could not create socket while sending";
			return;
		}

		//specify address
		struct sockaddr_in addr;
	    socklen_t addr_len=sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    addr.sin_family = AF_INET; // Use IPV4
	    addr.sin_port = htons(port);
	    addr.sin_addr.s_addr = ip;

	    //send the message to the address
	    sendto(s, message, sizeof(message), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
	}
	void UDPReceive(int s, in_addr_t ip, unsigned short port, char* buffer){
		cout << "enter function : UDPReceive" << endl;
		if(s == -1){
			cout << "could not create socket while receiving";
			return;
		}

		//specify address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    addr.sin_family = AF_INET; // Use IPV4
	    addr.sin_port = htons(port);
	    //inet_aton(ip, &myaddr.sin_addr.s_addr);
	    addr.sin_addr.s_addr = ip;
	    cout << "1" << endl;
	    //receive the message from the address
	    recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
		cout << "2" << endl;
	}
	static void flp2pSend(flp2p* thiz){
		//create socket
		int s = socket(AF_INET, SOCK_DGRAM, 0);
		while(1)//single thread or multi threads
		for (auto &host : thiz->hosts){
			if(host.id != thiz->myID){
				//message only include current Process ID 			
				thiz->UDPSend(s, host.ip, host.port, reinterpret_cast<char*>(&thiz->myID));
				//log this send event
				string tag = "b ";
				string loginfo = tag + to_string(thiz->send_seq);
				thiz->log << loginfo << endl;
				thiz->send_seq++;
			}
		}
	}
	static void flp2pDeliver(flp2p* thiz, myhost host){
		//create socket
		int s = socket(AF_INET, SOCK_DGRAM, 0);
		while(1){//single thread or multi threads
			char recvinfo[256];
			//message only include send Process ID 			
			thiz->UDPReceive(s, host.ip, host.port,recvinfo);
			//log this receive event
			string tag = "d ";
			cout << "Deliver is writing!" << endl;
			string loginfo = tag + recvinfo + to_string(thiz->recv_seq);
			thiz->log << loginfo << endl;
			cout << loginfo << endl;
			thiz->recv_seq++;
		}
	}
};