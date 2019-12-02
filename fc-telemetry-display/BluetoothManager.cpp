#include "BluetoothManager.h"
#include <BluetoothException.h>
#include "Timer.h"
const string BluetoothManager::TELEMETRY_PACKET_HEADER = "FCTP";
void BluetoothManager::initialize()
{
	free();
	devInq = DeviceINQ::Create();
	threadRunning = true;
	btMgrLockConditionVar = SDL_CreateCond();
	if (!btMgrLockConditionVar)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to create condition variable: '%s'\n", 
					 SDL_GetError());
		SDL_assert(false);
	}
	if (!orderedThreadLock.initialize())
	{
		SDL_assert(false);
	}
	thread = SDL_CreateThread(BluetoothManager::bluetoothManagerThreadMain,
							  "BluetoothManagerThread", this);
	if (!thread)
	{
		SDL_assert(false);
	}
}
void BluetoothManager::free()
{
	threadRunning = false;
	if (thread)
	{
		int retVal;
		SDL_WaitThread(thread, &retVal);
	}
	thread = nullptr;
	orderedThreadLock.free();
	if (btMgrLockConditionVar)
	{
		SDL_DestroyCond(btMgrLockConditionVar);
	}
	btMgrLockConditionVar = nullptr;
	if (devInq)
	{
		delete devInq;
	}
	devInq = nullptr;
}
void BluetoothManager::lock(SDL_cond* conditionVariable)
{
	orderedThreadLock.lock(conditionVariable);
}
void BluetoothManager::unlock(SDL_cond* conditionVariable)
{
	orderedThreadLock.unlock(conditionVariable);
}
bool BluetoothManager::bluetoothSerialConnected() const
{
	return btSerial != nullptr;
}
void BluetoothManager::requestDeviceInquiry()
{
	deviceInquiryRequested = true;
}
bool BluetoothManager::isQueryingForDevices() const
{
	return deviceInquiryRequested;
}
vector<device> const& BluetoothManager::getDeviceList() const
{
	return devices;
}
void BluetoothManager::requestBluetoothConnection(string const& deviceAddress)
{
	requestedDeviceAddress = deviceAddress;
	if (btSerial)
	{
		SDL_assert(false);
	}
}
bool BluetoothManager::isRequestingBtConnection() const
{
	return requestedDeviceAddress != "" && !btSerial;
}
vector<char> const& BluetoothManager::getRawTelemetry() const
{
	return rawTelemetry;
}
size_t BluetoothManager::extractNewTelemetryByteCount()
{
	const size_t retVal = newRawTelemetryBytes;
	newRawTelemetryBytes = 0;
	return retVal;
}
void BluetoothManager::sendData(char const* data, size_t size)
{
	for (size_t c = 0; c < size; c++)
	{
		if (telemetrySendBuffer.size() >= MAX_RAW_TELEMETRY_BUFFER_SIZE)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						 "telemetrySendBuffer overflow!\n");
			SDL_assert(false); return;
		}
		telemetrySendBuffer.push_back(data[c]);
	}
	if (!(size == 3 && strcmp(data, "$$$") == 0))
	{
		telemetrySendBuffer.push_back('\n');
	}
}
BluetoothManager::TelemetryPacket const& 
	BluetoothManager::getLatestCompleteTelemetryPacket() const
{
	return latestCompleteTelemetryPacket;
}
void BluetoothManager::drawImGuiFrameMetrics() const
{
	// Draw the ImGui data metrics //
	ImGui::Begin("Telemetry Frame Metrics");// , nullptr,
				 //ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PlotHistogram("deltaMicros", frameMicrosecondsDelta.data(),
					     static_cast<int>(maxFrameMetricCount),
					     frameMetricsOffset, nullptr, FLT_MAX, FLT_MAX, {0,100});
	ImGui::PlotLines("gyroX", frameRadiansPerSecondX.data(),
					 static_cast<int>(maxFrameMetricCount),
					 frameMetricsOffset, nullptr, FLT_MAX, FLT_MAX, {0,100});
	ImGui::PlotLines("gyroY", frameRadiansPerSecondY.data(),
					 static_cast<int>(maxFrameMetricCount),
					 frameMetricsOffset, nullptr, FLT_MAX, FLT_MAX, { 0,100 });
	ImGui::PlotLines("gyroZ", frameRadiansPerSecondZ.data(),
					 static_cast<int>(maxFrameMetricCount),
					 frameMetricsOffset, nullptr, FLT_MAX, FLT_MAX, { 0,100 });
	ImGui::PlotLines("orientRadiansX", frameRelativeOrientationRadiansX.data(),
					 static_cast<int>(maxFrameMetricCount),
					 frameMetricsOffset, nullptr, FLT_MAX, FLT_MAX, {0,100});
	ImGui::PlotLines("orientRadiansY", frameRelativeOrientationRadiansY.data(),
					 static_cast<int>(maxFrameMetricCount),
					 frameMetricsOffset, nullptr, FLT_MAX, FLT_MAX, { 0,100 });
	ImGui::PlotLines("orientRadiansZ", frameRelativeOrientationRadiansZ.data(),
					 static_cast<int>(maxFrameMetricCount),
					 frameMetricsOffset, nullptr, FLT_MAX, FLT_MAX, { 0,100 });
	ImGui::End();
}
int BluetoothManager::bluetoothManagerThreadMain(void* pBluetoothManager)
{
	BluetoothManager* const btm = 
		static_cast<BluetoothManager*>(pBluetoothManager);
	while (true)
	{
		Timer simulationFrameTimer;
		btm->orderedThreadLock.lock(btm->btMgrLockConditionVar);
		if (!btm->threadRunning)
		{
			return 0;
		}
		const bool doDeviceInquery = btm->deviceInquiryRequested;
		const string requestedDevAddr = 
			btm->btSerial ? "" : btm->requestedDeviceAddress;
		vector<char> telemetrySendBuffer = btm->telemetrySendBuffer;
		btm->orderedThreadLock.unlock(btm->btMgrLockConditionVar);
		if (btm->btSerial)
		{
			static const size_t TEMP_BUFF_SIZE = 1024;
			char telemetryTempBuff[TEMP_BUFF_SIZE];
			int numBytesRead = 0;
			try
			{
				numBytesRead = btm->btSerial->Read(telemetryTempBuff,
												   TEMP_BUFF_SIZE);
			}
			catch (BluetoothException const& bte)
			{
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					"Read FAILURE! BluetoothException='%s'\n", bte.what());
			}
			if (numBytesRead > 0)
			{
				btm->orderedThreadLock.lock(btm->btMgrLockConditionVar);
				for (int c = 0; c < numBytesRead; c++)
				{
					// scan for telemetry packet header
					if (btm->numHeaderBytesRead < 
							TELEMETRY_PACKET_HEADER.size())
					{
						if (telemetryTempBuff[c] == 
							TELEMETRY_PACKET_HEADER[btm->numHeaderBytesRead])
						{
							btm->numHeaderBytesRead++;
						}
					}
					else 
					if(btm->numTelemetryPacketBytesRead < sizeof(TelemetryPacket))
					{
						btm->telemetryPacket.rawBytes[btm->numTelemetryPacketBytesRead] =
							telemetryTempBuff[c];
						btm->numTelemetryPacketBytesRead++;
						if (btm->numTelemetryPacketBytesRead >= 
								sizeof(TelemetryPacket))
						{
							// we've filled up a complete telemetry packet, so
							//	we can now extract the information! //
							btm->latestCompleteTelemetryPacket = 
								btm->telemetryPacket.tp;
//							SDL_Log("---TelemetryPacket---\n\t"
//								"milliseconds=%i "
//								"relativeOrientationRadians={%f,%f,%f} ",
//								btm->telemetryPacket.tp.milliseconds,
//								btm->telemetryPacket.tp.relativeOrientationRadians.x,
//								btm->telemetryPacket.tp.relativeOrientationRadians.y,
//								btm->telemetryPacket.tp.relativeOrientationRadians.z);
							// append new telemetry data to GUI buffers //
							btm->frameMicrosecondsDelta[btm->frameMetricsOffset] =
								static_cast<float>(btm->latestCompleteTelemetryPacket.microsecondsDelta);
							btm->frameRadiansPerSecondX[btm->frameMetricsOffset] =
								btm->latestCompleteTelemetryPacket.radiansPerSecond.x;
							btm->frameRadiansPerSecondY[btm->frameMetricsOffset] =
								btm->latestCompleteTelemetryPacket.radiansPerSecond.y;
							btm->frameRadiansPerSecondZ[btm->frameMetricsOffset] =
								btm->latestCompleteTelemetryPacket.radiansPerSecond.z;
							btm->frameRelativeOrientationRadiansX[btm->frameMetricsOffset] =
								btm->latestCompleteTelemetryPacket.relativeOrientationRadians.x;
							btm->frameRelativeOrientationRadiansY[btm->frameMetricsOffset] =
								btm->latestCompleteTelemetryPacket.relativeOrientationRadians.y;
							btm->frameRelativeOrientationRadiansZ[btm->frameMetricsOffset] =
								btm->latestCompleteTelemetryPacket.relativeOrientationRadians.z;
							btm->frameMetricsOffset = 
								(btm->frameMetricsOffset + 1) % 
									btm->maxFrameMetricCount;
							// reset the telemetry packet state machine to look
							//	for the next telemetry packet header //
							btm->numTelemetryPacketBytesRead = 0;
							btm->numHeaderBytesRead = 0;
						}
					}
					btm->rawTelemetry.push_back(telemetryTempBuff[c]);
				}
				if (btm->rawTelemetry.size() > MAX_RAW_TELEMETRY_BUFFER_SIZE)
				{
					size_t const excessSize = 
						btm->rawTelemetry.size() - 
							MAX_RAW_TELEMETRY_BUFFER_SIZE;
					btm->rawTelemetry.erase(btm->rawTelemetry.begin(),
											btm->rawTelemetry.begin() + excessSize);
				}
				btm->newRawTelemetryBytes += numBytesRead;
				btm->orderedThreadLock.unlock(btm->btMgrLockConditionVar);
			}
			if (!telemetrySendBuffer.empty())
			{
				int numBytesWritten = 0;
				try
				{
					numBytesWritten = 
						btm->btSerial->Write(telemetrySendBuffer.data(), 
											 static_cast<int>(telemetrySendBuffer.size()));
				}
				catch (BluetoothException const& bte)
				{
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						"Write FAILURE! BluetoothException='%s'\n", bte.what());
				}
				if (numBytesWritten > 0)
				{
					btm->orderedThreadLock.lock(btm->btMgrLockConditionVar);
					btm->telemetrySendBuffer.erase(btm->telemetrySendBuffer.begin(),
												   btm->telemetrySendBuffer.begin() +
														numBytesWritten);
					btm->orderedThreadLock.unlock(btm->btMgrLockConditionVar);
				}
			}
		}
		if (doDeviceInquery)
		{
			vector<device> const devices = btm->devInq->Inquire();
			btm->orderedThreadLock.lock(btm->btMgrLockConditionVar);
			btm->devices                = devices;
			btm->deviceInquiryRequested = false;
			btm->orderedThreadLock.unlock(btm->btMgrLockConditionVar);
		}
		else if (requestedDevAddr != "")
		{
			BTSerialPortBinding* btSerial = nullptr;
			try
			{
				SDL_Log("Binding to address '%s'...", 
						requestedDevAddr.c_str());
				btSerial = BTSerialPortBinding::Create(requestedDevAddr, 1);
				SDL_Log("SUCCESS!\n");
				btSerial->setTimoutRead(1);
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
					SDL_Log("Connecting to '%s'...", requestedDevAddr.c_str());
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
			btm->orderedThreadLock.lock(btm->btMgrLockConditionVar);
			btm->btSerial				= btSerial;
			btm->requestedDeviceAddress = "";
			btm->newRawTelemetryBytes   = 0;
			btm->rawTelemetry.clear();
			btm->orderedThreadLock.unlock(btm->btMgrLockConditionVar);
		}
		const i32 elapsedMs = k10::OS_SCHEDULING_OVERHEAD_MS +
						  simulationFrameTimer.getElapsedTime().milliseconds();
		if (elapsedMs < k10::FIXED_FRAME_TIME.milliseconds())
		{
			SDL_Delay(k10::FIXED_FRAME_TIME.milliseconds() - elapsedMs);
		}
	}
}