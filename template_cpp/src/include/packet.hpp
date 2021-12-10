#include <string>
#include <vector>
using namespace std;
string getID(uint8_t id);
string getID(uint8_t id){
	int temp = id;
	return to_string(temp);
}
class fifoPacket{//8byte
public:
	int msg = 0;
	int seq = 0;
	string getTag(){
		return to_string(msg) + to_string(seq);
	}
	bool operator<(const class fifoPacket & right)const{
		return this->seq < right.seq;
	}
};
class lcbPacket{//4*(hosts.size()+1)+1
public:
	int msg = 0;
	vector<int> V;
	string getTag(){
		/*
		string ret = ":";
		ret += to_string(msg) + "[";
		for(unsigned int i = 0; i < V.size(); i++){
			ret += to_string(V[i])+","; 
		}
		ret += "]";*/
		string ret = "";
		for(unsigned int i = 0; i < V.size(); i++){
			ret += to_string(V[i]); 
		}
		return ret;
	}
};

class urbPacket{
public:
	uint8_t originalSenderID = 0;
	lcbPacket lcbmsg;
	string getTag(){
		return getID(originalSenderID) + lcbmsg.getTag();
	}	
};
