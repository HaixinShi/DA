#include "sp2p.hpp"
using namespace std;

class pp2p : public sp2p{
private:
	set<string> delivers;

public:
	void (*callurb) (deliver);

	pp2p(uint8_t myID, vector<myhost>* hosts, const char* output): sp2p(myID, hosts, output){

	}
	void pp2pSend(unsigned int target, urbPacket u){
		/*
		while(taskQueueSize > 1000){
			sleep(1);
		}*/
		task t;
		t.target = target;
		t.urbmsg = u;
		//taskQueue_mtx.lock();
		taskQueue.push(t);
		//++taskQueueSize;
		//taskQueue_mtx.unlock();		
	}
	~pp2p(){
	}
	void pp2pDeliver(deliver d){
		//while true
		if(d.realSenderID != 0){
			string msgVal = getID(d.realSenderID)+ d.urbmsg.getTag();
			//cout << "-------try to pp2pDeliver:"<< msgVal <<endl;
			if(!delivers.count(msgVal)){
				//cout << "-------pp2pDeliver:"<< msgVal <<endl;
				delivers.insert(msgVal);
				//return d;
				callurb(d);
			}				
		}
		//return d;		
	}

};