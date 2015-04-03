#include <exception>
#include <sstream>

#include "QueuingSystem.h"

const std::chrono::duration<int, std::milli> c_threadWait = std::chrono::milliseconds(20u);

QueuingSystem::QueuingSystem(const std::string& inputFile, size_t chunkSize, unsigned long long chunksCount)
    : input(inputFile, std::ios::binary), chunkSize(chunkSize), stop(), chunksCount(chunksCount)
{
    input.exceptions(std::ios::failbit);
    if (!input)
    {
        std::ostringstream ss;
        ss << "input file doesn't exist: " << inputFile;
        throw std::exception(ss.str().c_str());
    }

    stop.store(false);
}

QueuingSystem::~QueuingSystem(void)
{
    stop.store(true);
}

void QueuingSystem::run(ExceptionPtr& exceptionPtr)
{
    try
    {
        unsigned long long processed = 0u;
        while (!exceptionPtr && processed < chunksCount)
        {
            //std::unique_lock<std::mutex> condLock(condMutex);
            //conditional.wait(condLock);

            std::unique_lock<std::mutex> lock(queueMutex);
            if (!queue.empty())
            {
                RequestPtr requestPtr = queue.front();
                queue.pop();
                lock.unlock();

                input.seekg(requestPtr->offset);
                requestPtr->destination->resize(chunkSize, 0);
                
                // Check this
                input.readsome(&*requestPtr->destination->begin(), chunkSize);
                requestPtr->done = true;
                requestPtr->condition.notify_one();
                ++processed;
            }
            else
            {
                std::this_thread::sleep_for(c_threadWait);
            }
        }
    }
    catch (...)
    {
        exceptionPtr = std::current_exception();
    }
}

void QueuingSystem::postRequest(std::condition_variable& condition, 
                                std::shared_ptr<std::vector<char>> destination, unsigned long long chunkNumber, bool& done)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queue.push(std::make_shared<Request>(condition, destination, chunkNumber * chunkSize, done));

    conditional.notify_one();
}

QueuingSystem::Request::Request(std::condition_variable& condition, 
                                std::shared_ptr<std::vector<char>> destination, unsigned long long offset, bool& done)
    : condition(condition), destination(destination), offset(offset), done(done)
{
}