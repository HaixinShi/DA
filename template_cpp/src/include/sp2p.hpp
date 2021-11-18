#include "SafeQueue.hpp"
#include "flp2p.hpp"
using namespace std;
class task{
public:
	unsigned int target;
	urbPacket urbmsg;
};

class sp2p : public flp2p{
public:
	//mutex taskQueue_mtx;
	SafeQueue<task> taskQueue;//total queue
	queue<multitask> retransmitQueue;
	vector<queue<urbPacket>> processQueues;
	sp2p(uint8_t myID, vector<myhost>* hosts, const char* output): flp2p(myID, hosts, output){
		for(unsigned int i = 0; i < hosts->size(); i++){
			queue<urbPacket> temp;
			processQueues.push_back(temp);
		}
	}
	void sp2pSend(){
		while(!stopflag){
			//taskQueue_mtx.lock();
			if(!taskQueue.empty()){
				//cout << "taskQueue is not empty-------------" << endl;
				task t;//= taskQueue.front();
				taskQueue.move_pop(t);
				processQueues[t.target - 1].push(t.urbmsg);
				//cout<< to_string(t.target - 1)<<"--sp2pSend:" << to_string(processQueues[t.target - 1].size()) << endl;
				if(processQueues[t.target - 1].size() == maxSize){
					flp2pSend(t.target, processQueues[t.target - 1]);
					
					//empty processQueues[t.target.id - 1] and push them into retransmitQueue
					multitask mt;
					mt.target = t.target;
					mt.urbmsgs.swap(processQueues[t.target - 1]);
					//swap(&mt.urbmsgs, &processQueues[t.target - 1]);
					retransmitQueue.push(mt);
				}							
			}
			else{
				//cout << "taskQueue is empty-------------" << endl;
				unsigned int i =0;
				for(; i < hosts.size(); i++){
					if(!processQueues[i].empty()){
						flp2pSend(i + 1 , processQueues[i]);	
						multitask mt;
						mt.target = i+1;
						//swap(mt.urbmsgs, processQueues[i]);
						mt.urbmsgs.swap(processQueues[i]);
						retransmitQueue.push(mt);					
					}
				}
				if(!retransmitQueue.empty()){
					//cout << "start retransmit-------------" << endl;
					multitask mt = retransmitQueue.front();
					retransmitQueue.pop();
					ack_mtx.lock();				
					if(!ack.count(mt.getTag())){
						//cout << "--------sp2pSend:" << 	getID(t.target.id) + t.urbmsg.getTag()<< endl;
						retransmitQueue.push(mt);
						flp2pSend(mt.target, mt.urbmsgs);
					}
					ack_mtx.unlock();
				}
			}
		}	
	}
	
	deliver sp2pDeliver(){
		deliver d;
		if(!pending.empty()){
			//= pending.front();
			pending.move_pop(d);
			//cout << "------------sp2p deliver:----" << endl;
			//cout << getID(d.realSenderID) << endl;
			//cout << to_string(d.urbmsg.fifomsg.msg) << endl;
			return d;
		}
		d.realSenderID = 0;
		return d;
	}
};
