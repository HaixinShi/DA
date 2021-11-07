#include "pp2p.hpp"
#include <map>
#include <sstream>
using namespace std;

struct urbMsg{
	string senderID;
	string body;
};

class urb{

public:
	map<string, set<unsigned long>> ack;
	set<string> delivers;
	set<string> pending;


	pp2p* pl;
	string myID;
	vector<myhost>* hosts;
	bool stopflag = false;
	const char* output;
	mutex pendinglock;	
	mutex acklock;
	string serializeMsg(urbMsg m){
		string msg = m.senderID + ","+m.body;
		return msg;
	}
	urbMsg deserializeMsg(string m){
		urbMsg msg;
		size_t split = m.find(",");
		msg.senderID = m.substr(0, split);
		msg.body = m.substr(split + 1, m.size()- 1 -split);
		return msg;
	}

	urb(unsigned long myID, vector<myhost>* hosts, const char* output){
		this -> myID = to_string(myID);
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


	void bebBroadcast(string msg){
		for(unsigned int j = 0; j < hosts-> size(); j++){
			pl -> pp2pSend((*hosts)[j], msg);
		}
	}
	deliver bebDeliver(){
		return 	pl -> pp2pDeliver();			
	}

	void urbBroadcast(string msg){
		urbMsg m;
		m.senderID = myID;
		m.body = msg;
		string msgVal = serializeMsg(m);
		
		pendinglock.lock();
		pending.insert(msgVal);
		pendinglock.unlock();
		
		bebBroadcast(msgVal);
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
			if(d.msg != ""){	
				string msgVal = d.msg;
				acklock.lock();
				if(ack.find(d.msg)!= ack.end()){
					ack[d.msg].insert(d.senderID);			
				}
				else{
					set<unsigned long> temp;
					temp.insert(d.senderID);
					ack[d.msg] = temp;
				}
				acklock.unlock();
				pendinglock.lock();
				if(!pending.count(d.msg)){
					pending.insert(d.msg);//original sender + msg
					bebBroadcast(d.msg);
				}
				pendinglock.unlock();
			}
		}
	}
	string urbTrytoDeliver(){
		if(pendinglock.try_lock()){
			for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
				string msgVal = *it;
				if(canDeliver(msgVal) && !delivers.count(msgVal)){	
					delivers.insert(msgVal);
					urbMsg urbm = deserializeMsg(msgVal);
					pendinglock.unlock();
					return 	urbm.body;			
				}
			}
			pendinglock.unlock();
		}
		return "";	
	}
};