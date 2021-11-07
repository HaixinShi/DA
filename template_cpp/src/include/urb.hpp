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
		//cout << "bebBroadcast:" << msg<< endl;
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
		
		//pendinglock.lock();
		//pending.insert(msgVal);
		//pendinglock.unlock();
		
		bebBroadcast(msgVal);
	}
	bool canDeliver(string msg){
		//cout << "canDeliver-1" << endl;
		if(ack.find(msg)!= ack.end()){
			//cout << "canDeliver-2" << endl;
			//cout << "ack_size:"+to_string(ack[msg].size())<< endl;
			//cout << "canDeliver-3" << endl;
			return 2*(ack[msg].size()) > hosts->size();
		}
		else{
			return false;
		}
	}
	void urbDeliver(){
		while(!stopflag){
			deliver d = bebDeliver();
			if(d.msg != ""){
				cout << "urbDeliver-3" << endl;
				cout << "urbDeliver:"<<d.msg << endl;
				cout << "real senderID: " << d.senderID <<endl;	
				string msgVal = d.msg;
				cout << "urbDeliver-4" << endl;
				if(ack.find(d.msg)!= ack.end()){
					ack[d.msg].insert(d.senderID);	
					cout << "urbDeliver-5-1" << endl;		
				}
				else{
					cout << "urbDeliver-5-2" << endl;
					set<unsigned long> temp;
					temp.insert(d.senderID);
					ack[d.msg] = temp;
				}
				cout << "urbDeliver-6" << endl;
				//pendinglock.lock();
				if(!pending.count(d.msg)){
					cout << "pending insert:" << d.msg << endl;
					pending.insert(d.msg);//original sender + msg
					bebBroadcast(d.msg);
				}
				//pendinglock.unlock();
				cout << "urbDeliver-7" << endl;
			}
		}
	}
	string urbTrytoDeliver(){
		if(true){//pendinglock.try_lock()
			//cout << "urbTrytoDeliver-2" << endl;
			for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
				string msgVal = *it;
				//cout << (*it) << endl;
				//if(canDeliver(msgVal) && !delivers.count(msgVal)){	
				if(!delivers.count(msgVal)){
					//cout << "1---urbTrytoDeliver pending:" << msgVal << endl;	
					//cout << "2---urbTrytoDeliver pending:" << msgVal << endl;		
					cout << "urbTrytoDeliver-3" << endl;
					delivers.insert(msgVal);
					cout << "urbTrytoDeliver-4" << endl;
					urbMsg urbm = deserializeMsg(msgVal);
					cout << "2---urbTrytoDeliver pending:" << urbm.body << endl;
					cout << "urbTrytoDeliver-5" << endl;
					return 	urbm.body;			
				}
			}
			//pendinglock.unlock();
		}
		//cout << "urbTrytoDeliver-6" << endl;
		return "";	
	}
};