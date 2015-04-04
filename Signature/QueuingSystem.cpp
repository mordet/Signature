#include <exception>
#include <sstream>

#include "QueuingSystem.h"

const std::chrono::duration<int, std::milli> c_threadWait = std::chrono::milliseconds(20u);
const size_t c_maximumQueueSize = 32u;

QueuingSystem::QueuingSystem(const std::string& inputFile, size_t chunkSize, 
                             unsigned long long chunksCount, unsigned long long fileSize)
    : input(inputFile, std::ios::binary), chunkSize(chunkSize), 
    chunksCount(chunksCount), fileSize(fileSize), done()
{
    input.exceptions(std::ios::failbit);
    if (!input)
    {
        std::ostringstream ss;
        ss << "input file doesn't exist: " << inputFile;
        throw std::exception(ss.str().c_str());
    }

    done.store(false);
}

QueuingSystem::~QueuingSystem(void)
{
}

void QueuingSystem::run(ExceptionPtr& exceptionPtr)
{
    try
    {
        for (unsigned long long processed = 0u; processed < chunksCount && !exceptionPtr; ++processed)
        {
            std::unique_lock<std::mutex> lock(conditionalMutex);
            notFull.wait(lock, [this]() { return queue.size() < c_maximumQueueSize; });


            input.seekg(chunkSize * processed);/*
            if (processed > 0u)
                input.seekg(chunkSize, std::ios::cur);*/

            size_t currentChunkSize = std::min(chunkSize, static_cast<size_t>(fileSize - chunkSize * processed));

            RequestPtr requestPtr(new Request(processed, currentChunkSize));
            input.read(&*requestPtr->destination.begin(), currentChunkSize);

            queue.push(requestPtr);
            notEmpty.notify_one();
        }

        notEmpty.notify_all();
        done.store(true);
    }
    catch (...)
    {
        exceptionPtr = std::current_exception();
        done.store(true);
    }
}

QueuingSystem::RequestPtr QueuingSystem::get()
{
    std::unique_lock<std::mutex> lock(conditionalMutex);
    notEmpty.wait(lock, [this](){ return done.load() || !queue.empty(); });

    if (done.load() && queue.empty())
        return RequestPtr(nullptr);

    RequestPtr requestPtr = queue.front();
    queue.pop();

    notFull.notify_one();
    return requestPtr;
}

QueuingSystem::Request::Request(unsigned long long ordinalNumber, size_t size)
    : destination(size, 0), ordinalNumber(ordinalNumber)
{
}