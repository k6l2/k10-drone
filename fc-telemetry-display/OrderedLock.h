// Taken from: https://stackoverflow.com/a/14792685/4526664
//	Translated to use SDL2 thread synchronization primitives.
#pragma once
class OrderedLock
{
public:
	bool initialize();
	void free();
	bool lock();
	bool unlock();
	//bool isLocked() const;
private:
	queue<SDL_cond*> conditionQ;
	SDL_mutex* orderedMutex = nullptr;
	bool locked = false;
};
