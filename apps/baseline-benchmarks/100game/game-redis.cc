#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <condition_variable>
#include <mutex>

#include "benchmark_common.h"

#include <redox.hpp>

using namespace std;
using namespace redox;
namespace po = boost::program_options;

std::mutex m;
std::condition_variable cv;
bool notificationReceived = true;
bool done = false;
bool twoPlayers = false;
uint64_t prevTurnTime;

int main(int argc, char ** argv) {
   std::string host;
   int port;
   std::string myName;
   std::string keyPrefix;

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help", "produce help message")
       ("name", po::value<std::string>(&myName)->required(), "name to use in the game (required)")
       ("host", po::value<std::string>(&host)->required(), "Redis host (required)")
       ("port", po::value<int>(&port)->required(), "Redis port (required)")
       ("keyprefix", po::value<std::string>(&keyPrefix)->required(), "key prefix (required)");
       
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("host") || !vm.count("port") || !vm.count("keyprefix")) {
       std::cout << desc << std::endl;
       return 1;
    }
    po::notify(vm);

    vector<string> players;
    int sum, turn;

    string cp;
    Redox rdx; Subscriber sub;

    auto cb = [](Command<int>& c) {
       if(!c.ok()) {
          exit(1);
       }
    };
    
   // Set up redis and pubsub connections
   if (!rdx.connect(host, port)) exit(1);
   if (!sub.connect(host, port)) exit(1);

   // Set up our print outs
   sub.subscribe(keyPrefix + ":ping",
                 [&] (const string& topic, const string& msg) {
                    if (msg == "players") {
                       Command<vector<string>>& r =
                          rdx.commandSync<vector<string>>({"ZRANGE",
                                   keyPrefix+":players", "0", "-1"});
                       if (r.ok()) {
                          players = r.reply();
                       } else {
                          exit(1);
                       }
                    }

                    int numPlayers = players.size();
                    Command<vector<string>>& r =
                       rdx.commandSync<vector<string>>({"MGET",
                                keyPrefix+":sum",
                                keyPrefix+":turn"});
                    if (r.ok()) {
                       string sumStr = r.reply()[0];
                       if (sumStr.size() == 0) {
                          sumStr = "0";
                       }
                       string turnStr = r.reply()[1];
                       if (turnStr.size() == 0) {
                          turnStr = "0";
                       }
                       sum = stoi(sumStr);
                       turn = stoi(turnStr);
                    } else {
                       exit(1);
                    }

                    if (numPlayers > 0) {
                       string cp = players[turn % numPlayers];
                       if (sum >= 100) {
                          //cout << cp << " won! Game Over!" << endl;
                          {
                             std::unique_lock<std::mutex> lock(m);
                             notificationReceived = true;
                             done = true;
                             cv.notify_all();
                          }
                       } else if (cp == myName) {
                          //cout << " Enter number between 1 and 10: " << endl;
                          {
                             std::unique_lock<std::mutex> lock(m);
                             notificationReceived = true;
                             if (numPlayers >= 2) {
                                twoPlayers = true;
                             }
                             cv.notify_all();
                          }
                       }
                    }
                 });
   
    sleep(1); // race condition: client will miss its own update if the subscribe
              // above happens after the publish below

    rdx.command<int>({"ZADD", keyPrefix+":players", "1", myName}, cb);
    rdx.publish(keyPrefix + ":ping", "players");
 
    // Cycle on user input
    prevTurnTime = currentTimeMillis();
    while (1) {
       while (!notificationReceived) {
          std::unique_lock<std::mutex> lock(m);
          cv.wait(lock);
       }
       notificationReceived = false;
       if (done) {
          break;
       }
       if (!twoPlayers) {
          continue;
       }
        //cout << "Got a signal from the CV" << endl;

       int inc = 1;
       string cp = players[turn % players.size()];
       // If it's the user's turn, make move
       if (cp == myName && inc >= 1 && inc <= 10) {
          //cout << "It's my turn" << endl;
          rdx.command<int>({"INCRBY", keyPrefix+":sum",
                   to_string(inc)}, cb);
	  
          if (sum < 100) {
             rdx.command<int>({"INCR", keyPrefix+":turn"}, cb);
          }
          rdx.publish(keyPrefix+":ping", "move");
       }
       uint64_t turnTime = currentTimeMillis();
        
       std::cout << prevTurnTime << "\t" << turnTime << std::endl;

       prevTurnTime = turnTime;
    }
    return 0;
}
