#pragma once

#include <iostream>

struct timestep
{
    timestep(float time = 0.0f) : time(time) {};

	inline float GetSeconds() { return time; };
	inline float GetMilliseconds() { return time * 1000.0f; };

	float time;
};
