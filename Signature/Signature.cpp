#include <thread>
#include <fstream>
#include <vector>
#include <limits>
#include <exception>

#pragma warning(disable: 4244)
#pragma warning(disable: 4245)
#include <boost/crc.hpp> 
#pragma warning(disable: 4245)
#pragma warning(default: 4244)

#include "Signature.h"

const unsigned c_defaultThreadCount = 4u;

Signature::Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize)
	: inputFile(inputFilePath), inputFileLength(-1), outputFile(outputFilePath), 
	blockSize(blockSize), threads(), terminateExecution(), exceptionMutex(), currentException()
{
	terminateExecution.store(false);

	std::ifstream input(inputFilePath, std::ios::binary | std::ios::ate);
	if (!input)
		throw std::logic_error("input file does not exist");

	inputFileLength = input.tellg();
	if (inputFileLength <= 0)
		throw std::logic_error("input file can't be read");

	long long chunksCount = static_cast<long long>(std::ceil((double)inputFileLength / blockSize));
	
	unsigned concurrency = std::thread::hardware_concurrency();
	if (0 == concurrency)
		concurrency = c_defaultThreadCount;

	if (chunksCount < std::numeric_limits<unsigned>::max())
		concurrency = std::min(concurrency, static_cast<unsigned>(chunksCount));

	for (unsigned i = 1u; i < concurrency; ++i)
	{
		threads.push_back(std::thread(&Signature::work, this, i, concurrency));
	}

	work(0u, concurrency);

	for (std::thread& thread : threads)
		if (thread.joinable())
			thread.join();

	if (currentException)
	{
		std::rethrow_exception(currentException);
	}
}

Signature::~Signature(void)
{
	for (std::thread& thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}
}

void Signature::work(unsigned threadNumber, unsigned threadsCount)
{
	try
	{
		std::ifstream input(inputFile, std::ios::binary);
		input.exceptions(std::ios::failbit);
		if (!input)
			throw std::logic_error("input file does not exist");

		std::ofstream output(outputFile, std::ios::binary);
		output.exceptions(std::ios::failbit);
		if (!output)
			throw std::logic_error("output file can't be opened");

		input.seekg(threadNumber * blockSize);

		std::vector<char> data(blockSize, 0);

		for (unsigned iteration = 0; !terminateExecution.load()
			&& blockSize * (iteration * threadsCount + threadNumber) < inputFileLength; 
			++iteration)
		{
			size_t nextChunkSize = std::min(blockSize, 
				static_cast<size_t>(inputFileLength - blockSize * (iteration * threadsCount + threadNumber)));

			char *begin = &*data.begin();
			input.read(begin, nextChunkSize);

			boost::crc_32_type result;
			result.process_bytes(begin, nextChunkSize);
			output.seekp((result.bit_count / 8u) * (threadNumber + (iteration * threadsCount)));
			output << result.checksum();
		}
	}
	catch (...)
	{
		terminateExecution.store(true);
		std::unique_lock<std::mutex> lock(exceptionMutex);
		currentException = std::current_exception();
	}
}