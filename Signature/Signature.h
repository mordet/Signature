#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

#include "QueuingSystem.h"
#include "ExceptionPtr.h"
#include "Reporter.h"

class Signature
{
public:
	Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize);
	~Signature();

    void parallelRead();
    void syncRead();

private:
	const std::string inputFile;
	const unsigned long long inputFileLength;
	const std::string outputFile;
	const size_t blockSize;

	std::atomic_bool terminateExecution;
    mutable ExceptionPtr currentException;

    void parallelReadThread(unsigned threadNumber, unsigned threadsCount, Crc32Reporter& reporter);
    void syncReadThread(QueuingSystem& producer, Crc32Reporter& reporter);

	unsigned long long getFileLength(const std::string& fileName) const;
	unsigned selectThreadsCount(unsigned long long chunksCount) const;
};

