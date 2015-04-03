#pragma once

#include <exception>
#include <mutex>
#include <boost/noncopyable.hpp>

class ExceptionPtr : boost::noncopyable
{
public:
    ExceptionPtr(void) {}
    ~ExceptionPtr(void) {}
    
    ExceptionPtr& operator =(std::exception_ptr toStore)
    {
        std::unique_lock<std::mutex> lock(mutex);
        ptr = toStore;
        return *this;
    };

    operator bool() const
    {
        return ptr;
    }

    void rethrow()
    {
        if (ptr)
            std::rethrow_exception(ptr);
    }

private:
    std::mutex mutex;
    std::exception_ptr ptr;
};

