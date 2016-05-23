#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <includes/data_types.h>
#include "benchmark_common.h"

/*
 * Client that measures latency of read-only transactions with and without client-side caching and local commit
 */

namespace po = boost::program_options;
using namespace diamond;

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string keyFile;
    int nKeys = 0;
    int numSeconds = 10;
    bool readOnly = false;
    bool printKeys = false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("keys", po::value<std::string>(&keyFile)->required(), "file from which to read keys (required)")
        ("numkeys", po::value<int>(&nKeys)->required(), "number of keys to read (required)")
        ("readonly", po::bool_switch(&readOnly), "run transactions in read-only mode")
        ("time", po::value<int>(&numSeconds), "number of seconds to run (default 10)")
        ("printkeys", po::bool_switch(&printKeys), "print keys for each transaction (default false)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("config") || !vm.count("keys") || !vm.count("numkeys")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    initRand();

    DObject::SetLinearizable();
    DObject::SetCaching(readOnly); // we only want to use caching if in read-only mode

    std::vector<std::string> keys;
    parseKeys(keyFile, nKeys, keys);

    std::vector<DString> dstrings;
    for (auto &key : keys) {
        DString dstring;
        DObject::Map(dstring, key);
        dstrings.push_back(dstring);
    }

    std::cout << "start-time\tend-time\tcommitted\tread-only\t";
    if (printKeys) {
        std::cout << "key-1\tkey-2\t";
    }
    std::cout << std::endl;

    int varIndex1 = randInt(0, dstrings.size() - 1);
    int varIndex2 = randInt(0, dstrings.size() - 1);

    for (int i = 0; i < numSeconds; i++) {
        uint64_t startTime = currentTimeMillis();
        if (readOnly) {
            DObject::BeginRO();
        }
        else {
            DObject::TransactionBegin();
        }
        std::string temp1 = dstrings[varIndex1].Value();
        std::string temp2 = dstrings[varIndex2].Value();
        int committed = DObject::TransactionCommit();
        uint64_t endTime = currentTimeMillis();

        std::cout << startTime << "\t"
                  << endTime << "\t"
                  << committed << "\t"
                  << readOnly << "\t";
        if (printKeys) {
            std::cout << keys[varIndex1] << "\t"
                      << keys[varIndex2] << "\t";
        }
        std::cout << std::endl;

        sleep(1);
    }
}
