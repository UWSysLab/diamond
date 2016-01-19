#include "includes/data_types.h"
#include "lib/message.h"

using namespace diamond;

int main(int argc , char **argv){
   DiamondInit(argv[1], 1, 0);

    DLong long1;
    DString str1;
    DObject::Map(long1, "thiskeydoesnotexist");
    DObject::Map(str1, "thiskeydoesnotexisteither");

    Notice("Value of string is %s\n", str1.Value().c_str());
    Notice("Value of long is %ld\n", long1.Value());

    return 0;
}

