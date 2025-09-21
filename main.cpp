#include <chrono>
#include <Windows.h>
#include "scene7.h"


int main()
{
	const auto start = std::chrono::high_resolution_clock::now();
	scene();
	const auto end = std::chrono::high_resolution_clock::now();
	const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	const std::string time_log = "(time: )" + std::to_string(duration.count() / 1000) + " s\n";
	OutputDebugStringA(time_log.c_str());
}
