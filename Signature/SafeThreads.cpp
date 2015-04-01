#include <algorithm>

#include "SafeThreads.h"

SafeThreads::SafeThreads()
{
}

SafeThreads::~SafeThreads()
{
	for (std::thread& thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}
}

void SafeThreads::add(std::thread&& thread)
{
	threads.emplace_back(std::move(thread));
}