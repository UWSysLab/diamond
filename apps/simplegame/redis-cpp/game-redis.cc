#include <cstdlib>
#include <iostream>
#include <string>

#include <redox.hpp>

/*
 * Redis version of the 100 game.
 *
 * Note that in order to build this code, you must clone Redox from git into a
 * directory named 'redox' inside of the 'simplegame' directory and then
 * compile Redox.
 *
 * Also, in its current form, this game will crash if there are not already
 * variables named 'turn' and 'sum' initialized to 0 in your Redis instance.
 */

using namespace std;
using namespace redox;

vector<string> players;
int sum, turn;

int main(int argc, char **argv) {
   string cp, myname = string(argv[2]);
   Redox rdx; Subscriber sub;

   // Set up redis and pubsub connections
   if (!rdx.connect(string(argv[1]), 6379)) exit(1);
   if (!sub.connect(string(argv[1]), 6379)) exit(1);
   
   // generic callback, exit if command fails
   auto cb = [] (Command<int> &c) {
      if (!c.ok()) exit(1); };
   
   // Add user to the game
   rdx.command<int>({"ZADD", "players", "1", myname},
                    cb);
   rdx.publish("ping", "");
   
   // Set up our print outs
   sub.subscribe("ping",
    [&] (const string& topic, const string& msg) {
     Command<vector<string>>& r =
        rdx.commandSync<vector<string>>({"ZRANGE",
                 "players", "0", "-1"});
     players = r.reply();
     sum = stoi(rdx.get("sum"));
     turn = stoi(rdx.get("turn"));
     cout << "Current total: " << sum << "\n";
     cp = players[turn % players.size()];
     if (sum >= 100)
	cout << cp << " won! Game Over!\n";
     else if (cp == myname)
	cout << "Enter number between 1 and 10: \n";
     else
	cout << "It's " << cp << "'s turn. \n";
   });
   
   // Cycle on user input
   while (1) {
      int inc; cin >> inc;
      cp = players[turn % players.size()];
      if (cp != myname) {
         cout << "Not your turn!\n"; continue;
      }
      if (inc < 1 || inc > 10) {
         cout << "Incorrect input\n"; continue;
      }
      sum += inc; if (sum < 100) turn++;
      rdx.command<int>({"INCRBY", "sum",
	       to_string(inc)}, cb);
      rdx.command<int>({"INCR", "turn"}, cb);
      rdx.publish("ping", "");
   }
   rdx.disconnect(); sub.disconnect(); return 0;
}
