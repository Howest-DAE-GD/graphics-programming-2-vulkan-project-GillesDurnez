#pragma once
#include <chrono>

namespace gp2
{
	class Time
	{
	public:
		inline static float Lag{};
		inline static float DeltaTime{};
		inline static float FixedTimeStep{};
		inline static std::chrono::time_point<std::chrono::steady_clock> Last_time{};
	};
}