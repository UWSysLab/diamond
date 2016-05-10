#include <iostream>
#include <string>
#include <unistd.h>
#include <boost/program_options.hpp>

#include <includes/data_types.h>
#include <includes/transactions.h>

using namespace diamond;
namespace po = boost::program_options;

DString dstr;

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string key;
    std::string value;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("key", po::value<std::string>(&key)->required(), "key to publish to")
        ("value", po::value<std::string>(&value)->required(), "value to write")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();

    DObject::Map(dstr, key);

    DObject::TransactionBegin();
    dstr.Set(value);
    DObject::TransactionCommit();

    return 0;
}
