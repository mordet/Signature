#pragma once

#include <string>
#include <thread>
#include <vector>

class Signature
{
public:
	Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize);
	~Signature();

private:
	std::vector<std::thread> threads;
	std::string inputFile;
	long long inputFileLength;
	std::string outputFile;
	size_t blockSize;

	void work(unsigned threadNumber, unsigned threadsCount);
};

