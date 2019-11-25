#pragma once
class GuiDebugFrameMetrics
{
public:
	void drawImGuiFrameMetrics(class Time const& frameDelta);
private:
	size_t maxFrameMetricCount = 60 * 5;
	vector<float> frameMilliseconds = vector<float>(maxFrameMetricCount);
	int frameMetricsOffset = 0;
};
