#include <queue>
#include "flp2p.hpp"
using namespace std;

class sp2p : public flp2p{
public:
	mutex taskQueue_mtx;
	queue<task> taskQueue;
	//set<string> sended;	
	sp2p(unsigned long myID, vector<myhost>* hosts, const char* output): flp2p(myID, hosts, output){

	}
	void sp2pSend(){
		while(!stopflag){
			taskQueue_mtx.lock();
			if(!taskQueue.empty()){
				task t = taskQueue.front();
				string msg(t.msg);
				string msgVal = to_string(t.target.id) + msg;
				taskQueue.pop();
				ack_mtx.lock();					
				if(!ack.count(msgVal)){
					taskQueue.push(t);
					flp2pSend(t.target, t.msg);
					
					//log this send event
					/*					
					if(!sended.count(msgVal)){
						sended.insert(msgVal);
						string m(t.msg);
						//log += "b " + m + "\n";
					}*/
				}
				ack_mtx.unlock();							
			}	
			taskQueue_mtx.unlock();
			
		}
		
	}
	deliver sp2pDeliver(){
		return flp2pDeliver();
	}
};
