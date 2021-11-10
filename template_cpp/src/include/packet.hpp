#include <string>
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
		//cout << endl;
		//cout << to_string(msg)<<endl;
		//cout << to_string(seq)<<endl;
		return to_string(msg) + to_string(seq);
	}
	bool operator<(const class fifoPacket & right)const{
		return this->seq < right.seq;
	}
};

class urbPacket{//9byte
public:
	uint8_t originalSenderID = 0;
	fifoPacket fifomsg;
	string getTag(){
		//cout << endl;
		//int ID = originalSenderID;
		//cout << to_string(ID-48) << endl;
		//return to_string(ID-48) + fifomsg.getTag();
		return getID(originalSenderID) + fifomsg.getTag();
	}	
};
