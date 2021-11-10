#include "sp2p.hpp"
using namespace std;

class pp2p : public sp2p{
private:
	set<string> delivers;

public:
	thread* sendthreadPtr;
	pp2p(uint8_t myID, vector<myhost>* hosts, const char* output): sp2p(myID, hosts, output){

	}
	void startPerfectLink(){
		sendthreadPtr = new thread(&sp2p::sp2pSend, this);
	}
	void pp2pSend(myhost& target, urbPacket u){
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
		if(d.ackflag == '0'){
			//cout << "-------pp2pDeliver:"<<endl;
			string msgVal = getID(d.realSenderID)+ d.urbmsg.getTag();
			//cout << "-------pp2pDeliver:"<< msgVal <<endl;
			if(!delivers.count(msgVal)){
				delivers.insert(msgVal);
				return d;
			}				
		}
		d.ackflag = '1';
		return d;		
	}

};