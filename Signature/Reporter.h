#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <map>
#include <boost/noncopyable.hpp>

template <class T>
class Reporter : boost::noncopyable
{
public:
    Reporter(const std::string& outputFile, unsigned long long chunksCount);
    ~Reporter(void);

    void postResult(unsigned long long chunkNumber, T result);

private:
    std::mutex mutex;
    std::ofstream output;
    const unsigned long long chunksCount;
    unsigned long long nextChunk;
    std::map<unsigned long long, T> results;
};

typedef Reporter<unsigned> Crc32Reporter;

template <class T>
inline Reporter<T>::Reporter(const std::string& outputFile, unsigned long long chunksCount)
    : output(outputFile, std::ios::trunc), chunksCount(chunksCount), 
    nextChunk(0u), results(), mutex()
{
    if (!output)
    {
        std::ostringstream ss;
        ss << "failed to write to " << outputFile;
        throw std::exception(ss.str().c_str());
    }
    output.exceptions(std::ios::failbit);
}

template <class T>
inline Reporter<T>::~Reporter(void)
{
}

template <class T>
inline void Reporter<T>::postResult(unsigned long long chunkNumber, T result)
{
    std::unique_lock<std::mutex> lock(mutex);

    if (chunkNumber == nextChunk)
    {
        output << chunkNumber << " -> " << result << std::endl;
        ++nextChunk;

        for (auto it = results.find(nextChunk); chunksCount > nextChunk
            && results.end() != it; it = results.find(nextChunk))
        {
            output << it->first << " -> " << it->second << std::endl;
            results.erase(it);
            ++nextChunk;
        }
    }
    else
    {
        results[chunkNumber] = result;
    }
}

