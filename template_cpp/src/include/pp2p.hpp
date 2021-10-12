#include "sp2p.hpp"

using namespace std;

class pp2p : public sp2p{
private:
	vector<deliver> delivers;
public:
	pp2p(unsigned long myID, vector<myhost>* hosts, const char* output, const char* config, int timeout){
		sp2p(myID, hosts, output, config, timeout);
	}
	~pp2p(){
		~sp2p();
	}
	static void pp2pSend(pp2p* thiz){
		thiz->sp2pSend(thiz);
	}
	static void pp2pDeliver(pp2p* thiz, myhost host){
		thiz->sp2pDeliver(thiz, host);
	}

};