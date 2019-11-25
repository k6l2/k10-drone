#include "GuiDebugFrameMetrics.h"
#include "Time.h"
void GuiDebugFrameMetrics::drawImGuiFrameMetrics(Time const& frameDelta)
{
	// Append new frame metrics to our data buffer(s) //
	{
		frameMilliseconds[frameMetricsOffset] = 
			static_cast<float>(frameDelta.milliseconds());
		frameMetricsOffset = (frameMetricsOffset + 1) % maxFrameMetricCount;
	}
	// Calculate meta data //
	stringstream ssMetaMs;
	{
		float msMin = frameMilliseconds[0];
		float msMax = frameMilliseconds[0];
		float msAvg = frameMilliseconds[0];
		for (size_t f = 1; f < maxFrameMetricCount; f++)
		{
			if (frameMilliseconds[f] < msMin)
			{
				msMin = frameMilliseconds[f];
			}
			if (frameMilliseconds[f] > msMax)
			{
				msMax = frameMilliseconds[f];
			}
			msAvg += frameMilliseconds[f];
		}
		msAvg /= maxFrameMetricCount;
		ssMetaMs << "[" << (int)msMin << "-" << (int)msMax << 
					"]~" << (int)msAvg;
	}
	// Draw the ImGui data metrics //
	ImGui::Begin("DEBUG Frame Metrics", nullptr, 
				 ImGuiWindowFlags_NoTitleBar |
				 ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PlotHistogram("ms", frameMilliseconds.data(), 
						 static_cast<int>(maxFrameMetricCount),
						 frameMetricsOffset, ssMetaMs.str().c_str(),
						 FLT_MAX, FLT_MAX, { 0,50 });
	ImGui::End();
}