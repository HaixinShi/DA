#include <chrono>
#include <iostream>
#include <thread>
#include "parser.hpp"
#include "hello.h"
#include <signal.h>
#include "pp2p.hpp"

pp2p* pl;
thread* deliverthread;
thread* sendthread;
static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
  pl -> stop =true; 
  while(!(pl->pp2p_send_stop && pl->pp2p_recv_stop)){
    std::cout <<"wait..."<<endl;
  }
  
  // write/flush output file if necessary 
  std::cout << "Writing output.\n";
  if(pl){
    delete pl;
  }
  std::cout << "exit.\n";
  // exit directly from signal handler
  exit(0);
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
  std::cout << std::endl;

  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";

  std::cout << "My ID: " << parser.id() << "\n\n";

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
    temp.id = host.id;
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
  
  pl = new pp2p(parser.id(), &myhosts, parser.outputPath(), 3);
  
  ifstream configFile(parser.configPath());
  string line;
  getline(configFile, line);

  istringstream iss(line);

  int m;
  int i;

  iss >> m >> i; 

  cout << "m: " << to_string(m) << endl;
  cout << "i: " << to_string(i) << endl;
  
  if(myhosts[i].id == parser.id()){
    // I am the receiver!
    cout << "receive thread init start" << endl;
    deliverthread = new thread(pl->pp2pDeliver, pl);
    cout << "receive thread init finish" << endl;
    deliverthread -> join();     
  }
  else{
    cout << "pp2p sending thread init start" << endl;
    sendthread = new thread(pl -> pp2pSend, pl, myhosts[i], m);
    cout << "pp2p sending thread init finish" << endl;  
    sendthread -> join();    
  }
  
  std::cout << "Broadcasting and delivering messages...\n\n";

  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
