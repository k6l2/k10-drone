#include "Window.h"
#include "Timer.h"
#include "Input.h"
#include "GuiDebugFrameMetrics.h"
#include <BluetoothException.h>
DeviceINQ* const devInq = DeviceINQ::Create();
vector<device> devices;
BTSerialPortBinding* btSerial = nullptr;
enum class ApplicationState : u8
{
	DEFAULT,
	CONNECTING_TO_BLUETOOTH_DEVICE
};
ApplicationState appState = ApplicationState::DEFAULT;
char const* selectedBtConnectDevice = nullptr;
string selectedBtConnectAddress;
static const size_t MAX_RAW_TELEMETRY_BUFFER_SIZE = 1000000;
vector<char> rawTelemetry;
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
		// Main Menu GUI //
		const bool enableMenuItems = (appState == ApplicationState::DEFAULT);
		if (ImGui::BeginMainMenuBar())
		{
			if (btSerial)
			{
				//TODO: disconnect
			}
			else
			{
				if (ImGui::MenuItem("Connect", nullptr, nullptr, enableMenuItems))
				{
					selectedBtConnectDevice = nullptr;
					SDL_Log("Querying bluetooth devices...\n");
					devices = devInq->Inquire();
					for (auto const& d : devices)
					{
						if (!selectedBtConnectDevice)
						{
							selectedBtConnectDevice = d.name.c_str();
						}
						SDL_Log("device '%s' @ '%s'\n", d.name.c_str(), d.address.c_str());
					}
					appState = ApplicationState::CONNECTING_TO_BLUETOOTH_DEVICE;
				}
			}
			ImGui::EndMainMenuBar();
		}
		switch (appState)
		{
		case ApplicationState::DEFAULT: {
			if (btSerial)
			{
				static const size_t TEMP_BUFF_SIZE = 1024;
				char telemetryTempBuff[TEMP_BUFF_SIZE];
				int numBytesRead;
				try
				{
					numBytesRead = btSerial->Read(telemetryTempBuff, 
												  TEMP_BUFF_SIZE);
				}
				catch (BluetoothException const& bte)
				{
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						"Read FAILURE! BluetoothException='%s'\n", bte.what());
				}
				if (numBytesRead > 0)
				{
					for (int c = 0; c < numBytesRead; c++)
					{
						rawTelemetry.push_back(telemetryTempBuff[c]);
					}
					if (rawTelemetry.size() > MAX_RAW_TELEMETRY_BUFFER_SIZE)
					{
						size_t const excessSize = 
							rawTelemetry.size() - MAX_RAW_TELEMETRY_BUFFER_SIZE;
						rawTelemetry.erase(rawTelemetry.begin(), 
										   rawTelemetry.begin() + excessSize);
					}
				}
				ImGui::Begin("Raw Telemetry");
				ImGui::TextUnformatted(rawTelemetry.data(), 
									   rawTelemetry.data() + rawTelemetry.size());
				if (numBytesRead > 0)
				{
					ImGui::SetScrollHereY(1.f);
				}
				ImGui::End();
				//TODO: console interface with the modem
			}
		}	break;
		case ApplicationState::CONNECTING_TO_BLUETOOTH_DEVICE: {
			bool open;
			ImGui::Begin("Connect to Bluetooth Device", &open);
			if (ImGui::BeginCombo("devices", selectedBtConnectDevice))
			{
				for (auto const& d : devices)
				{
					bool isSelected = (selectedBtConnectDevice == d.name.c_str());
					if (ImGui::Selectable(d.name.c_str(), isSelected))
					{
						selectedBtConnectDevice = d.name.c_str();
						selectedBtConnectAddress = d.address;
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Connect"))
			{
				try
				{
					SDL_Log("Binding to address '%s'...", selectedBtConnectAddress.c_str());
					btSerial = BTSerialPortBinding::Create(selectedBtConnectAddress, 1);
					SDL_Log("SUCCESS!\n");
				}
				catch (BluetoothException const& bte)
				{
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						"FAILURE! BluetoothException='%s'\n", bte.what());
				}
				if (btSerial)
				{
					try
					{
						SDL_Log("Connecting to '%s'...", selectedBtConnectDevice);
						btSerial->Connect();
						SDL_Log("SUCCESS!\n");
					}
					catch (BluetoothException const& bte)
					{
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
							"FAILURE! BluetoothException='%s'\n", bte.what());
						delete btSerial; btSerial = nullptr;
					}
				}
				if (btSerial)
				{
					open = false;
				}
			}
			ImGui::End();
			if (!open)
			{
				appState = ApplicationState::DEFAULT;
			}
		}	break;
		}
		k10::window->swapBuffer();
	}
	return EXIT_SUCCESS;
}