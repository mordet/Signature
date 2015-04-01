#pragma once

#include <thread>
#include <vector>

class SafeThreads
{
public:
	SafeThreads();
	~SafeThreads();
	void add(std::thread&& thread);

private:
	std::vector<std::thread> threads;
};

