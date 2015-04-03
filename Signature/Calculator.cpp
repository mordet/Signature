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
        std::mutex mutex;
        std::shared_ptr<std::vector<char>> buffer(new std::vector<char>());

        for (size_t it = ordinalNumber; !exceptionPtr && it < chunksCount; it += calculatorsCount)
        {
            {
                bool done = false;
                std::condition_variable conditional;
                std::unique_lock<std::mutex> lock(mutex);
                queuingSystem->postRequest(conditional, buffer, it, done);
                conditional.wait(lock, [&done](){ return done; });
            }

            boost::crc_32_type crc;
            crc.process_bytes(&*buffer->begin(), buffer->size());
            reporter.postResult(it, crc.checksum());
        }
    }
    catch (...)
    {
        exceptionPtr = std::current_exception();
    }
}
