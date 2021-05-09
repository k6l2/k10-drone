// Taken from: https://stackoverflow.com/a/14792685/4526664
//	Translated to use SDL2 thread synchronization primitives.
#pragma once
class OrderedLock
{
public:
	bool initialize();
	void free();
	bool lock(SDL_cond* conditionVariable);
	bool unlock(SDL_cond* conditionVariable);
private:
	queue<SDL_cond*> conditionQ;
	SDL_mutex* orderedMutex = nullptr;
};
