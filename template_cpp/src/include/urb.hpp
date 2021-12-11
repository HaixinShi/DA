#include "pp2p.hpp"
#include <map>
#include <sstream>
#include <stdio.h>
#include <time.h>
using namespace std;

class urb{

public:
	map<string, set<uint8_t>> ack;
	unordered_set<string> delivers;

	pp2p* pl;
	uint8_t myID;
	vector<myhost>* hosts;
	bool stopflag = false;
	const char* output;

	void (*calllcb) (urbPacket);	
	//mutex acklock;
	urb(uint8_t myID, vector<myhost>* hosts, const char* output){
		this -> myID = myID;
		this -> hosts = hosts;
		this -> output = output;
		pl = new pp2p(myID, hosts, output);
		
	}
	~urb(){
		delete pl;
		pl = NULL;
	}

	void bebBroadcast(urbPacket u){
		for(unsigned int j = 0; j < hosts-> size(); j++){
			pl -> pp2pSend(j+1, u);
		}
	}
	void urbBroadcast(lcbPacket l){
		urbPacket u;
		u.originalSenderID = myID;
		u.lcbmsg = l;	
		bebBroadcast(u);
	}
	bool canDeliver(string msg){
		bool flag = false;
		if(ack.find(msg)!= ack.end()){
			flag = 2*(ack[msg].size()) > hosts->size();
			if(flag){
				ack.erase(msg);
			}
		}
		return flag;
	}

	void urbDeliver(deliver d){
		//TODO:check bebbroadcast
		string tag = getID(d.urbmsg.originalSenderID) + to_string(d.urbmsg.lcbmsg.V[d.urbmsg.originalSenderID-1]);
		if(delivers.count(tag)){
			return;
		}
		if(ack.find(tag)!= ack.end()){
			//in pending set
			ack[tag].insert(d.realSenderID);			
		}
		else{
			//not in pending set
			set<uint8_t> temp;
			temp.insert(d.realSenderID);
			ack[tag] = temp;
			bebBroadcast(d.urbmsg);
		}
		if(canDeliver(tag)){
			calllcb(d.urbmsg);
			delivers.insert(tag);
		}
	}

};