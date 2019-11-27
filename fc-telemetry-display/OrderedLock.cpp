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
			SDL_DestroyCond(conditionQ.front());
			conditionQ.pop();
		}
		SDL_UnlockMutex(orderedMutex);
		SDL_DestroyMutex(orderedMutex);
	}
	orderedMutex = nullptr;
}
bool OrderedLock::lock()
{
	//OPTICK_EVENT();
	if (SDL_LockMutex(orderedMutex) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to lock mutex: '%s'\n", SDL_GetError());
		SDL_assert(false); return false;
	}
	if (locked)
	{
		SDL_cond*const conditionVar = SDL_CreateCond();
		if (!conditionVar)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to create condition var: '%s'\n", SDL_GetError());
			SDL_assert(false); return false;
		}
		conditionQ.push(conditionVar);
		while (conditionQ.front() != conditionVar)
		{
			SDL_CondWait(conditionVar, orderedMutex);
		}
	}
	else
	{
		locked = true;
	}
	SDL_UnlockMutex(orderedMutex);
	return true;
}
bool OrderedLock::unlock()
{
	if (SDL_LockMutex(orderedMutex) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to lock mutex: '%s'\n", SDL_GetError());
		SDL_assert(false); return false;
	}
	if (conditionQ.empty())
	{
		locked = false;
	}
	else
	{
		if (SDL_CondSignal(conditionQ.front()) != 0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to signal condition: '%s'\n", SDL_GetError());
			SDL_assert(false); return false;
		}
		SDL_DestroyCond(conditionQ.front());
		conditionQ.pop();
	}
	SDL_UnlockMutex(orderedMutex);
	return true;
}
//bool OrderedLock::isLocked() const
//{
//	return locked;
//}