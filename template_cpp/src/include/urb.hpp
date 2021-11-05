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
	bool stop = false;
	const char* output;
	mutex pendinglock;
	mutex acklock;
	void logfunction(){
		ofstream out;
		out.open(this->output);
		loglock.lock();
		if(log.length()>0){
			log.erase(log.end()-1);
		}
		out << log;
		loglock.unlock();
		out.flush();
		out.close();
		cout << "I finish logging!"<< endl;
	}	

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
		cout << "starturb 1"<< endl;
		pl -> startPerfectLink();//start sending
		//start listenning
		thread urbDeliverThread(&urb::urbDeliver, this);
		thread urbTrytoDeliverThread(&urb::urbTrytoDeliver, this);
		cout << "starturb 2"<< endl;
		//join the threads
		pl -> sendthreadPtr -> join();
		cout << "starturb 3"<< endl;
		urbDeliverThread.join();
		cout << "starturb 4"<< endl;
		urbTrytoDeliverThread.join();		
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
		while(!stop){
			deliver d = bebDeliver();
			if(d.msg != "N"){			
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
					pending.insert(d.msg);
					bebBroadcast(d.msg);
				}
				pendinglock.unlock();
			}
		}
	}
	urbMsg urbTrytoDeliver(){
		if(pendinglock.try_lock()){
			for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
				string msgVal = *it;
				cout << "urbTrytoDeliver pending:" << msgVal << endl;
				if(canDeliver(msgVal) && !delivers.count(msgVal)){
					delivers.insert(msgVal);
					return deserializeMsg(msgVal);			
				}
			}
			pendinglock.unlock();
		urbMsg fail;
		fail.body = "";
		return fail;	
	}
};