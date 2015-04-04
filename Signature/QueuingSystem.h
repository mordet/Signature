#pragma once

#include <condition_variable>
#include <fstream>
#include <string>
#include <thread>
#include <queue>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <boost/noncopyable.hpp>

#include "ExceptionPtr.h"

class QueuingSystem : private boost::noncopyable
{
public:
    class Request;
    typedef std::shared_ptr<Request> RequestPtr;

    explicit QueuingSystem(const std::string& inputFile, size_t chunkSize, unsigned long long chunksCount, unsigned long long fileSize);
    ~QueuingSystem(void);

    void run(ExceptionPtr& exceptionPtr);
    RequestPtr get();
    
private:
    std::ifstream input;
    const size_t chunkSize;
    const unsigned long long chunksCount;
    const unsigned long long fileSize;
    std::atomic_bool done;

    std::mutex conditionalMutex;
    std::condition_variable notEmpty;
    std::condition_variable notFull;
    
    std::mutex queueMutex;
    std::queue<RequestPtr> queue;
};

class QueuingSystem::Request : boost::noncopyable
{
public:
    Request(unsigned long long ordinalNumber, size_t size);

    std::vector<char> destination;
    const unsigned long long ordinalNumber;
};
