#include "TelemetryVisualizer3D.h"
#include "Window.h"
bool TelemetryVisualizer3D::init()
{
	if (!uboGlobalUniformBlock.create(GfxBuffer::BufferTarget::UNIFORMS,
									  GfxBuffer::MemoryUsage::DYNAMIC, 1, 
									  sizeof(GlobalMatrixBlock)))
	{
		SDL_assert(false);  return false;
	}
	uboGlobalUniformBlock.bind(0, 0, 1);
	if(!vboOriginMesh.create(GfxBuffer::BufferTarget::VERTEX_ATTRIBUTES,
							 GfxBuffer::MemoryUsage::STATIC, 6,
							 sizeof(ModelAsset::Mesh::Vertex)))
	{
		SDL_assert(false);  return false;
	}
	// create the origin vertices & upload to gfx device //
	{
		vector<ModelAsset::Mesh::Vertex> vertices(6);
		vertices[0].position = v3f(0, 0, 0);
		vertices[0].color    = Color::Red;
		vertices[1].position = v3f(1, 0, 0);
		vertices[1].color    = Color::Red;
		vertices[2].position = v3f(0, 0, 0);
		vertices[2].color    = Color::Green;
		vertices[3].position = v3f(0, 1, 0);
		vertices[3].color    = Color::Green;
		vertices[4].position = v3f(0, 0, 0);
		vertices[4].color    = Color::Blue;
		vertices[5].position = v3f(0, 0, 1);
		vertices[5].color    = Color::Blue;
		if (!vboOriginMesh.upload(0, vertices.size(), vertices.data()))
		{
			SDL_assert(false);  return false;
		}
	}
	return true;
}
void TelemetryVisualizer3D::free()
{
	uboGlobalUniformBlock.destroy();
}
void TelemetryVisualizer3D::step(bool droneTelemetryConnected)
{
	// populate a GlobalMatrixBlock uniform buffer which is bound to global 
	//	OpenGL context //
	{
//		OPTICK_EVENT("Simulation::draw::uploadGlobalMatrixBlock");
		const v2i windowSize = k10::window->getSize();
		GlobalMatrixBlock gmb;
		gmb.projection = 
			glm::perspective(camera.getFieldOfViewRadians(),
							 static_cast<float>(windowSize.x) / windowSize.y, 
							 camera.getClipDistanceNear(), 
							 camera.getClipDistanceFar());
		gmb.view = camera.getTransform();
		uboGlobalUniformBlock.upload(0, 1, &gmb);
	}
	// draw the origin //
	{
		glm::mat4 originModel = glm::scale(glm::mat4(1.f), v3f(10,10,10));
		k10::assetDb.getGfxProgram("simple-draw")->setUniform(0, originModel);
		k10::vaoMesh.bindVertexBuffer(vboOriginMesh, 0, 0);
		if (!GfxProgram::use(k10::assetDb.getGfxProgram("simple-draw")))
		{
			SDL_assert(false);
		}
		GfxVertexArray::use(&k10::vaoMesh);
		glDrawArrays(GL_LINES, 0, 6);
	}
	if (droneTelemetryConnected)
	{
		// draw the 3D visualization of the drone's orientation //
		static char const* const gfxProgName = "simple-draw-textured";
		static char const* const modelName   = "droid-fighter/droid-fighter.fbx";
		glm::mat4 meshModel = glm::mat4(1.f);
		k10::assetDb.getGfxProgram(gfxProgName)->setUniform(0, meshModel);
		if (!GfxProgram::use(k10::assetDb.getGfxProgram(gfxProgName)))
		{
			SDL_assert(false);
		}
		const bool renderSuccess = k10::assetDb.getModel(modelName)->render();
		if (!renderSuccess)
		{
			SDL_assert(false);
		}
	}
}