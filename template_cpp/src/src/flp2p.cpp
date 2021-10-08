#include "flp2p.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include "parser.hpp"
using namespace std;
flp2p::flp2p(unsigned long myID, vector<Host> host, string output, string config){
	this.myID=myID;
	this.host=host;
	this.output=output;
	this.config=config;
	this.log.open(this.output);
	thread th1(flp2pSend);
	thread th2(flp2pDeliver);
	th1.join();
	th2.join();
}

void flp2p::UDPSend(string ip, unsigned short port, char* message){
	//create socket
	int s = socket(AF_INET, SOCK_DGRAM, 0);
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
    inet_aton(ip, &myaddr.sin_addr.s_addr);

    //send the message to the address
    sendto(s, message, sizeof(message), 0, (struct sockaddr *)&addr, addr_len);

}

void flp2p::flp2pSend(){
	while(1)//single thread or multi threads
	for (auto &host : hosts){
		if(host.id != myID){
			int ID=myID;
			//message only include current Process ID 			
			UDPSend(host.ip, host.port, itoa(ID));
			//log this send event
			log<<"b "+ myID<<endl;
			send_seq++;
		}
	}
}

void flp2p::UDPReceive(string ip, unsigned short port, char* buffer){
	//create socket
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s == -1){
		cout<<"could not create socket while receiving";
		return;
	}

	//specify address
	struct sockaddr_in addr;
    socklen_t addr_len=sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; // Use IPV4
    addr.sin_port = htons(port);
    inet_aton(ip, &myaddr.sin_addr.s_addr);

    //receive the message from the address
    recvfrom(s, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, addr_len);
}

void flp2p::flp2pDeliver(){
	while(1)//single thread or multi threads
	for (auto &host : hosts){
		if(host.id != myID){
			char* recvinfo;
			//message only include send Process ID 			
			UDPReceive(host.ip, host.port,recvinfo);
			//log this receive event
			log<<"d "<<recvinfo<<" "+recv_seq<<endl;
			recv_seq++;
		}
	}
}