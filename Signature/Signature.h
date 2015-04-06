#pragma once

#include <string>
#include <thread>
#include <atomic>

#include "ExceptionPtr.h"
#include "Reporter.h"

class Signature
{
public:
	Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize);
	~Signature();

private:
	typedef Reporter<unsigned> Crc32Reporter;

	const std::string inputFile;
	const unsigned long long inputFileLength;
	const size_t blockSize;
	std::atomic_bool terminateExecution;
	ExceptionPtr currentException;

	void processFilePart(unsigned threadNumber, unsigned threadsCount, Crc32Reporter& reporter);
	unsigned long long getFileLength(const std::string& fileName) const;
	unsigned selectThreadsCount(unsigned long long chunksCount) const;
};

