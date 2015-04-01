#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class Signature
{
public:
	Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize);
	~Signature();

private:
	const std::string inputFile;
	const unsigned long long inputFileLength;
	const std::string outputFile;
	const size_t blockSize;

	std::atomic_bool terminateExecution;
	std::mutex exceptionMutex;
	std::exception_ptr currentException;

	void parallelRead();
	void parallelReadThread(unsigned threadNumber, unsigned threadsCount);

	void syncRead();

	unsigned long long getFileLength(const std::string& fileName) const;
	unsigned selectThreadsCount(size_t blockSize) const;
};

