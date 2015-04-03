#pragma once

#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

#include "ExceptionPtr.h"
#include "Reporter.h"

class QueuingSystem;

class Calculator : private boost::noncopyable
{
public:
    Calculator(std::shared_ptr<QueuingSystem> queuingSystem, Crc32Reporter& reporter,
        unsigned long long chunksCount, unsigned ordinalNumber, unsigned calculatorsCount);
    ~Calculator(void);

    void run(ExceptionPtr& exceptionPtr);

private:
    std::shared_ptr<QueuingSystem> queuingSystem;
    Crc32Reporter& reporter;
    const unsigned long long chunksCount;
    const unsigned ordinalNumber;
    const unsigned calculatorsCount;
};

