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
	map<string, vector<int>> ack;
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
		//start sending
		pl -> startPerfectLink();
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
		if(!pending.count(msgVal))
			pending.insert(msgVal);
		pendinglock.unlock();
		bebBroadcast(msgVal);
	}
	bool canDeliver(string msg){
		unsigned int sum = 0;
		for(unsigned int i = 0; i < hosts->size(); i++){
			acklock.lock();
			if(ack.count(msg)&&ack[msg][i]){
				++sum;
			}
			acklock.unlock();
		}
		return 2*sum > hosts->size();
	}
	void urbDeliver(){
		while(!stopflag){
			cout << "enter urbDeliver" << endl;
			deliver d = bebDeliver();
			if(d.msg != ""){			
				acklock.lock();
				if(ack.find(d.msg)!= ack.end()){
					if(ack[d.msg][d.senderID - 1]==0){
						ack[d.msg][d.senderID - 1]=1;
					}			
				}
				else{
					vector<int> temp(hosts->size(),0);
					temp[d.senderID-1] = 1;
					ack[d.msg] = temp; 
				}
				acklock.unlock();

				pendinglock.lock();
				if(!pending.count(d.msg)){
					cout << "urbDeliver_pending:"<< d.msg << endl;
					pending.insert(d.msg);
					bebBroadcast(d.msg);
				}
				pendinglock.unlock();
			}
			else{
				cout << "urbDeliver get empty msg" << endl;
			}
		}
	}
	urbMsg urbTrytoDeliver(){
		//cout << "enter urbTrytoDeliver" << endl;
		if(pendinglock.try_lock()){
			for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
				string msgVal = *it;
				cout << "urbTrytoDeliver pending:" << msgVal << endl;
				//if(canDeliver(msgVal) && !delivers.count(msgVal)){
					delivers.insert(msgVal);
					cout << "urb delivered:"<< msgVal << endl;
					return deserializeMsg(msgVal);			
				//}
			}
			pendinglock.unlock();	
		}
		urbMsg fail;
		fail.body = "";
		return fail;
	}
};