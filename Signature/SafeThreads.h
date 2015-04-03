#pragma once

#include <thread>
#include <vector>
#include <memory>

class SafeThreads
{
public:
	SafeThreads();
	~SafeThreads();
	void add(std::thread&& thread);

private:
	std::vector<std::thread> threads;
};

inline SafeThreads::SafeThreads()
{
}

inline SafeThreads::~SafeThreads()
{
	for (std::thread& thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}
}

inline void SafeThreads::add(std::thread&& thread)
{
	threads.emplace_back(std::move(thread));
}