// ----------------------------------------------------
// Timer.cpp
// Body for CPU Tick Timer class
// ----------------------------------------------------

#include "Timer.h"
#include "SDL\include\SDL_timer.h"

// ---------------------------------------------
Timer::Timer()
{
	Start();
}

Timer::~Timer()
{
}

// ---------------------------------------------
void Timer::Start()
{
	started_at = SDL_GetTicks();
	active = true;
}

// ---------------------------------------------
Uint32 Timer::Read() const
{
	if (active)
	{
		if (paused)
			return (paused_at - started_at);
		else
			return (SDL_GetTicks() - started_at);
	}
	else
		return 0;
}

// ---------------------------------------------
float Timer::ReadSec() const
{
	return float(Read()) / 1000.0f;
}

void Timer::SubstractTimeFromStart(float sec)
{
	started_at -= (sec * 1000);
}

void Timer::Stop()
{
	active = false;
}

void Timer::PauseOn()
{
	if (paused == false)
	{
		paused_at = SDL_GetTicks();
		paused = true;
	}
}

void Timer::PauseOff()
{
	if (paused == true)
	{
		started_at += (SDL_GetTicks() - paused_at);

		paused_at = 0;
		paused = false;
	}
}

bool Timer::IsActive()
{
	return active;
}


