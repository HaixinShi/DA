#include "urb.hpp"	
#include <atomic>
using namespace std;
class lcb
{
public:
	//traffic control
	long unsigned int max_balance = 0;
	long unsigned int memory = 3840; 
	atomic_uint balance = 0;
	//
	int lsn = 0;
	vector<int>V;
	map<int, vector<int>> neighbor;
	string log;
	mutex loglock;
	map<int, set<int>> affectSet;
	map<int, set<urbPacket>>pending;
	uint8_t myID;
	vector<myhost>* hosts;
	bool stopflag = false;
	const char* output;
	
	urb* urbPtr;
	void logfunction(){
		ofstream out;
		out.open(this->output);
		loglock.lock();
		out << log;
		loglock.unlock();
		out.flush();
		out.close();
		cout << "I finish logging!"<< endl;
	}
	lcb(uint8_t myID, vector<myhost>* hosts, const char* output, map<int, vector<int>>&neighbor, map<int, set<int>> &affectSet){
		this -> myID = myID;
		this -> hosts = hosts;
		this -> output = output;
		this -> neighbor = neighbor;
		this -> affectSet = affectSet;
		for(unsigned int i = 0; i < hosts->size();i++){
			V.push_back(0);
		}
		max_balance = memory/hosts->size();
		urbPtr = new urb(myID, hosts, output);		
	}
	~lcb(){
		delete urbPtr;
		urbPtr = NULL;
	}
	void lcbBroadcast(int msg){
		
		while(balance > max_balance){
			sleep(1);
		}
		//cout << "balance:" << to_string(balance) << endl;
		loglock.lock();
		log += "b " + to_string(msg) + "\n";
		loglock.unlock();
		
		lcbPacket l;
		l.V = V;
		l.V[myID - 1] = lsn;
		l.msg = msg;
		++lsn;
		urbPtr -> urbBroadcast(l);
		balance++;
	}
	bool canDeliver(urbPacket u){
		for(unsigned int i = 0; i< neighbor[u.originalSenderID].size(); i++){
			int index = neighbor[u.originalSenderID][i] - 1;
			if(V[index] < u.lcbmsg.V[index]){
				return false;
			}
		}
		return true;
	}
	void recursiveDeliver(int id, bool hasDeliverInput){
		//TODO self again
		if(!pending.count(id)){
			return;
		}
		bool hasDeliver = hasDeliverInput;
		//for lcbDeliver function call, ensure it would call affectSet
		//for recursiveDeliver function call, 
		//ensure to check if its set can deliver any message

		set<urbPacket>::iterator it = pending[id].begin();
		while(it != pending[id].end()){
			urbPacket temp = (*it);
			if(canDeliver(temp)){
				//cout << "lcb can deliver: balance: "<< to_string(balance)<< endl;
				++V[temp.originalSenderID - 1];
				loglock.lock();
				int originalSenderID = temp.originalSenderID;
				log += "d " + to_string(originalSenderID) +" "+ to_string(temp.lcbmsg.msg) +"\n";
				//cout << "d " <<" "<< temp.getTag() +"\n";
				loglock.unlock();
				if(temp.originalSenderID == myID){
					balance--;
				}
				hasDeliver = true;	
				pending[id].erase(it++);				
			}
			else{
				break;
			}
		}
		if(hasDeliver){
			//map<int, set<int>> affectSet;	
		    for (auto iter = affectSet[id].begin(); iter != affectSet[id].end(); ++iter) {
		        int index = *iter;
		        recursiveDeliver(index, false);
		    }		
		}		
	}
	void lcbDelibver(urbPacket u){
		if(canDeliver(u)){
			++V[u.originalSenderID - 1];
			loglock.lock();
			int originalSenderID = u.originalSenderID;
			log += "d " + to_string(originalSenderID) +" "+ to_string(u.lcbmsg.msg) +"\n";
			loglock.unlock();	
			if(u.originalSenderID == myID){
				balance--;
			}
			recursiveDeliver(u.originalSenderID, true);
		}
		else{
			//map<int, queue<urbPacket>>pending;
			if(pending.count(u.originalSenderID)){
				pending[u.originalSenderID].insert(u);
			}
			else{
				set<urbPacket> temp;
				temp.insert(u);
				pending[u.originalSenderID] = temp;
			}
		}	
	}
	
};