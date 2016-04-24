#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <includes/data_types.h>
#include "benchmark_common.h"

namespace po = boost::program_options;
using namespace diamond;

int main(int argc, char ** argv) {
    std::string configPrefix;
    int numSeconds = 10;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("time", po::value<int>(&numSeconds), "number of seconds to run (default 10)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("config")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    initRand();

    std::string key1 = getRandomKey();
    std::string key2 = getRandomKey();
    std::string key3 = getRandomKey();
    DString str1;
    DString str2;
    DString str3;
    DObject::Map(str1, key1);
    DObject::Map(str2, key2);
    DObject::Map(str3, key3);

    bool done = false;
    uint64_t globalStartTime = currentTimeMillis();

    while (!done) {
        DObject::TransactionBegin();
        std::string temp1 = str1.Value();
        std::string temp2 = str2.Value();
        std::string temp3 = str3.Value();
        int committed = DObject::TransactionCommit();

        std::cout << "GET" << std::endl;

        uint64_t currentTime = currentTimeMillis();

        double runtimeSeconds = (currentTime - globalStartTime) / 1000.0;
        done = (runtimeSeconds >= numSeconds);
    }
}
