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
	
	queue<urbPacket>pending;
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
	lcb(uint8_t myID, vector<myhost>* hosts, const char* output, map<int, vector<int>>&neighbor){
		this -> myID = myID;
		this -> hosts = hosts;
		this -> output = output;
		this -> neighbor = neighbor;
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
	void lcbDelibver(urbPacket u){
		pending.push(u);					
		long unsigned int cur = pending.size();
		long unsigned int prev = pending.size();
		while(!stopflag && cur){
			urbPacket temp = pending.front();
			pending.pop();
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
			}
			else{
				//cout << "lcb can not deliver:" << temp.getTag() << endl;
				pending.push(temp);
			}
			--cur;
			//cout << "cur:" << to_string(cur) << endl;
			if(cur == 0){
				//cout << "pending size:" << to_string(pending.size()) << endl;
				//cout << "prev:" << to_string(prev) << endl;
				if(prev == pending.size()){
					break;
				}
				else{
					prev = pending.size();
					cur = pending.size();
				}
			}
		}
		//cout << "lcb pending size:" << to_string(pending.size()) << endl;		
	}
	
};