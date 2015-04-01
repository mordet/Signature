#include <iostream>
#include <string>
#include <chrono>

#include "Signature.h"

const size_t c_defaultBlockSize = 1048576u;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage: Signature <input_file> <output_file> [block_size]" << std::endl;
		return 1;
	}

	std::string inputFilePath(argv[1]);
	std::string outputFilePath(argv[2]);

	size_t blockSize = c_defaultBlockSize;
	if (argc >= 4)
	{
		try
		{
			blockSize = std::stoul(argv[3]);
		}
		catch (const std::invalid_argument&)
		{
			std::cerr << "block_size is not a number: " << argv[3] << std::endl;
			return 1;
		}
		catch (const std::out_of_range&)
		{
			std::cerr << "block_size is too big: " << argv[3] << std::endl;
			return 1;
		}
	}

	auto start = std::chrono::system_clock::now();
	Signature signature(inputFilePath, outputFilePath, blockSize);
	auto end = std::chrono::system_clock::now();
	std::cout << "total duration: " << std::chrono::duration<double>(end - start).count() 
		<< "s" << std::endl;
	return 0;
}

