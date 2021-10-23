#include "sp2p.hpp"
#include <set>
using namespace std;

class pp2p : public sp2p{
private:
	set<string> delivers;
public:
	pp2p(unsigned long myID, vector<myhost>* hosts, const char* output): sp2p(myID, hosts, output){

	}
	void startPerfectLink(){
		thread sendthread(&sp2p::sp2pSend, this);
		thread recvthread(&pp2p::pp2pDeliver, this);
		sendthread.join();
		recvthread.join();
	}
	void pp2pSend(myhost target, string msg){
		task t;
		t.target = target;
		t.msg = msg;
		cout << "pp2pSend" << endl;
		taskQueue_mtx.lock();
		cout << "taskQueue.push" << endl;
		taskQueue.push(t);
		taskQueue_mtx.unlock();		
	}
	void pp2pDeliver(){
		while(!stop){
			deliver d = sp2pDeliver();
			if(d.ackflag == '0'){
				string msgVal = to_string(d.senderID) + d.msg;				
				if(!delivers.count(msgVal)){
					delivers.insert(msgVal);
					log += "d " + to_string(d.senderID) +" "+ d.msg +"\n"; 
					count++;
				}				
			}

		}
		
	}

};