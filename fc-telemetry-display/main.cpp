#include "Window.h"
#include "Timer.h"
#include "Input.h"
#include "GuiDebugFrameMetrics.h"
#include "BluetoothManager.h"
#include "TelemetryVisualizer3D.h"
TelemetryVisualizer3D visualizer3d;
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
char bluetoothInputBuffer[256] = { '\n' };
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
	// create bluetooth manager // 
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
	// create the 3D telemetry visualizer //
	if (!visualizer3d.init())
	{
		return EXIT_FAILURE;
	}
	defer(visualizer3d.free());
//	Timer totalApplicationTimer;
	Timer frameTimer;
	while (k10::window->isOpen())
	{
		Timer currentFrameSleepTimer;
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
		visualizer3d.step(btManager.bluetoothSerialConnected());
		k10::guiDebugFrameMetrics->drawImGuiFrameMetrics(frameTime);
		ImGui::ShowDemoWindow();
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
				// 1 separator, 1 input text
				const float footer_height_to_reserve = 
					ImGui::GetStyle().ItemSpacing.y + 
						ImGui::GetFrameHeightWithSpacing(); 
				ImGui::BeginChild("ScrollingRegion", 
								  // Leave room for 1 separator + 1 InputText
								  ImVec2(0, -footer_height_to_reserve), false, 
								  ImGuiWindowFlags_HorizontalScrollbar); 
				// Tighten spacing
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
				if (!currentRawTelemetry.empty())
				{
					ImGui::TextUnformatted(currentRawTelemetry.data(), 
										   currentRawTelemetry.data() + 
												currentRawTelemetry.size());
				}
				if (btManager.extractNewTelemetryByteCount() > 0 &&
					ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				{
					ImGui::SetScrollHereY(1.f);
				}
				ImGui::PopStyleVar();
				ImGui::EndChild();
				ImGui::Separator();
				bool reclaimInputFocus = false;
				if (ImGui::InputText("BluetoothInput", bluetoothInputBuffer, 
									 IM_ARRAYSIZE(bluetoothInputBuffer), 
									 ImGuiInputTextFlags_EnterReturnsTrue))
				{
					//TODO: console interface with the modem
					strcpy_s(bluetoothInputBuffer, "");
					//bluetoothInputBuffer[0] = '\n';
					reclaimInputFocus = true;
				}
				// the console input should be this window's default focus item
				ImGui::SetItemDefaultFocus();
				if (reclaimInputFocus)
				{
					// set the focus on the previous item
					ImGui::SetKeyboardFocusHere(-1);
				}
				ImGui::End();
			}
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
		// Attempt to intelligently sleep the render thread under the 
		//	assumption that buffer swap is relatively fast //
		{
			const i32 elapsedMs = k10::OS_SCHEDULING_OVERHEAD_MS +
				currentFrameSleepTimer.getElapsedTime().milliseconds();
			const int refreshRate = k10::window->getRefreshRate();
			const i32 targetMs = k10::window->isVSyncEnabled() ?
				1000 / refreshRate : k10::FIXED_FRAME_TIME.milliseconds();
			if (elapsedMs < targetMs)
			{
				SDL_Delay(targetMs - elapsedMs);
			}
		}
		k10::window->swapBuffer();
	}
	return EXIT_SUCCESS;
}