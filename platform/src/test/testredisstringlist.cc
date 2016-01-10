#include "storage/cloud.h"
#include "includes/data_types.h"
#include "lib/message.h"

using namespace diamond;

int main(void){
    DiamondInit();

    DRedisStringList list("testlist");

    list.Clear();
    list.Append("test1");
    list.Append("test2");

    int size = list.Size();
    if (size != 2) {
        Panic("list.Size() is %d, should be 2", size);
    }

    std::string val = list.Value(1);
    if (val != "test2") {
        Panic("List has value %s at index 1, should be test2", val.c_str());
    }

    list.Append("test2");
    list.Remove("test2");

    size = list.Size();
    if (size != 2) {
        Panic("list.Size() is %d, should be 2", size);
    }

    val = list.Value(1);
    if (val != "test2") {
        Panic("List has value %s at index 1, should be test2", val.c_str());
    }

    list.EraseFirst();
    list.EraseFirst();

    size = list.Size();
    if (size != 0) {
        Panic("list.Size() is %d, should be 0", size);
    }


    return 0;
}

