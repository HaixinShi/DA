#include <time.h>
#include "flp2p.hpp"
using namespace std;

class sp2p : public flp2p{
private:
	int timeout;
	long int start_time = 0;
	long int end_time = 0;
public:
	sp2p(unsigned long myID, vector<myhost>* hosts, const char* output, const char* config, int timeout){
		flp2p(myID, hosts, output, config);
		this->timeout = timeout;
	}
	~sp2p(){
		~flp2p();
	}
	static void sp2pSend(sp2p* thiz){
		thiz->start_time = clock();
		while(1){
			thiz->end_time = clock();
			if(thiz->end_time - thiz->start_time > thiz->timeout){
				thiz->flp2pSend(thiz);
			}
		}
	}
	static void sp2pDeliver(sp2p* thiz, myhost host){
		thiz->flp2pDeliver(thiz, host);
	}
};
