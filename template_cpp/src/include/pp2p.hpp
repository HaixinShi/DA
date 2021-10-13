#include "sp2p.hpp"

using namespace std;

class pp2p : public sp2p{
private:
	vector<deliver> delivers;
public:
	pp2p(unsigned long myID, vector<myhost>* hosts, const char* output, const char* config, int timeout): sp2p(myID, hosts, output, config, timeout){
		cout << "pp2p sending thread init start" << endl;
		thread sendthread(pp2pSend, this);
		cout << "pp2p sending thread init finish" << endl;
		cout << "receive thread init start" << endl;
		thread deliverthread(pp2pDeliver, this);
		cout << "receive thread init finish" << endl;
		//join the threads
		sendthread.join();
		deliverthread.join();
	}
	static void pp2pSend(pp2p* thiz){
		thiz->sp2pSend(thiz);
	}
	static void pp2pDeliver(pp2p* thiz){
		thiz->sp2pDeliver(thiz);
	}

};