#include <time.h>
#include <queue>
#include <list>
#include "flp2p.hpp"
using namespace std;

class sp2p : public flp2p{
private:
	long int start_time = 0;
	long int end_time = 0;
public:
	mutex taskQueue_mtx;
	queue<task> taskQueue;
	set<string> sended;	
	sp2p(unsigned long myID, vector<myhost>* hosts, const char* output): flp2p(myID, hosts, output){

	}
	void sp2pSend(){
		while(!stop){
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
					count++;
					//log this send event
					
					if(!sended.count(msgVal)){
						sended.insert(msgVal);
						string m(t.msg);
						log += "b " + m + "\n";
					}
				}
				ack_mtx.unlock();							
			}	
			taskQueue_mtx.unlock();
			
		}




		/*
		while(!stop){
			while(!taskQueue.empty()){
				mtx.lock();
				task t = taskQueue.front();
				flp2pSend(t.target, t.msg);
				sendSet.push_back(t);
				taskQueue.pop();
				mtx.unlock();
			}
			while(taskQueue.empty()&&!stop){
				list<task>::iterator i;
				for(i = sendSet.begin(); i != sendSet.end(); i++){
					flp2pSend((*i).target, (*i).msg);
				}				
			}
		}*/


		/*
		bool retransmit = false; 
		flp2pSend(target, retransmit);
		retransmit = true;
		start_time = clock();
		//srand(static_cast<int>(thiz->start_time));
		while(!stop){
			//thiz-> timeout = random_timeout(0,3);
			end_time = clock();
			if((end_time - start_time)/CLOCKS_PER_SEC > timeout){
				//microseconds
				cout << "send again!" << endl;
				thiz->flp2pSend(target, retransmit);
				thiz->start_time = clock();
			}
		}*/
		
	}
	deliver sp2pDeliver(){
		return flp2pDeliver();
		/*
		deliver d = flp2pDeliver();
		if(d.ackflag == '1'){
			//this message is ack
			list<task>::iterator i;
			sendSet_mtx.lock();
			for(i = sendSet.begin(); i != sendSet.end(); i++){
				string dmsg(d.msg);
				string imsg((*i).msg);
				if((dmsg == imsg)&&(d.senderID == (*i).target.id)){
					sendSet.erase(i);
					break;
				}
			}
			sendSet_mtx.unlock();				
		}
		return d;*/
	}
};
