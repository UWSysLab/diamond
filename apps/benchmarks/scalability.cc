#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <sys/time.h>

#include <includes/data_types.h>

namespace po = boost::program_options;
using namespace diamond;

void parseKeys(const std::string &keyFile, int nKeys, std::vector<std::string> &keys) {
    std::string key;
    std::ifstream in;
    in.open(keyFile.c_str());
    if (!in) {
        fprintf(stderr, "Could not read keys from: %s\n", keyFile.c_str());
        exit(0);
    }

    for (unsigned int i = 0; i < nKeys; i++) {
        std::getline(in, key);
        keys.push_back(key);
    }
    in.close();
}

uint64_t getMilliseconds(const struct timeval &time) {
    return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

std::string getRandomKey() {
    int range = 10 + 26 + 26; // digits + uppercase + lowercase
    char keyChars[65];
    for (int i = 0; i < 64; i++) {
        int index = rand() % range;
        int finalVal = -1;
        if (index < 10) {
            finalVal = index + 48;
        }
        else if (index >= 10 && index < 36) {
            finalVal = index - 10 + 65;
        }
        else { // index >= 36
            finalVal = index - 36 + 97;
        }

        keyChars[i] = finalVal;
    }
    keyChars[64] = '\0';
    return std::string(keyChars);
}

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string keyFile;
    int nKeys = 0;
    int numSeconds = 10;
    bool printKeys = false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("keys", po::value<std::string>(&keyFile)->required(), "file from which to read keys (required)")
        ("numkeys", po::value<int>(&nKeys)->required(), "number of keys to read (required)")
        ("time", po::value<int>(&numSeconds), "number of seconds to run (default 10)")
        ("printkeys", po::bool_switch(&printKeys), "number of seconds to run (default 10)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("config") || !vm.count("keys") || !vm.count("numkeys")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    srand(time(NULL));

    std::vector<std::string> keys;
    parseKeys(keyFile, nKeys, keys);

    std::vector<DString> dstrings;
    for (auto &key : keys) {
        DString dstring;
        DObject::Map(dstring, key);
        dstrings.push_back(dstring);
    }

    std::string uniqueKey = getRandomKey();
    DString uniqueString;
    DObject::Map(uniqueString, uniqueKey);

    bool done = false;
    struct timeval globalStartTime;
    gettimeofday(&globalStartTime, NULL);

    std::cout << "start-time\tend-time\tcommitted\t";
    if (printKeys) {
        std::cout << "read-key\twrite-key\t";
    }
    std::cout << std::endl;

    while (!done) {
        std::string val(std::to_string(rand()));
        int varIndex = rand() % dstrings.size();

        struct timeval startTime;
        gettimeofday(&startTime, NULL);

        // Read from a (randomly chosen) known key and write to the client's unique key
        DObject::TransactionBegin();
        std::string temp = dstrings[varIndex].Value();
        uniqueString.Set(val);
        int committed = DObject::TransactionCommit();

        struct timeval endTime;
        gettimeofday(&endTime, NULL);

        std::cout << getMilliseconds(startTime) << "\t"
                  << getMilliseconds(endTime) << "\t"
                  << committed << "\t";
        if (printKeys) {
            std::cout << keys[varIndex] << "\t"
                      << uniqueKey << "\t";
        }
        std::cout << std::endl;

        double runtimeSeconds = difftime(endTime.tv_sec, globalStartTime.tv_sec);
        done = (runtimeSeconds >= numSeconds);
    }
}
