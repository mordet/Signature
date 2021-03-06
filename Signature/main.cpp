#include <iostream>
#include <string>
#include <chrono>

#include "Signature.h"

const size_t c_defaultBlockSize = 1048576u;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cerr << "usage: " << argv[0] << " <input_file> <output_file> [block_size]" << std::endl;
		return EXIT_FAILURE;
	}

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

		Signature signature(argv[1], argv[2], blockSize);

		auto end = std::chrono::system_clock::now();
		std::cout << "total duration: " << std::chrono::duration<double>(end - start).count() 
			<< "s" << std::endl;

		return EXIT_SUCCESS;
	}
	catch (const std::logic_error& ex)
	{
		std::cerr << ex.what() << std::endl;
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
}

