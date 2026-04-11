#pragma once
#include <string>

class AReceiver
{
public:
    virtual std::string receive() = 0;
};

class FooReceiver : public AReceiver
{
public:
    std::string receive() override;
};
