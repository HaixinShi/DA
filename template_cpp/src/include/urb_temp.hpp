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
	bool stop = false;
	const char* output;
	mutex pendinglock;	

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
		cout << "bebBroadcast:" << msg<< endl;
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
		string msgVal = serializeMsg(m);//original sender + m
		
		pendinglock.lock();
		pending.insert(msgVal);
		pendinglock.unlock();
		
		bebBroadcast(msgVal);
	}
	bool canDeliver(string msg){
		if(ack.find(msg)!= ack.end()){
			cout << "ack_size:"+to_string(ack[msg].size())<< endl;
			return 2*(ack[msg].size()) > hosts->size();
		}
		else{
			return false;
		}
	}
	void urbDeliver(){
		while(!stop){
			deliver d = bebDeliver();
			if(d.msg != "N"){
				cout <<"urbDeliver:"<<d.msg<<endl;
				string msgVal = d.msg;
				if(ack.find(msgVal)!= ack.end()){
					if(ack[msgVal][d.senderID]==0){
						ack[msgVal][d.senderID]=1;
					}			
				}
				else{
					vector<int> temp(hosts->size(),0);
					temp[d.senderID-1] = 1;
					ack[msgVal] = temp; 
				}
				pendinglock.lock();
				if(!pending.count(msgVal)){
					pending.insert(msgVal);
					bebBroadcast(msgVal);
				}
				pendinglock.unlock();
			}
			urbTrytoDeliver();
		}
	}
	void urbTrytoDeliver(){
		if(pendinglock.try_lock()){
			for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
				string msgVal = *it;
				cout << "urbTrytoDeliver pending:" << msgVal << endl;
				if(canDeliver(msgVal) && !delivers.count(msgVal)){	
				//if(!delivers.count(msgVal)){		
					delivers.insert(msgVal);
					message m = deserializeMsg(msgVal);
					loglock.lock();
					log += "d " + m.senderID +" "+ m.body +"\n";
					loglock.unlock();			
				}
			}
			pendinglock.unlock();
		}	
	}
};