#include "urb.hpp"
using namespace std;
struct fifoMsg
{
	string seq;
	string fifomsg;
};
class fifo
{
private:
	int lsn = 0;
	set<string> pending;
	vector<int> next = temp(hosts->size(),1);
	string log;
	mutex loglock;
public:
	bool stop = false;
	urb* urbPtr;
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
		string msg = m.seq + "," + m.msg;
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
		while(!stop){
			urbMsg urbm = urbTrytoDeliver();
			if(m.body!=""){
				pending.insert(urbPtr->serializeMsg(urbm));
				fifoMsg fifom = deserializeMsg(urbm.body);
				for(set<string>::iterator it=pending.begin() ;it!=pending.end();it++){
					string msgVal = *it;
					urbMsg temp = urbPtr->deserializeMsg(msgVal);
					string senderID = temp.senderID
					string seq = deserializeMsg(temp.body).seq;
					if(next[stoi(senderID) - 1]==seq){
						next[stoi(senderID) - 1]++;
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
