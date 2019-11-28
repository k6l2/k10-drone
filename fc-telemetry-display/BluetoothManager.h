/*
	Performs the following functions on a separate thread:
		-Bluetooth Device Inquiry
		-Bluetooth Serial I/O
*/
#pragma once
#include "OrderedLock.h"
class BluetoothManager
{
public:
	void initialize();
	void free();
	void lock(SDL_cond* conditionVariable);
	void unlock(SDL_cond* conditionVariable);
	bool bluetoothSerialConnected() const;
	void requestDeviceInquiry();
	bool isQueryingForDevices() const;
	vector<device> const& getDeviceList() const;
	void requestBluetoothConnection(string const& deviceAddress);
	bool isRequestingBtConnection() const;
	vector<char> const& getRawTelemetry() const;
	size_t extractNewTelemetryByteCount();
	void sendData(char const* data, size_t size);
private:
	static int bluetoothManagerThreadMain(void* pBluetoothManager);
private:
	// Thread data ////////////////////////////////////////////////////////////
	SDL_Thread* thread = nullptr;
	OrderedLock orderedThreadLock;
	SDL_cond* btMgrLockConditionVar;
	bool threadRunning = false;
	// Bluetooth data /////////////////////////////////////////////////////////
	DeviceINQ* devInq = nullptr;
	BTSerialPortBinding* btSerial = nullptr;
	vector<device> devices;
	bool deviceInquiryRequested = false;
	string requestedDeviceAddress = "";
	static const size_t MAX_RAW_TELEMETRY_BUFFER_SIZE = 1000000;
	vector<char> rawTelemetry;
	size_t newRawTelemetryBytes = 0;
	vector<char> telemetrySendBuffer;
};
