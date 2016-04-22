#include <boost/program_options.hpp>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include <includes/data_types.h>
#include <includes/transactions.h>
#include "benchmark_common.h"

/*
 * Client that measures latency of reactive transactions with and without client-side caching
 */

namespace po = boost::program_options;
using namespace diamond;

DString dstr;
std::mutex m;
std::condition_variable cv;
bool receivedNotification = false;

int main(int argc, char ** argv) {
    std::string configPrefix;
    int numSeconds = 10;
    bool caching = false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("caching", po::bool_switch(&caching), "enable caching (disabled by default)")
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
    StartTxnManager();
    initRand();
    DObject::SetCaching(caching);

    std::string key = getRandomKey();
    DObject::Map(dstr, key);

    uint64_t reactive_id = reactive_txn([] () {
        std::string temp = dstr.Value();
        std::unique_lock<std::mutex> lock(m);
        receivedNotification = true;
        cv.notify_all();
    });

    bool done = false;
    uint64_t globalStartTime = currentTimeMillis();
    uint64_t prevEndTime = globalStartTime;
    while (!done) {
        while (!receivedNotification) {
            // wait for notification to run next interactive transaction
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock);
        }
        receivedNotification = false;

        uint64_t startTime = currentTimeMillis();

        DObject::TransactionBegin();
        dstr.Set(std::to_string(startTime));
        int committed = DObject::TransactionCommit();

        uint64_t endTime = currentTimeMillis();

        std::cout << (endTime - prevEndTime) << "\t" << committed << std::endl;

        prevEndTime = endTime;

        if (endTime - globalStartTime > numSeconds * 1000) {
            done = true;
        }
    }

    // sleep to catch the last notification from the frontend (to prevent the frontend from segfaulting)
    sleep(1);
}
