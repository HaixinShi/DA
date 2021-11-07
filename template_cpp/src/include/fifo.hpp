#include "urb.hpp"
using namespace std;
struct fifoMsg
{
	string senderID;
	string fifomsg;
	string seq;
};
class fifo
{
public:
	int lsn = 0;
	set<string> pending;
	string log;
	mutex loglock;
	
	string myID;
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
	string serializeMsg(fifoMsg m){
		string msg = m.senderID + ","+ m.fifomsg + "," + m.seq;
		return msg;
	}
	fifoMsg deserializeMsg(string m){
		fifoMsg msg;
		size_t split = m.find(",");
		msg.senderID = m.substr(0, split);
		string temp = m.substr(split + 1, m.size()- 1 -split);
		split = temp.find(",");
		msg.fifomsg = temp.substr(0, split);
		msg.seq = temp.substr(split + 1, m.size()- 1 -split);
		return msg;
	}
	fifo(unsigned long myID, vector<myhost>* hosts, const char* output){
		this -> myID = to_string(myID);
		this -> hosts = hosts;
		this -> output = output;
		for(unsigned int i = 0; i < hosts->size();i++){
			next[to_string(i+1)] = 1;
		}
		urbPtr = new urb(myID, hosts, output);		
	}
	void fifoBroadcast(string msg){
		loglock.lock();
		log += "b " + msg + "\n";
		loglock.unlock();
		++lsn;
		fifoMsg m;
		m.senderID = myID;
		m.seq = to_string(lsn);
		m.fifomsg = msg;
		urbPtr -> urbBroadcast(serializeMsg(m));
	}
	void fifoDelibver(){
		while(!stopflag){
			string urbm = urbPtr->urbTrytoDeliver();
			if(urbm!=""){
				pending.insert(urbm);//senderID+m+seq
				fifoMsg fifom = deserializeMsg(urbm);//seq+m
				set<string>::iterator it=pending.begin();
				while(it != pending.end()){	
					
					fifoMsg temp = deserializeMsg(*it);
					
					if(fifom.senderID == temp.senderID){
						if(temp.seq == to_string(next[fifom.senderID])){
							next[fifom.senderID]++;
							pending.erase(it);
							it = pending.begin();
							loglock.lock();
							log += "d " + temp.senderID+" "+ temp.fifomsg +"\n";
							loglock.unlock();
						}
						else{
							++it;
						}
					}
					else{
						++it;
					}
					
				}
								
			}					
		}		
	}
	~fifo();
	
};