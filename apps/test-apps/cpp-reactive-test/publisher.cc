#include <iostream>
#include <string>
#include <unistd.h>
#include <boost/program_options.hpp>

#include <includes/data_types.h>
#include <includes/transactions.h>

using namespace diamond;
namespace po = boost::program_options;

DString str;

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string message;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("message", po::value<std::string>(), "message to publish to the Diamond string")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    if (vm.count("message") > 0) {
        message = vm["message"].as<std::string>();
    }
    else {
        message = "Testing!";
    }

    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();

    DObject::Map(str, "cppreactivetest:str");

    DObject::TransactionBegin();
    str.Set(message);
    DObject::TransactionCommit();

    return 0;
}
