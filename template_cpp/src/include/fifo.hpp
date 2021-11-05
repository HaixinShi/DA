#include "urb.hpp"
using namespace std;
struct fifoMsg
{
	string seq;
	string fifomsg;
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
		if(log.length()>0){
			log.erase(log.end()-1);
		}
		out << log;
		loglock.unlock();
		out.flush();
		out.close();
		cout << "I finish logging!"<< endl;
	}
	string serializeMsg(fifoMsg m){
		string msg = m.seq + "," + m.fifomsg;
		return msg;
	}
	fifoMsg deserializeMsg(string m){
		fifoMsg msg;
		size_t split = m.find(",");
		msg.seq = m.substr(0, split);
		msg.fifomsg = m.substr(split + 1, m.size()- 1 -split);
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
		m.seq = to_string(lsn);
		m.fifomsg = msg;
		urbPtr -> urbBroadcast(serializeMsg(m));
	}
	void fifoDelibver(){
		while(!stopflag){
			urbMsg urbm = urbPtr->urbTrytoDeliver();
			if(urbm.body!=""){
				cout << "fifoDelibver get urbm" << urbPtr->serializeMsg(urbm) << endl;
				pending.insert(urbPtr->serializeMsg(urbm));//senderID+seq+m
				fifoMsg fifom = deserializeMsg(urbm.body);//seq+m
				for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
					string msgVal = *it;
					cout << "fifo pending:"<< msgVal << endl;
					urbMsg temp = urbPtr->deserializeMsg(msgVal);//senderID+seq+m
					string senderID = temp.senderID;
					string seq = deserializeMsg(temp.body).seq;//candidate seq
					if(to_string(next[senderID])==seq){
						next[senderID]++;
						pending.erase(it);
						loglock.lock();
						log += "d " + senderID+" "+ deserializeMsg(temp.body).fifomsg +"\n";
						loglock.unlock();
					}
				}
								
			}

						
		}		
	}
	~fifo();
	
};