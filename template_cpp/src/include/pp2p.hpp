#include "sp2p.hpp"

using namespace std;

class pp2p : public sp2p{
private:
	vector<deliver> delivers;
public:
	pp2p(unsigned long myID, vector<myhost>* hosts, const char* output, const char* config, int timeout): sp2p(myID, hosts, output, config, timeout){
		
		ifstream configFile(this->config);
		string line;
		getline(configFile, line);

		istringstream iss(line);

		int m;
		int i;

		iss >> m >> i; 

		cout << "m: " << to_string(m) << endl;
		cout << "i: " << to_string(i) << endl;

		if((*hosts)[i].id == myID){
			// I am the receiver!
			cout << "receive thread init start" << endl;
			thread deliverthread(pp2pDeliver, this);
			cout << "receive thread init finish" << endl;
			deliverthread.join();			
		}
		else{
			cout << "pp2p sending thread init start" << endl;
			thread sendthread(pp2pSend, this, (*hosts)[i], m);
			cout << "pp2p sending thread init finish" << endl;	
			sendthread.join();		
		}
	}
	static void pp2pSend(pp2p* thiz, myhost target, int m){
		thiz->sp2pSend(thiz, target, m);
	}
	static void pp2pDeliver(pp2p* thiz){
		while(1){
			deliver d = thiz->sp2pDeliver(thiz);
			unsigned int i = 0;
			for(; i< thiz -> delivers.size(); i++){
				if(thiz -> delivers[i].senderID == d.senderID && thiz -> delivers[i].msg == d.msg){
					break;
				}
			}
			if(i == thiz -> delivers.size()){
				thiz -> delivers.push_back(d);
				string tag = "d ";
				string loginfo = tag + to_string(d.senderID) +" "+ d.msg;
				thiz->log << loginfo << endl;
			}
		}
		
	}

};