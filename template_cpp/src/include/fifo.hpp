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
	int test =0;
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
		cout << "serializeMsg" <<endl;
		string msg = m.senderID + ","+ m.fifomsg + "," + m.seq;
		return msg;
	}
	fifoMsg deserializeMsg(string m){
		cout << "deserializeMsg-1" <<endl;
		fifoMsg msg;
		size_t split = m.find(",");
		msg.senderID = m.substr(0, split);
		string temp = m.substr(split + 1, m.size()- 1 -split);
		split = temp.find(",");
		msg.fifomsg = temp.substr(0, split);
		msg.seq = temp.substr(split + 1, m.size()- 1 -split);
		cout << "deserializeMsg-2" <<endl;	
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
			if(test == 20){
				cout<< "I finish!!!" <<endl;
			}
			string urbm = urbPtr->urbTrytoDeliver();
			if(urbm!=""){
				cout << "fifoDeliver:"<< urbm << endl;
				pending.insert(urbm);//senderID+m+seq
				fifoMsg fifom = deserializeMsg(urbm);//seq+m
				set<string>::iterator it=pending.begin();
				while(it != pending.end()){	
					
					cout <<"fifoDelibver-3-"<<(*it)<<endl;
					
					fifoMsg temp = deserializeMsg(*it);
					
					if(fifom.senderID == temp.senderID){
						cout <<"fifoDelibver-4:"<< fifom.seq <<endl;
						cout <<"fifoDelibver-4:"<< to_string(next[fifom.senderID])<<endl;
						if(temp.seq == to_string(next[fifom.senderID])){
							cout <<"fifoDelibver-6"<< endl;
							next[fifom.senderID]++;
							cout <<"fifoDelibver-7"<< endl;
							pending.erase(it);
							it = pending.begin();
							cout <<"fifoDelibver-8"<< endl;
							loglock.lock();
							log += "d " + temp.senderID+" "+ temp.fifomsg +"\n";
							loglock.unlock();
							cout << "d " + temp.senderID+" "+ temp.fifomsg +"\n";
							test++;
							cout << "pending_size ="<<pending.size()<<endl;
						}
						else{
							++it;
						}
					}
					else{
						++it;
					}
					
				}
				/*
				for(unsigned int i = 0; i < hosts ->size(); i++){
					set<string>::iterator it=pending.begin();
					while(it != pending.end()){
						fifoMsg temp = deserializeMsg(*it);
						if(temp.seq == to_string(next[to_string(i+1)])){
							cout <<"fifoDelibver-6"<< endl;
							next[to_string(i+1)]++;
							cout <<"fifoDelibver-7"<< endl;
							pending.erase(it++);
							cout <<"fifoDelibver-8"<< endl;
							loglock.lock();
							log += "d " + temp.senderID+" "+ temp.fifomsg +"\n";
							loglock.unlock();
							cout << "d " + temp.senderID+" "+ temp.fifomsg +"\n";
							test++;
							cout << "pending_size ="<<pending.size()<<endl;
						}
						else{
							++it;
						}						
					}
				}*/
				cout << "fifo see you!"<<endl;
								
			}					
		}		
	}
	~fifo();
	
};