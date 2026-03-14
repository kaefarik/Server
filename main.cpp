#include "Engine.h"
#include "Receiver.h"
#include <iostream>

using namespace std;

int main()
{
    AReceiver *ptr = new FooReceiver;
    ptr->receive();
}
