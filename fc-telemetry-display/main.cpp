#include "Window.h"
#include "Timer.h"
#include "Input.h"
#include "GuiDebugFrameMetrics.h"
#include <DeviceINQ.h>
int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		return EXIT_FAILURE;
	}
	defer(SDL_Quit());
	if (!k10::initializeGlobalVariables())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to load global variables!\n");
		return EXIT_FAILURE;
	}
	if (!k10::loadGlobalAssets())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to load global assets!\n");
		return EXIT_FAILURE;
	}
#ifndef NDEBUG
	// test out bluetooth serial port library //
	{
		DeviceINQ* const devInq = DeviceINQ::Create();
		vector<device> devices = devInq->Inquire();
		for (auto const& d : devices)
		{
			SDL_Log("device '%s' @ '%s'\n", d.name.c_str(), d.address.c_str());
		}
	}
#endif
	Timer totalApplicationTimer;
	Timer frameTimer;
	while (k10::window->isOpen())
	{
		SDL_Event sdlEvent;
		k10::input->step();
		while (SDL_PollEvent(&sdlEvent))
		{
			k10::input->processEvent(sdlEvent);
			k10::window->processEvent(sdlEvent);
			switch (sdlEvent.type)
			{
			case SDL_EventType::SDL_MOUSEBUTTONDOWN: {
				if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
				{
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
			}break;
			case SDL_EventType::SDL_MOUSEBUTTONUP: {
				if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
				{
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}
			}break;
			case SDL_EventType::SDL_QUIT: {
				k10::window->close();
			}break;
			}
		}
		const Time frameTime = frameTimer.restart();
		const Time appTime = totalApplicationTimer.getElapsedTime();
		if (k10::input->actionPressed("quickExit") &&
			k10::input->actionHeld("quickExitActive"))
		{
			k10::window->close();
		}
		k10::window->clear(Color(30, 30, 30));
		k10::guiDebugFrameMetrics->drawImGuiFrameMetrics(frameTime);
		k10::window->swapBuffer();
	}
	return EXIT_SUCCESS;
}