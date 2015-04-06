#include <fstream>
#include <limits>
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
const unsigned c_outputStringWidth = 16u;

Signature::Signature(const std::string& inputFilePath, const std::string& outputFilePath, size_t blockSize)
	: inputFile(inputFilePath), 
	inputFileLength(getFileLength(inputFilePath)),
	blockSize(blockSize), terminateExecution(), currentException()
{
	terminateExecution.store(false);

	unsigned long long chunksCount = inputFileLength / blockSize;
	if (inputFileLength % blockSize > 0u)
		++chunksCount;

	Crc32Reporter reporter(outputFilePath, chunksCount);

	unsigned concurrency = selectThreadsCount(chunksCount);

	std::unique_ptr<SafeThreads> threads(new SafeThreads());
	for (unsigned i = 1u; i < concurrency; ++i)
		threads->add(std::thread(&Signature::processFilePart, this, i, concurrency, std::ref(reporter)));

	processFilePart(0u, concurrency, reporter);

	if (currentException)
	{
		currentException.rethrow();
	}
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

void Signature::processFilePart(unsigned threadNumber, unsigned threadsCount, Crc32Reporter& reporter)
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