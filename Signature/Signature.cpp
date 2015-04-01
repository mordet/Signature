#include <fstream>
#include <limits>
#include <exception>
#include <memory>
#include <sstream>

#pragma warning(disable: 4244)
#pragma warning(disable: 4245)
#include <boost/crc.hpp> 
#pragma warning(disable: 4245)
#pragma warning(default: 4244)

#include "Signature.h"
#include "SafeThreads.h"

const unsigned c_defaultThreadCount = 4u;

Signature::Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize)
	: inputFile(inputFilePath), 
	inputFileLength(getFileLength(inputFilePath)), 
	outputFile(outputFilePath), 
	blockSize(blockSize), terminateExecution(), exceptionMutex(), currentException()
{
	terminateExecution.store(false);
	
	parallelRead();
}

unsigned long long Signature::getFileLength(const std::string& fileName) const
{
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);
	if (!file)
	{
		std::ostringstream ss;
		ss << "file \"" << fileName << "\" does not exist";
		throw std::logic_error(ss.str());
	}

	long long fileLength = file.tellg();
	if (fileLength < 0)
		throw std::logic_error("input file can't be read");

	return static_cast<unsigned long long>(fileLength);
}

unsigned Signature::selectThreadsCount(size_t blockSize) const
{
	unsigned long long chunksCount = inputFileLength / blockSize;
	if (inputFileLength % blockSize > 0u)
		++chunksCount;

	unsigned concurrency = std::thread::hardware_concurrency();
	if (0 == concurrency)
		concurrency = c_defaultThreadCount;

	if (chunksCount < std::numeric_limits<unsigned>::max())
		concurrency = std::min(concurrency, static_cast<unsigned>(chunksCount));
	return concurrency;
}

Signature::~Signature(void)
{
}

void Signature::parallelRead()
{
	unsigned concurrency = selectThreadsCount(blockSize);

	std::unique_ptr<SafeThreads> threads(new SafeThreads());
	for (unsigned i = 1u; i < concurrency; ++i)
		threads->add(std::thread(&Signature::parallelReadThread, this, i, concurrency));

	parallelReadThread(0u, concurrency);

	if (currentException)
	{
		std::rethrow_exception(currentException);
	}
}

void Signature::parallelReadThread(unsigned threadNumber, unsigned threadsCount)
{
	try
	{
		std::ifstream input(inputFile, std::ios::binary);
		input.exceptions(std::ios::failbit);
		if (!input)
			throw std::logic_error("input file does not exist");

		std::ofstream output(outputFile, std::ios::binary);
		if (!output)
		{
			std::stringstream ss;
			ss << "output file \"" << outputFile << "\" couldn't be opened for writing";
			throw std::logic_error(ss.str());
		}
		output.exceptions(std::ios::failbit);
		
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

			unsigned checksum = result.checksum();

			// Binary hack (ugly one)
			output.write(reinterpret_cast<const char*>(&checksum), result.bit_count / 8u);
		}
	}
	catch (...)
	{
		terminateExecution.store(true);
		std::unique_lock<std::mutex> lock(exceptionMutex);
		currentException = std::current_exception();
	}
}

void Signature::syncRead()
{
	unsigned concurrency = selectThreadsCount(blockSize);

	std::unique_ptr<SafeThreads> threads(new SafeThreads());
	for (unsigned i = 1u; i < concurrency; ++i)
		threads->add(std::thread(&Signature::parallelReadThread, this, i, concurrency));

	parallelReadThread(0u, concurrency);

	if (currentException)
	{
		std::rethrow_exception(currentException);
	}
}