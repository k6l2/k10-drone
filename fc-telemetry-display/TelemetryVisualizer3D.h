#pragma once
#include "Camera.h"
class TelemetryVisualizer3D
{
public:
	bool init();
	void free();
	void step(bool droneTelemetryConnected);
private:
	struct GlobalMatrixBlock
	{
		glm::mat4 projection;
		glm::mat4 view;
	};
private:
	GfxBuffer uboGlobalUniformBlock;
	Camera camera = Camera(v3f(25,25,25), v3f(0,0,0));
	GfxBuffer vboOriginMesh;
};

