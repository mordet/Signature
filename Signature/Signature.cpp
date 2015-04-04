#include <fstream>
#include <limits>
#include <exception>
#include <memory>
#include <sstream>
#include <iomanip>

#pragma warning(disable: 4244)
#pragma warning(disable: 4245)
#include <boost/crc.hpp> 
#pragma warning(disable: 4245)
#pragma warning(default: 4244)

#include "Signature.h"
#include "SafeThreads.h"

#include "QueuingSystem.h"
//#include "Calculator.h"
#include "Reporter.h"

const unsigned c_defaultThreadCount = 4u;
const unsigned c_outputStringWidth = 16u;

Signature::Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize)
	: inputFile(inputFilePath), 
	inputFileLength(getFileLength(inputFilePath)), 
	outputFile(outputFilePath), 
    blockSize(blockSize), terminateExecution(), currentException()
{
	terminateExecution.store(false);
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

unsigned Signature::selectThreadsCount(unsigned long long chunksCount) const
{
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
    unsigned long long chunksCount = inputFileLength / blockSize;
	if (inputFileLength % blockSize > 0u)
		++chunksCount;

    Crc32Reporter reporter(outputFile, chunksCount);

    unsigned concurrency = selectThreadsCount(chunksCount);

	std::unique_ptr<SafeThreads> threads(new SafeThreads());
	for (unsigned i = 1u; i < concurrency; ++i)
        threads->add(std::thread(&Signature::parallelReadThread, this, i, concurrency, std::ref(reporter)));
	
    parallelReadThread(0u, concurrency, reporter);

	if (currentException)
	{
        currentException.rethrow();
	}
}

void Signature::parallelReadThread(unsigned threadNumber, unsigned threadsCount, Crc32Reporter& reporter)
{
	try
	{
		std::ifstream input(inputFile, std::ios::binary);
		if (!input)
			throw std::logic_error("input file does not exist");
        input.exceptions(std::ios::failbit);
 
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
            reporter.postResult(threadNumber + (iteration * threadsCount), result.checksum());
		}
	}
	catch (...)
	{
		terminateExecution.store(true);
		currentException = std::current_exception();
	}
}

void Signature::syncRead()
{
    unsigned long long chunksCount = inputFileLength / blockSize;
	if (inputFileLength % blockSize > 0u)
		++chunksCount;

	unsigned concurrency = selectThreadsCount(chunksCount);

    QueuingSystem queuingSystem(inputFile, blockSize, chunksCount, inputFileLength);
    Crc32Reporter reporter(outputFile, chunksCount);

    std::unique_ptr<SafeThreads> threads(new SafeThreads());
    for (unsigned i = 0; i < concurrency; ++i)
    {
        threads->add(std::thread(&Signature::syncReadThread, this, std::ref(queuingSystem), std::ref(reporter)));
    }

    queuingSystem.run(currentException);

    if (currentException)
	{
        currentException.rethrow();
	}
}

void Signature::syncReadThread(QueuingSystem& producer, Crc32Reporter& reporter)
{
    try
    {
        for (QueuingSystem::RequestPtr requestPtr = producer.get(); 
            !terminateExecution.load() && requestPtr; requestPtr = producer.get())
        {
            boost::crc_32_type crc;
            crc.process_bytes(&*requestPtr->destination.begin(), requestPtr->destination.size());
            reporter.postResult(requestPtr->ordinalNumber, crc.checksum());
        }
    }
    catch (...)
    {
        terminateExecution.store(true);
        currentException = std::current_exception();
    }
}