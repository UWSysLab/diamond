#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <includes/data_types.h>

namespace po = boost::program_options;
using namespace diamond;

int main(int argc, char ** argv) {
    std::string configPrefix;
    int numItems = 0;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("write", po::value<int>(&numItems), "number of items to write to set")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);

    DStringSet set;
    DObject::Map(set, "test:set");

    if (numItems > 0) {
        int committed = 0;
        while (!committed) {
            DObject::TransactionBegin();
            for (int i = 0; i < numItems; i++) {
                set.Add(std::to_string(rand()));
            }
            committed = DObject::TransactionCommit();
        }
    }
    else {
        int committed = 0;
        while (!committed) {
            DObject::TransactionBegin();
            std::unordered_set<std::string> members = set.Members();
            std::cout << "BEGIN SET: |";
            for (auto it = members.begin(); it != members.end(); it++) {
                std::cout << *it << " " << std::endl;
            }
            std::cout << "| END SET" << std::endl;
            committed = DObject::TransactionCommit();
            std::cout << " Committed? " << committed << std::endl;
        }
    }
}
