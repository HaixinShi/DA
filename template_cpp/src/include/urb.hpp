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
	int msgNum;
	bool stop;
	const char* output;
	string serializeMsg(message m){
		string msg = m.senderID + ","+m.body;
		return msg;
	}
	message deserializeMsg(string m){
		message msg;
		size_t split = m.find(",");
		msg.senderID = m.substr(0, split);
		msg.body = m.substr(split + 1, m.size()- 1 -split);
	}

	urb(unsigned long myID, vector<myhost>* hosts, const char* output, int msgNum){
		this -> myID = to_string(myID);
		this -> hosts = hosts;
		this -> output = output;
		pl = new pp2p(myID, hosts, output);
		pl -> startPerfectLink();
	}
	void bebBroadcast(string msg){
		for(unsigned int j = 0; j < hosts-> size(); j++){
			pl -> pp2pSend(hosts[j], msg);
		}
	}
	deliver bebDeliver(){
		return 	p -> pp2pDeliver();			
	}

	void urbBroadcast(string msg){
		pending.insert(myID + msg);
		bebBroadcast(msg);
	}
	bool canDeliver(){
		return (ack.size()*1.0 - (0.5)*hosts->size()) > 0;
	}
	deliver urbDeliver(){
		deliver d = bebDeliver();
		string msgVal = to_string(d.senderID) + d.msg;
		if(ack.find(msgVal)!= ack.end()){
			if(ack[msgVal][d.senderID]==0){
				ack[msgVal][d.senderID]==1;
			}			
		}
		else{
			vector<unsigned long> temp(hosts->size(),0);
			temp[d.senderID-1] = 1;
			ack[msgVal] = temp; 
		}

		if(!pending.count(msgVal)){
			pending.insert(msgVal);
			bebBroadcast(msg);
		}

	}
	void trytoDeliver(){
		if(pending.count(msgVal) && )
		delivers.insert();
	}
}