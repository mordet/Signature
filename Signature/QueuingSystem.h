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
    explicit QueuingSystem(const std::string& inputFile, size_t chunkSize, unsigned long long chunksCount);
    ~QueuingSystem(void);

    void run(ExceptionPtr& exceptionPtr);
    void asyncShutdown();
    void postRequest(std::condition_variable& condition, std::shared_ptr<std::vector<char>> destination, unsigned long long chunkNumber, bool& done);

private:
    class Request;
    typedef std::shared_ptr<Request> RequestPtr;

    std::ifstream input;
    const size_t chunkSize;
    const unsigned long long chunksCount;

    std::thread thread;
    std::atomic_bool stop;

    std::mutex condMutex;
    std::condition_variable conditional;

    std::mutex queueMutex;
    std::queue<RequestPtr> queue;
};

inline void QueuingSystem::asyncShutdown()
{
    stop.store(true);
}

class QueuingSystem::Request : boost::noncopyable
{
public:
    Request(std::condition_variable& condition, std::shared_ptr<std::vector<char>> destination, unsigned long long offset, bool& done);

    std::condition_variable& condition;
    std::shared_ptr<std::vector<char>> destination;
    unsigned long long offset;
    mutable bool& done;
};
