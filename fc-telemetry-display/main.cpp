#include "Window.h"
#include "Timer.h"
#include "Input.h"
#include "GuiDebugFrameMetrics.h"
#include "BluetoothManager.h"
BluetoothManager btManager;
enum class ApplicationState : u8
{
	DEFAULT,
	CONNECTING_TO_BLUETOOTH_DEVICE
};
ApplicationState appState = ApplicationState::DEFAULT;
vector<device> currentDeviceList;
char const* selectedBtConnectDevice = nullptr;
string      selectedBtConnectAddress;
vector<char> currentRawTelemetry;
SDL_cond* btMgrLockConditionVar = nullptr;
int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		return EXIT_FAILURE;
	}
	defer(SDL_Quit());
	btMgrLockConditionVar = SDL_CreateCond();
	if (!btMgrLockConditionVar)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to create condition variable: '%s'\n", 
					 SDL_GetError());
		return EXIT_FAILURE;
	}
	defer(SDL_DestroyCond(btMgrLockConditionVar));
	btManager.initialize();
	defer(btManager.free());
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
//	Timer totalApplicationTimer;
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
		btManager.lock(btMgrLockConditionVar);
		const Time frameTime = frameTimer.restart();
//		const Time appTime = totalApplicationTimer.getElapsedTime();
		if (k10::input->actionPressed("quickExit") &&
			k10::input->actionHeld("quickExitActive"))
		{
			k10::window->close();
		}
		k10::window->clear(Color(30, 30, 30));
		k10::guiDebugFrameMetrics->drawImGuiFrameMetrics(frameTime);
		// Main Menu GUI //
		const bool enableMenuItems = (appState == ApplicationState::DEFAULT);
		if (ImGui::BeginMainMenuBar())
		{
			if (btManager.bluetoothSerialConnected())
			{
				if (ImGui::MenuItem("Disconnect", nullptr, nullptr, enableMenuItems))
				{
					SDL_assert(false);
					//TODO
				}
			}
			else
			{
				if (ImGui::MenuItem("Connect", nullptr, nullptr, enableMenuItems))
				{
					currentDeviceList.clear();
					selectedBtConnectAddress = "";
					selectedBtConnectDevice = nullptr;
					btManager.requestDeviceInquiry();
					appState = ApplicationState::CONNECTING_TO_BLUETOOTH_DEVICE;
				}
			}
			ImGui::EndMainMenuBar();
		}
		switch (appState)
		{
		case ApplicationState::DEFAULT: {
			if (btManager.bluetoothSerialConnected())
			{
				currentRawTelemetry = btManager.getRawTelemetry();
				ImGui::Begin("Raw Telemetry");
				if (!currentRawTelemetry.empty())
				{
					ImGui::TextUnformatted(currentRawTelemetry.data(), 
										   currentRawTelemetry.data() + 
												currentRawTelemetry.size());
				}
				if (btManager.extractNewTelemetryByteCount() > 0)
				{
					ImGui::SetScrollHereY(1.f);
				}
				ImGui::End();
			}
			//TODO: console interface with the modem
		}	break;
		case ApplicationState::CONNECTING_TO_BLUETOOTH_DEVICE: {
			bool open = true;
			ImGui::Begin("Connect to Bluetooth Device", &open);
			if (btManager.isQueryingForDevices())
			{
				ImGui::Text("Scanning bluetooth devices... %c", 
							"|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
			}
			else
			{
				if (currentDeviceList.empty())
				{
					currentDeviceList = btManager.getDeviceList();
				}
				if (currentDeviceList.empty())
				{
					if (ImGui::Button("Scan For Bluetooth"))
					{
						currentDeviceList.clear();
						selectedBtConnectAddress = "";
						selectedBtConnectDevice = nullptr;
						btManager.requestDeviceInquiry();
					}
				}
				else if (btManager.isRequestingBtConnection())
				{
					ImGui::Text("Connecting to '%s'... %c",
								selectedBtConnectDevice,
								"|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
				}
				else
				{
					if (ImGui::BeginCombo("devices", selectedBtConnectDevice))
					{
						for (auto const& d : currentDeviceList)
						{
							bool isSelected = (selectedBtConnectDevice == d.name.c_str());
							if (ImGui::Selectable(d.name.c_str(), isSelected))
							{
								selectedBtConnectDevice  = d.name.c_str();
								selectedBtConnectAddress = d.address;
							}
							if (isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}
					if (!btManager.isRequestingBtConnection() &&
						ImGui::Button("Connect"))
					{
						btManager.requestBluetoothConnection(
							selectedBtConnectAddress);
					}
				}
			}
			ImGui::End();
			if (!open || btManager.bluetoothSerialConnected())
			{
				appState = ApplicationState::DEFAULT;
			}
		}	break;
		}
		btManager.unlock(btMgrLockConditionVar);
		SDL_Delay(15);
		k10::window->swapBuffer();
	}
	return EXIT_SUCCESS;
}