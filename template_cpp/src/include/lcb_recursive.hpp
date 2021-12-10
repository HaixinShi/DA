#include "urb.hpp"	
using namespace std;
class lcb
{
public:
	int balance = 0;
	int lsn = 0;
	vector<int>V;
	map<int, vector<int>> neighbor;
	string log;
	mutex loglock;
	map<int, set<int>> affectSet;
	map<int, queue<urbPacket>>pending;
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
		urbPtr = new urb(myID, hosts, output);		
	}
	~lcb(){
		delete urbPtr;
		urbPtr = NULL;
	}
	void lcbBroadcast(int msg){
		while(balance > 1000){
			sleep(1);
		}
		loglock.lock();
		log += "b " + to_string(msg) + "\n";
		loglock.unlock();
		
		lcbPacket l;
		l.V = V;
		l.V[myID - 1] = lsn;
		l.msg = msg;
		++lsn;
		urbPtr -> urbBroadcast(l);
		++balance;
	}
	bool canDeliver(urbPacket u){
		/*string v ="[";
		for(unsigned int i = 0; i< V.size(); i++){
			v += to_string(V[i]) + "," ;
		}
		v += "]";
		cout << v << endl;*/
		for(unsigned int i = 0; i< neighbor[u.originalSenderID].size(); i++){
			int index = neighbor[u.originalSenderID][i] - 1;
			if(V[index] < u.lcbmsg.V[index]){
				return false;
			}
		}
		return true;
	}
	void recursiveDeliver(int id){
		if(!pending.count(id)){
			return;
		}
		bool hasDeliver = false;
		long unsigned int cur = pending[id].size();
		long unsigned int prev = pending[id].size();
		while(!stopflag && cur){
			urbPacket temp = pending[id].front();
			pending[id].pop();
			if(canDeliver(temp)){
				//cout << "lcb can deliver: "<< temp.getTag() << endl;
				++V[temp.originalSenderID - 1];
				loglock.lock();
				int originalSenderID = temp.originalSenderID;
				log += "d " + to_string(originalSenderID) +" "+ to_string(temp.lcbmsg.msg) +"\n";
				//cout << "d " <<" "<< temp.getTag() +"\n";
				loglock.unlock();
				if(temp.originalSenderID == myID){
					--balance;
				}
				hasDeliver = true;				
			}
			else{
				//cout << "lcb can not deliver:" << temp.getTag() << endl;
				pending[id].push(temp);
			}
			--cur;
			//cout << "cur:" << to_string(cur) << endl;
			if(cur == 0){
				//cout << "pending size:" << to_string(pending.size()) << endl;
				//cout << "prev:" << to_string(prev) << endl;
				if(prev == pending[id].size()){
					break;
				}
				else{
					prev = pending[id].size();
					cur = pending[id].size();
				}
			}
		}
		if(hasDeliver){
			//map<int, set<int>> affectSet;	
		    for (auto iter = affectSet[id].begin(); iter != affectSet[id].end(); ++iter) {
		        int index = *iter;
		        recursiveDeliver(index);
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
				--balance;
			}
			recursiveDeliver(u.originalSenderID);
		}
		else{
			//map<int, queue<urbPacket>>pending;
			if(pending.count(u.originalSenderID)){
				pending[u.originalSenderID].push(u);
			}
			else{
				queue<urbPacket> temp;
				temp.push(u);
				pending[u.originalSenderID] = temp;
			}
		}	
	}
	
};