#include "urb.hpp"	
using namespace std;
class fifo
{
public:
	int lsn = 0;

	map<uint8_t, set<fifoPacket>> pending;
	string log;
	mutex loglock;
	
	uint8_t myID;
	vector<myhost>* hosts;
	map<string,int> next;
	bool stopflag = false;
	const char* output;
	
	urb* urbPtr;

	void startfifo(){
		thread fifoDelibverThread(&fifo::fifoDelibver, this);
		urbPtr -> starturb();
		fifoDelibverThread.join();
	}
	void logfunction(){
		ofstream out;
		out.open(this->output);
		loglock.lock();
		out << log;
		loglock.unlock();
		out.flush();
		out.close();
		cout << "I finish logging!"<< endl;
	}
	fifo(uint8_t myID, vector<myhost>* hosts, const char* output){
		this -> myID = myID;
		this -> hosts = hosts;
		this -> output = output;
		for(unsigned int i = 0; i < hosts->size();i++){
			next[to_string(i+1)] = 1;
		}
		urbPtr = new urb(myID, hosts, output);		
	}
	void fifoBroadcast(int msg){
		loglock.lock();
		log += "b " + to_string(msg) + "\n";
		loglock.unlock();
		++lsn;
		fifoPacket f;
		f.seq = lsn;
		f.msg = msg;
		urbPtr -> urbBroadcast(f);
	}
	void fifoDelibver(){
		while(!stopflag){
			urbPacket u = urbPtr->urbTrytoDeliver();
			if(u.originalSenderID != 0){				
				if(pending.count(u.originalSenderID)){
					pending[u.originalSenderID].insert(u.fifomsg);
				}
				else{
					set<fifoPacket> temp;
					temp.insert(u.fifomsg);
					pending[u.originalSenderID] = temp;
				}
				set<fifoPacket>::iterator it = pending[u.originalSenderID].begin();
				while(it != pending[u.originalSenderID].end()){
					fifoPacket temp = (*it);
					string originalSenderID = getID(u.originalSenderID);
					if(temp.seq == next[originalSenderID]){
							loglock.lock();
							log += "d " + originalSenderID +" "+ to_string(temp.msg) +"\n";
							//cout << "d " + originalSenderID +" "+ to_string(temp.msg) +"\n";
							loglock.unlock();
							next[originalSenderID]++;
							pending[u.originalSenderID].erase(it);
							it = pending[u.originalSenderID].begin();						
					}
					else{
						break;
					}
				}			
			}					
		}		
	}
	~fifo();
	
};