#include "OrderedLock.h"
bool OrderedLock::initialize()
{
	free();
	orderedMutex = SDL_CreateMutex();
	if (!orderedMutex)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to create mutex: '%s'\n", SDL_GetError());
		return false;
	}
	return true;
}
void OrderedLock::free()
{
	if (orderedMutex)
	{
		if (SDL_LockMutex(orderedMutex) != 0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						 "Failed to lock mutex: '%s'\n", SDL_GetError());
			SDL_assert(false);
		}
		while (!conditionQ.empty())
		{
			SDL_CondBroadcast(conditionQ.front());
			conditionQ.pop();
		}
		SDL_UnlockMutex(orderedMutex);
		SDL_DestroyMutex(orderedMutex);
	}
	orderedMutex = nullptr;
}
bool OrderedLock::lock(SDL_cond* conditionVariable)
{
	if (SDL_LockMutex(orderedMutex) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to lock mutex: '%s'\n", SDL_GetError());
		SDL_assert(false); return false;
	}
	defer(SDL_UnlockMutex(orderedMutex));
	conditionQ.push(conditionVariable);
	while (conditionQ.front() != conditionVariable)
	{
		SDL_CondWait(conditionVariable, orderedMutex);
	}
	return true;
}
bool OrderedLock::unlock(SDL_cond* conditionVariable)
{
	if (SDL_LockMutex(orderedMutex) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to lock mutex: '%s'\n", SDL_GetError());
		SDL_assert(false); return false;
	}
	defer(SDL_UnlockMutex(orderedMutex));
	// Destroy the condition variable that is currently locking.  This should 
	//	be the one the current thread added to the queue in the lock function //
	if (!conditionQ.empty())
	{
		if (conditionQ.front() != conditionVariable)
		{
			SDL_assert(false); return false;
		}
//		SDL_DestroyCond(conditionQ.front());
		conditionQ.pop();
	}
	// If there is another thread's condition variable waiting in line, tell it
	//	to take its turn now! //
	if (!conditionQ.empty())
	{
		if (SDL_CondSignal(conditionQ.front()) != 0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to signal condition: '%s'\n", SDL_GetError());
			SDL_assert(false); return false;
		}
	}
	return true;
}
