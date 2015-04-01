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
		return EXIT_FAILURE;
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
			return EXIT_FAILURE;
		}
		catch (const std::out_of_range&)
		{
			std::cerr << "block_size is too big: " << argv[3] << std::endl;
			return EXIT_FAILURE;
		}
	}

	try
	{
		auto start = std::chrono::system_clock::now();
		Signature signature(inputFilePath, outputFilePath, blockSize);
		auto end = std::chrono::system_clock::now();
		std::cout << "total duration: " << std::chrono::duration<double>(end - start).count() 
			<< "s" << std::endl;
		
	}
	catch (const std::logic_error& ex)
	{
		std::cerr << "logic error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (const std::ios_base::failure& fail)
	{
		std::cerr << "IO error: " << fail.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "Unknown exception: " << std::current_exception() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

