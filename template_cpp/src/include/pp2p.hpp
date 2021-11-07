#include "sp2p.hpp"
using namespace std;

class pp2p : public sp2p{
private:
	set<string> delivers;

public:
	thread* sendthreadPtr;
	pp2p(unsigned long myID, vector<myhost>* hosts, const char* output): sp2p(myID, hosts, output){

	}
	void startPerfectLink(){
		sendthreadPtr = new thread(&sp2p::sp2pSend, this);
	}
	void pp2pSend(myhost& target, string msg){
		task t;
		t.target = target;
		t.msg = msg;
		taskQueue_mtx.lock();
		taskQueue.push(t);
		taskQueue_mtx.unlock();		
	}
	deliver pp2pDeliver(){
		deliver d = sp2pDeliver();
		//while true
		if(d.ackflag == "0"){
			string msgVal = to_string(d.senderID) + d.msg;				
			if(!delivers.count(msgVal)){
				cout << "pp2pDeliver:"<< d.msg << endl;
				delivers.insert(msgVal);
				return d;
			}				
		}
		d.msg = "";
		return d;		
	}

};