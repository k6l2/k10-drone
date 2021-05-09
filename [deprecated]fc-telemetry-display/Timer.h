#pragma once
#include "Time.h"
class Timer
{
public:
	Time getElapsedTime() const;
	Time restart();
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> clockStart =
		std::chrono::high_resolution_clock::now();
};
