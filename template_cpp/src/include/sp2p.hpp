#include "SafeQueue.hpp"
#include "flp2p.hpp"
using namespace std;
class task{
public:
	myhost target;
	urbPacket urbmsg;
};
class sp2p : public flp2p{
public:
	//mutex taskQueue_mtx;
	SafeQueue<task> taskQueue;
	
	sp2p(uint8_t myID, vector<myhost>* hosts, const char* output): flp2p(myID, hosts, output){

	}
	void sp2pSend(){
		while(!stopflag){
			//taskQueue_mtx.lock();
			if(!taskQueue.empty()){
				task t;//= taskQueue.front();
				taskQueue.move_pop(t);
				//taskQueue.pop();
				ack_mtx.lock();				
				if(!ack.count(getID(t.target.id) + t.urbmsg.getTag())){
					//cout << "--------sp2pSend:" << 	getID(t.target.id) + t.urbmsg.getTag()<< endl;
					taskQueue.push(t);
					flp2pSend(t.target, t.urbmsg);
				}
				ack_mtx.unlock();							
			}	
			//taskQueue_mtx.unlock();
			
		}
		
	}
	deliver sp2pDeliver(){
		return flp2pDeliver();
	}
};
