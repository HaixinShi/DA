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
	ofstream log;
	int s;
public:
	//creator funtion of flp2p class
	flp2p(unsigned long myID, vector<myhost>* hosts, const char* output, const char* config){
		cout << "enter creator" << endl;
		this->myID = myID;
		this->hosts = *hosts;
		this->output = output;
		this->config = config;
		this->log.open(this->output);

		//bind the listening socket
		this->s = socket(AF_INET, SOCK_DGRAM, 0);
		for(unsigned int i=0; i<this->hosts.size(); i++){
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
			}
				
		}		


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
		log.close();
	}
	void UDPSend(int s, in_addr_t ip, unsigned short port, char* message){
		if(s == -1){
			cout<<"could not create socket while sending";
			return;
		}
		//specify address
		struct sockaddr_in addr;
	    socklen_t addr_len = sizeof(addr);
	    memset(&addr, 0, sizeof(addr));
	    addr.sin_family = AF_INET; // Use IPV4
	    addr.sin_port = port;
	    addr.sin_addr.s_addr = ip;

	    //send the message to the address
	    ssize_t ret = 0;
	    ret = sendto(s, message, sizeof(message), 0, reinterpret_cast<struct sockaddr *>(&addr), addr_len);
		if(ret == -1){
			cout << "sending encounter errors!" << endl;
		}
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
	    addr.sin_port = port;
	    //inet_aton(ip, &myaddr.sin_addr.s_addr);
	    addr.sin_addr.s_addr = ip;
	    //receive the message from the address
	    recvfrom(s, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addr_len);
	}
	static void flp2pSend(flp2p* thiz){
		//create socket
		int s = socket(AF_INET, SOCK_DGRAM, 0);
		while(1)//single thread or multi threads
		for (auto &host : thiz->hosts){
			if(host.id != thiz->myID){
				//message only include current Process ID
				string seq = to_string(thiz->send_seq);
				char* sendmsg = new char[seq.size()+1];
				for(unsigned int i = 0; i<seq.size(); i++){
					sendmsg[i] = seq[i];
				}
				sendmsg[seq.size()]='\0';		
				thiz->UDPSend(s, host.ip, host.port,sendmsg);
				//log this send event
				string tag = "b ";
				string loginfo = tag + to_string(thiz->send_seq);
				thiz->log << loginfo << endl;
				thiz->send_seq++;
			}
		}
	}
	static void flp2pDeliver(flp2p* thiz, myhost host){
		while(1){//single thread or multi threads
			char recvinfo[10];
			//message only include send Process ID 			
			thiz->UDPReceive(thiz->s, host.ip, host.port,recvinfo);
			//log this receive event
			string tag = "d ";
			string loginfo = tag + to_string(host.id) +" "+ recvinfo;
			thiz->log << loginfo << endl;
		}
	}
};