#include <time.h>
#include "flp2p.hpp"
using namespace std;
#define random_timeout(a,b) ((rand()%(b-a+1))+a)
class sp2p : public flp2p{
private:
	int timeout;
	long int start_time = 0;
	long int end_time = 0;
public:
	sp2p(unsigned long myID, vector<myhost>* hosts, const char* output, int timeout): flp2p(myID, hosts, output){
		this->timeout = timeout;
	}
	static void sp2pSend(sp2p* thiz, myhost target, int m){
		bool retransmit = false; 
		thiz->flp2pSend(thiz, target, m, retransmit);
		retransmit = true;
		thiz->start_time = clock();
		//srand(static_cast<int>(thiz->start_time));
		while(!thiz->stop){
			//thiz-> timeout = random_timeout(0,3);
			thiz->end_time = clock();
			if((thiz->end_time - thiz->start_time)/CLOCKS_PER_SEC > thiz->timeout){
				//microseconds
				cout << "send again!" << endl;
				thiz->flp2pSend(thiz, target, m, retransmit);
				thiz->start_time = clock();
			}
		}
		
	}
	static deliver sp2pDeliver(sp2p* thiz){
		return thiz->flp2pDeliver(thiz);
	}
};
