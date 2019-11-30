#pragma once
#include "Camera.h"
class TelemetryVisualizer3D
{
public:
	bool init();
	void free();
	void step(bool droneTelemetryConnected, float pitch, float roll, float yaw);
private:
	struct GlobalMatrixBlock
	{
		glm::mat4 projection;
		glm::mat4 view;
	};
private:
	GfxBuffer uboGlobalUniformBlock;
	float cameraOrbitDistance = 25;
	Camera camera = Camera(glm::normalize(v3f(1,1,1))* cameraOrbitDistance, v3f(0,0,0));
	GfxBuffer vboOriginMesh;
};

