#include "pp2p.hpp"
#include <map>
#include <sstream>
using namespace std;

struct message{
	string senderID;
	string body;
};
/*
class safeSet{
private:
	set<string> unsafeSet;
	mutex setLock;
public:
	void insert(string s){
		setLock.lock();
		unsafeSet.insert(s);
		setLock.unlock();
	}
	void 
};*/
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
	string log;

	mutex loglock;
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

	string serializeMsg(message m){
		string msg = m.senderID + ","+m.body;
		return msg;
	}
	message deserializeMsg(string m){
		message msg;
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
		loglock.lock();
		log += "b " + msg + "\n";
		loglock.unlock();
		message m;
		m.senderID = myID;
		m.body = msg;

		pendinglock.lock();
		pending.insert(myID + msg);
		pendinglock.unlock();
		bebBroadcast(serializeMsg(m));
	}
	bool canDeliver(string msg){
		unsigned int sum = 0;
		for(unsigned int i = 0; i < hosts->size(); i++){
			acklock.lock();
			if(ack[msg][i]){
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
				string msgVal = to_string(d.senderID) + d.msg;
				
				acklock.lock();
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
				acklock.unlock();

				pendinglock.lock();
				if(!pending.count(msgVal)){
					pending.insert(msgVal);
					bebBroadcast(msgVal);
				}
				pendinglock.unlock();
			}
		}
	}
	void urbTrytoDeliver(){
		while(!stop){
		pendinglock.lock();
		for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
			string msgVal = *it;
			if(canDeliver(msgVal) && !delivers.count(msgVal)){
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