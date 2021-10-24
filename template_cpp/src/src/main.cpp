#include <chrono>
#include <iostream>
#include <thread>
#include "parser.hpp"
#include "hello.h"
#include <signal.h>
#include "pp2p.hpp"

pp2p* pl;

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
  pl -> stop = true;
  close(pl -> s);
  
  // write/flush output file if necessary 
  std::cout << "Writing output.\n";
  pl -> logfuction();
  
  // exit directly from signal handler
  exit(0);
}
static void startSendingTask (myhost target, int m){
    //it could be run a thread. 
    //This function is deviced for not blocking main thread early
    int send_seq = 1;
    for(int j = 0; j < m; j++){
      pl -> pp2pSend(target, to_string(send_seq));
      send_seq ++;    
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
  
  pl = new pp2p(parser.id(), &myhosts, parser.outputPath());
  
  ifstream configFile(parser.configPath());
  string line;
  getline(configFile, line);

  istringstream iss(line);

  int m;
  int i;

  iss >> m >> i; 

  cout << "m: " << to_string(m) << endl;
  cout << "i: " << to_string(i) << endl;
  
  std::cout << "Broadcasting and delivering messages...\n\n";

  if(myhosts[i-1].id != parser.id()){
    //this part is for senders
    thread start(startSendingTask, myhosts[i-1], m);
    pl -> startPerfectLink();
    start.join();    
  }    
  else{
    //this part is for the receiver
    pl -> startPerfectLink();
  }

  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
