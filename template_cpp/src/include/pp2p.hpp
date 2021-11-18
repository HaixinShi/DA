#include "sp2p.hpp"
using namespace std;

class pp2p : public sp2p{
private:
	set<string> delivers;

public:
	thread* sendthreadPtr;
	thread* receivethreadPtr;
	pp2p(uint8_t myID, vector<myhost>* hosts, const char* output): sp2p(myID, hosts, output){

	}
	void startPerfectLink(){
		sendthreadPtr = new thread(&sp2p::sp2pSend, this);
		receivethreadPtr = new thread(&flp2p::UDPReceive, this);
	}
	void pp2pSend(unsigned int target, urbPacket u){
		task t;
		t.target = target;
		t.urbmsg = u;
		//taskQueue_mtx.lock();
		taskQueue.push(t);
		//taskQueue_mtx.unlock();		
	}
	deliver pp2pDeliver(){
		deliver d = sp2pDeliver();
		//while true
		if(d.realSenderID != 0){
			string msgVal = getID(d.realSenderID)+ d.urbmsg.getTag();
			//cout << "-------try to pp2pDeliver:"<< msgVal <<endl;
			if(!delivers.count(msgVal)){
				//cout << "-------pp2pDeliver:"<< msgVal <<endl;
				delivers.insert(msgVal);
				return d;
			}				
		}
		return d;		
	}

};