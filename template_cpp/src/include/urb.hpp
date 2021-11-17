#include "pp2p.hpp"
#include <map>
#include <sstream>
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
	//mutex pendinglock;	
	mutex acklock;

	urb(uint8_t myID, vector<myhost>* hosts, const char* output){
		this -> myID = myID;
		this -> hosts = hosts;
		this -> output = output;
		pl = new pp2p(myID, hosts, output);
		
	}
	void starturb(){

		pl -> startPerfectLink();//start sending
		//start listenning
		thread urbDeliverThread(&urb::urbDeliver, this);

		//join the threads
		pl -> sendthreadPtr -> join();
		urbDeliverThread.join();		
	}


	void bebBroadcast(urbPacket u){
		for(unsigned int j = 0; j < hosts-> size(); j++){
			pl -> pp2pSend((*hosts)[j], u);
		}
	}
	deliver bebDeliver(){
		return 	pl -> pp2pDeliver();			
	}

	void urbBroadcast(fifoPacket f){
		urbPacket u;
		u.originalSenderID = myID;
		u.fifomsg = f;

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
	void urbDeliver(){
		while(!stopflag){
			deliver d = bebDeliver();
			if(d.realSenderID != 0){
				//cout << "----------urbDeliver" <<endl;
				string urb_str = d.urbmsg.getTag();
				acklock.lock();
				if(ack.find(urb_str)!= ack.end()){
					ack[urb_str].insert(d.realSenderID);			
				}
				else{
					set<uint8_t> temp;
					temp.insert(d.realSenderID);
					ack[urb_str] = temp;
				}
				acklock.unlock();

				if(!pendingTag.count(urb_str)){
					pendingTag.insert(urb_str);
					pending.push(d.urbmsg);//original sender + msg +seq
					bebBroadcast(d.urbmsg);
				}
			}
		}
	}
	urbPacket urbTrytoDeliver(){
			long unsigned int num = pending.size();
			while(!stopflag && num > 0&&!pending.empty()){
				urbPacket u;//= pending.front();
				pending.move_pop(u);
				string urb_str = u.getTag();

				if(canDeliver(urb_str) && !delivers.count(urb_str)){
					//cout << "---------urbCanDeliver:" << urb_str <<endl;	
					delivers.insert(urb_str);
					return 	u;			
				}
				else{
					pending.push(u);
				}
			--num;
			}
		urbPacket u;
		u.originalSenderID = 0;
		return u;	
	}
};