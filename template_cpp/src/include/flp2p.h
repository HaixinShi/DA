#include <fstream>
#include <iostream>
#include <string>
#include "parser.hpp"
using namespace std;
class flp2p {
/*
This class is for fair-loss link, 
which is the most basic link 
that is implemented by using UDP in this project 

*/
private:
	unsigned long myID;//Process ID
	vector<Host> host;//It stores the global information of processes
	string output;//log file address
	string config;//determine the behaviours of process
	int send_seq=1;
	int recv_seq=1;
	ofstream log;
public:
	//creator funtion of flp2p class
	flp2p(unsigned long myID, vector<Host> host, string output, string config);
	~flp2p(){
		log.close();
	}
	void UDPSend(string ip, unsigned short port, char* message);
	void UDPReceive();
	void flp2pSend();
	void flp2pDeliver();
}