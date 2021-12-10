#include "sp2p.hpp"
using namespace std;

class pp2p : public sp2p{

public:
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
};