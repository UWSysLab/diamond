#include "includes/data_types.h"

using namespace diamond;

int main(void){
    DiamondInit();

    DStringList list1;
    DObject::Map(list1, "testlist");

    list1.Clear();
    list1.Append("Big long test message 1: this message is very long");
    list1.Append("Big long test message 2: this message is very long");
    list1.Append("Big long test message 3: this message is very long");
    list1.Append("Big long test message 4: this message is very long");
    list1.Append("Big long test message 5: this message is very long");
    list1.Append("Big long test message 6: this message is very long");
    list1.Append("Big long test message 7: this message is very long");
    list1.Append("Big long test message 8: this message is very long");
    list1.Append("Big long test message 9: this message is very long");
    list1.Append("Big long test message 10: this message is very long");
    return 0;
}

