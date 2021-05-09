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
	// pack the telemetry packet structure so we can safely union it with an
	//	array of characters to fill in the binary data from a serial stream.
#pragma pack(push, 1) 
//	struct i16v3
//	{
//		i16 x;
//		i16 y;
//		i16 z;
//	};
	struct TelemetryPacket
	{
		u32 microseconds;
		u32 microsecondsDelta;
		v3f gForce;
		v3f gForceMedian;
		v3f radiansPerSecond;
		v3f relativeOrientationRadians;
	};
#pragma pack(pop)
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
	TelemetryPacket const& getLatestCompleteTelemetryPacket() const;
	void drawImGuiFrameMetrics() const;
private:
	static int bluetoothManagerThreadMain(void* pBluetoothManager);
private:
	union TelemetryPacketUnion
	{
		TelemetryPacket tp;
		char rawBytes[sizeof(TelemetryPacket)];
	};
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
	bool deviceInquiryRequested = true;
	string requestedDeviceAddress = "";
	static const size_t MAX_RAW_TELEMETRY_BUFFER_SIZE = 1000000;
	vector<char> rawTelemetry;
	size_t newRawTelemetryBytes = 0;
	vector<char> telemetrySendBuffer;
	static const string TELEMETRY_PACKET_HEADER;
	TelemetryPacketUnion telemetryPacket;
	TelemetryPacket latestCompleteTelemetryPacket;
	u8 numHeaderBytesRead = 0;
	size_t numTelemetryPacketBytesRead = 0;
	// debug telemetry GUI ////////////////////////////////////////////////////
	size_t maxFrameMetricCount = 60 * 5;
	vector<float> frameMicrosecondsDelta           = vector<float>(maxFrameMetricCount);
	vector<float> frameRadiansPerSecondX           = vector<float>(maxFrameMetricCount);
	vector<float> frameRadiansPerSecondY           = vector<float>(maxFrameMetricCount);
	vector<float> frameRadiansPerSecondZ           = vector<float>(maxFrameMetricCount);
	vector<float> frameRelativeOrientationRadiansX = vector<float>(maxFrameMetricCount);
	vector<float> frameRelativeOrientationRadiansY = vector<float>(maxFrameMetricCount);
	vector<float> frameRelativeOrientationRadiansZ = vector<float>(maxFrameMetricCount);
	vector<float> frameGForceX                     = vector<float>(maxFrameMetricCount);
	vector<float> frameGForceY                     = vector<float>(maxFrameMetricCount);
	vector<float> frameGForceZ                     = vector<float>(maxFrameMetricCount);
	vector<float> frameGForceMedianX               = vector<float>(maxFrameMetricCount);
	vector<float> frameGForceMedianY               = vector<float>(maxFrameMetricCount);
	vector<float> frameGForceMedianZ               = vector<float>(maxFrameMetricCount);
	int frameMetricsOffset = 0;
};