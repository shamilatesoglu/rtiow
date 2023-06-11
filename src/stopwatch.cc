#include "stopwatch.h"

stopwatch::stopwatch()
{
	reset();
}

void stopwatch::reset()
{
	start_time = std::chrono::high_resolution_clock::now();
}

double stopwatch::elapsed() const
{
	auto end_time = std::chrono::high_resolution_clock::now();
	auto elapsed_time =
	    std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
	return elapsed_time.count() / 1000000.0;
}

std::string stopwatch::elapsed_str() const
{
	double seconds = elapsed();
	return elapsed_str(seconds);
}

std::string stopwatch::elapsed_str(double seconds)
{
	if (seconds < 1e-6) {
		return std::to_string(seconds * 1e9) + "ns";
	} else if (seconds < 1e-3) {
		return std::to_string(seconds * 1e6) + "us";
	} else if (seconds < 1) {
		return std::to_string(seconds * 1e3) + "ms";
	} else {
		return std::to_string(seconds) + "s";
	}
}