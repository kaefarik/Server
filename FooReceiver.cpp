#include "Receiver.h"
#include "string"
#include <iostream>

std::string FooReceiver::receive()
{
    std::string msg;
    std::cin >> msg;
    return msg;
}
