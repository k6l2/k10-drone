#include "BluetoothManager.h"
#include <BluetoothException.h>
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
int BluetoothManager::bluetoothManagerThreadMain(void* pBluetoothManager)
{
	BluetoothManager* const btm = 
		static_cast<BluetoothManager*>(pBluetoothManager);
	while (true)
	{
		btm->orderedThreadLock.lock(btm->btMgrLockConditionVar);
		if (!btm->threadRunning)
		{
			return 0;
		}
		const bool doDeviceInquery = btm->deviceInquiryRequested;
		const string requestedDevAddr = 
			btm->btSerial ? "" : btm->requestedDeviceAddress;
		btm->orderedThreadLock.unlock(btm->btMgrLockConditionVar);
		if (btm->btSerial)
		{
			static const size_t TEMP_BUFF_SIZE = 1024;
			char telemetryTempBuff[TEMP_BUFF_SIZE];
			int numBytesRead;
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
		SDL_Delay(15);
	}
}