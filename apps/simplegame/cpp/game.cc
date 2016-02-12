#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <includes/data_types.h>
#include <includes/transactions.h>

namespace po = boost::program_options;
using namespace diamond;

void myVariadicFunction(va_list args) {
    int * num = va_arg(args, int *);
    std::cout << "We are Diamond Dogs: " << *num << std::endl;
    *num = 3;
}

void myFunction(const std::string & str) {
    std::cout << "I don't know what I'm doing, but I got a string argument: " << str << std::endl;
}

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string name;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("name", po::value<std::string>(&name)->required(), "name to use in the game")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);

    int num = 2;
    std::string str("lambdas are cool");
    execute_txn([num] () { std::cout << "Lambda time: " << num << std::endl; } );

    while(true) {
    }
}
