#include "pp2p.hpp"
#include <map>
#include <sstream>
using namespace std;

struct message{
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
	string log;

	mutex loglock;
	mutex pendinglock;
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
		loglock.lock();
		log += "b " + msg + "\n";
		loglock.unlock();
		message m;
		m.senderID = myID;
		m.body = msg;

		pendinglock.lock();
		if(!pending.count(myID + msg))
			pending.insert(myID + msg);
		pendinglock.unlock();
		bebBroadcast(serializeMsg(m));
	}
	bool canDeliver(string msg){
		unsigned int sum = 0;
		for(unsigned int i = 0; i < hosts->size(); i++){
			if(ack.count(msg)&&ack[msg][i]){
				++sum;
			}
		}
		cout << "sum:" << sum << endl;
		if(2*sum > hosts->size())
			return true;
		else
			return false;
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