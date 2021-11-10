#include <chrono>
#include <iostream>
#include <thread>
#include "parser.hpp"
#include "hello.h"
#include <signal.h>
#include "fifo.hpp"

fifo* fifoPtr;
static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
  fifoPtr -> stopflag = true;
  fifoPtr -> urbPtr -> stopflag = true;
  fifoPtr -> urbPtr -> pl -> stopflag = true;
  close(fifoPtr ->urbPtr -> pl -> s);
  
  // write/flush output file if necessary 
  std::cout << "Writing output.\n";
  fifoPtr -> logfunction();
  // exit directly from signal handler
  exit(0);
}

static void startSendingTask (int m){
    //it could run a thread. 
    //This function is deviced for not blocking main thread early
    int arbitaryMsg = 1;
    for(int j = 0; j < m; j++){
      fifoPtr -> fifoBroadcast(arbitaryMsg);
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
  int test = MyID;
  std::cout << std::endl;
  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";
  std::cout << "My ID: " << to_string(test)<< "\n\n";

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
  
  fifoPtr = new fifo(MyID, &myhosts, parser.outputPath());
  
  ifstream configFile(parser.configPath());
  string line;
  getline(configFile, line);

  istringstream iss(line);

  int m;

  iss >> m; 

  cout << "m: " << to_string(m) << endl;

  std::cout << "Broadcasting and delivering messages...\n\n";
  thread start(startSendingTask, m);
  fifoPtr -> startfifo();
  start.join();
  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
