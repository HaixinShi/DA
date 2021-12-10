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
	SafeQueue<urbPacket> pending;
	unordered_set<string> pendingTag;

	pp2p* pl;
	uint8_t myID;
	vector<myhost>* hosts;
	bool stopflag = false;
	const char* output;

	void (*calllcb) (urbPacket);
	//mutex pendinglock;	
	mutex acklock;
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

		pending.push(u);
		
		bebBroadcast(u);
	}
	bool canDeliver(string msg){
			acklock.lock();
			bool flag = false;
			if(ack.find(msg)!= ack.end())
				flag = 2*(ack[msg].size()) > hosts->size();
			acklock.unlock();
			return flag;
	}
	void urbTrytoDeliver(){
		//long unsigned int num = pending.size();
		//while(!stopflag && num > 0&&!pending.empty()){
		while(!stopflag){
			if(pending.empty())
				continue;
			//time_t t;
			//time(&t);
			//cout << "time:"<< ctime(&t)<<"urb pending size:" << to_string( pending.size()) << endl;
			urbPacket u;//= pending.front();
			pending.move_pop(u);
			string tag = getID(u.originalSenderID)+to_string(u.lcbmsg.V[u.originalSenderID-1]);
			if(canDeliver(tag) && !delivers.count(tag)){
				//cout << "---------urbCanDeliver:" << urb_str <<endl;	
				delivers.insert(tag);
				pendingTag.erase(tag);
				//return 	u;
				calllcb(u);
			}
			else{
				pending.push(u);
			}
		//--num;
		}
		/*
		urbPacket u;
		u.originalSenderID = 0;	
		return u;*/
	}
	void urbDeliver(deliver d){
		//while(!stopflag){
			//deliver d = bebDeliver();
			//if(d.realSenderID != 0){
				//cout << "----------urbDeliver" <<endl;
				//string urb_str = d.urbmsg.getTag();
				string tag = getID(d.urbmsg.originalSenderID)+to_string(d.urbmsg.lcbmsg.V[d.urbmsg.originalSenderID-1]);
				acklock.lock();
				if(ack.find(tag)!= ack.end()){
					ack[tag].insert(d.realSenderID);			
				}
				else{
					set<uint8_t> temp;
					temp.insert(d.realSenderID);
					ack[tag] = temp;
				}
				acklock.unlock();

				if(!pendingTag.count(tag)){
					pendingTag.insert(tag);
					pending.push(d.urbmsg);//original sender + msg +seq
					//urbTrytoDeliver();
					bebBroadcast(d.urbmsg);
				}
			//}
		//}
	}

};