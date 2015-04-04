#include <mutex>

#pragma warning(disable: 4244)
#pragma warning(disable: 4245)
#include <boost/crc.hpp> 
#pragma warning(disable: 4245)
#pragma warning(default: 4244)

#include "Calculator.h"
#include "QueuingSystem.h"

Calculator::Calculator(std::shared_ptr<QueuingSystem> queuingSystem, Crc32Reporter& reporter, 
                       unsigned long long chunksCount, unsigned ordinalNumber, unsigned calculatorsCount)
    : queuingSystem(queuingSystem), reporter(reporter), chunksCount(chunksCount), 
    ordinalNumber(ordinalNumber), calculatorsCount(calculatorsCount)
{

}

Calculator::~Calculator(void)
{
}

void Calculator::run(ExceptionPtr& exceptionPtr)
{
    try
    {
        QueuingSystem::RequestPtr requestPtr = queuingSystem->get();
        if (requestPtr)
        {
            boost::crc_32_type crc;
            crc.process_bytes(&*requestPtr->destination.begin(), requestPtr->destination.size());
            reporter.postResult(requestPtr->ordinalNumber, crc.checksum());
        }
        else
        {
            return;
        }
    }
    catch (...)
    {
        exceptionPtr = std::current_exception();
    }
}
