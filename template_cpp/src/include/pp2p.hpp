#include "sp2p.hpp"

using namespace std;

class pp2p : public sp2p{
private:
	vector<deliver> delivers;
public:
	pp2p(unsigned long myID, vector<myhost>* hosts, const char* output, int timeout): sp2p(myID, hosts, output, timeout){

	}
	static void pp2pSend(pp2p* thiz, myhost target, int m){
		thiz->sp2pSend(thiz, target, m);
		cout <<"pp2pSend finish!"<< endl;
	}
	static void pp2pDeliver(pp2p* thiz){
		while(!thiz -> stop){
			deliver d = thiz->sp2pDeliver(thiz);
			unsigned int i = 0;
			for(; i< thiz -> delivers.size(); i++){
				if(thiz -> delivers[i].senderID == d.senderID && thiz -> delivers[i].msg == d.msg){
					break;
				}
			}
			if(i == thiz -> delivers.size()&&!thiz -> stop){
				thiz -> delivers.push_back(d);
				thiz -> log += "d " + to_string(d.senderID) +" "+ d.msg +"\n";
				thiz -> count++;
			}
		}
		cout << "pp2pDeliver finish!"<< endl;
		
	}

};