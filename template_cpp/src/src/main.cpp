#include <chrono>
#include <iostream>
#include <thread>
#include "parser.hpp"
#include "hello.h"
#include <signal.h>
#include "lcb.hpp"

lcb* lcbPtr;

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  
  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
  lcbPtr -> stopflag = true;
  lcbPtr -> urbPtr -> stopflag = true;
  lcbPtr -> urbPtr -> pl -> stopflag = true;
  close(lcbPtr ->urbPtr -> pl -> s);
  
  // write/flush output file if necessary 
  std::cout << "Writing output.\n";
  lcbPtr -> logfunction();
  // exit directly from signal handler*/
  exit(0);
}

static void callpp2p(deliver d){
  lcbPtr -> urbPtr -> pl -> pp2pDeliver(d);    
}

static void callurb(deliver d){
  lcbPtr -> urbPtr -> urbDeliver(d);
}

static void calllcb(urbPacket u){
  lcbPtr -> lcbDelibver(u);
}
static void SendingTask (int m){
    //it could run a thread. 
    //This function is deviced for not blocking main thread early
  int arbitaryMsg = 1;
  for(int j = 0; j < m; j++){
    lcbPtr -> lcbBroadcast(arbitaryMsg);
    arbitaryMsg ++;    
  } 
}
int main(int argc, char **argv) {
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;

  Parser parser(argc, argv);
  parser.parse();

  hello();
  uint8_t MyID = 0;
  unsigned long MyID_return = parser.id();
  memcpy(&MyID, &MyID_return, sizeof(uint8_t));
  int MyID_int = MyID;
  std::cout << std::endl;
  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";
  std::cout << "My ID: " << to_string(MyID_int)<< "\n\n";

  std::cout << "List of resolved hosts is:\n";
  std::cout << "==========================\n";
  auto hosts = parser.hosts();

  std::vector<myhost> myhosts;

  for (auto &host : hosts) {
    std::cout << host.id << "\n";
    std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
    std::cout << "Machine-readable IP: " << host.ip << "\n";
    std::cout << "Human-readbale Port: " << host.portReadable() << "\n";
    std::cout << "Machine-readbale Port: " << host.port << "\n";
    std::cout << "\n";

    struct myhost temp;
    memcpy(&temp.id, &host.id,sizeof(uint8_t));
    temp.ip = host.ip;
    temp.port = host.port;
    myhosts.push_back(temp);
  }
  std::cout << "\n";

  std::cout << "Path to output:\n";
  std::cout << "===============\n";
  std::cout << parser.outputPath() << "\n\n";

  std::cout << "Path to config:\n";
  std::cout << "===============\n";
  std::cout << parser.configPath() << "\n\n";

  std::cout << "Doing some initialization...\n\n";
  


  ifstream configFile(parser.configPath());
  


  string line;
  getline(configFile, line);
  istringstream iss(line);

  int m;

  map<int, vector<int>> neighbor;
  iss >> m; 
  while(getline(configFile, line)){
    istringstream iss(line);
    int x;
    int i = 0;
    int key;
    vector<int> val;
    while(iss >> x){
      if(i == 0){
        key = x;
      }
      val.push_back(x);
      i++;
    }
    neighbor[key] = val;
  }
  cout << "m: " << to_string(m) << endl;

  lcb lcbObj(MyID, &myhosts, parser.outputPath(), neighbor);

  lcbPtr = &lcbObj;

  lcbPtr -> urbPtr -> calllcb = &calllcb;

  lcbPtr -> urbPtr -> pl -> callurb = &callurb;

  lcbPtr -> urbPtr -> pl -> callpp2p = &callpp2p;
  
  std::cout << "Broadcasting and delivering messages...\n\n";
  //SendingTask(m);
  thread sp2pSendThread(&sp2p::sp2pSend, lcbPtr -> urbPtr -> pl);
  thread udpReceivethread(&flp2p::UDPReceive, lcbPtr -> urbPtr -> pl);
  thread TaskThread(&SendingTask, m);
  thread urbTrytoDeliverThread(&urb::urbTrytoDeliver, lcbPtr -> urbPtr);
  sp2pSendThread.join();
  udpReceivethread.join();
  TaskThread.join();
  urbTrytoDeliverThread.join();
  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.*/
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
