#include "pch.h"
#include "Window.h"
#include "GuiDebugFrameMetrics.h"
#include "GfxProgram.h"
#include "AssetDatabase.h"
#include "ModelAsset.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "Input.h"
namespace k10
{
	Window* window;
	GuiDebugFrameMetrics* guiDebugFrameMetrics;
	Input* input;
	string applicationBasePath;
	GfxVertexArray vaoMesh;
	AssetDatabase assetDb;
}
vector<u8> k10::readFile(string const& fileName)
{
	vector<u8> retVal;
	SDL_RWops* file = SDL_RWFromFile(fileName.c_str(), "rb");
	if (!file)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to open file '%s'!\n", fileName.c_str());
		SDL_assert(false);
		return {};
	}
	const i64 fileSize = SDL_RWseek(file, 0, RW_SEEK_END);
	if (fileSize < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to get file size of '%s'!\n", fileName.c_str());
		SDL_assert(false);
		SDL_RWclose(file);
		return {};
	}
	if (SDL_RWseek(file, 0, RW_SEEK_SET) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to seek to file start for '%s'!\n", fileName.c_str());
		SDL_assert(false);
		SDL_RWclose(file);
		return {};
	}
	retVal.resize(static_cast<size_t>(fileSize));
	size_t currentReadByte = 0;
	while (true)
	{
		SDL_assert(retVal.size() >= currentReadByte);
		size_t maxBytesLeft = retVal.size() - currentReadByte;
		if (maxBytesLeft == 0)
		{
			// There are no more bytes to read.  We're done!
			break;
		}
		const size_t bytesRead = SDL_RWread(file, 
											&retVal[currentReadByte], 
											sizeof(Uint8), 
											maxBytesLeft);
		currentReadByte += bytesRead;
		if (bytesRead <= 0)
		{
			// the only time bytesRead is 0 is on error, or if we reached 
			//	EOF, so there's no point in continuing...
			break;
		}
	}
	SDL_RWclose(file);
	if (currentReadByte != retVal.size())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to read file '%s'!\n", fileName.c_str());
		SDL_assert(false);
		return {};
	}
	return retVal;
}
bool k10::initializeGlobalVariables()
{
	window = Window::create("Flight Controller Telemetry", 1280, 720);
	if (!window)
	{
		return false;
	}
	guiDebugFrameMetrics = new GuiDebugFrameMetrics;
	input = new Input;
	{
		char* basePath = SDL_GetBasePath();
		if (!basePath)
		{
			return false;
		}
		applicationBasePath = basePath ? basePath : "";
#ifndef NDEBUG
		applicationBasePath += "../../";
#endif
		SDL_free(basePath);
	}
	if (!vaoMesh.create())
	{
		SDL_assert(false); return EXIT_FAILURE;
	}
	if (!vaoMesh.defineAttribute("vertexPosition", 0, 0, 
								 3, GL_FLOAT, GL_FALSE, 
								 offsetof(ModelAsset::Mesh::Vertex, position)))
	{
		SDL_assert(false); return EXIT_FAILURE;
	}
	if (!vaoMesh.defineAttribute("vertexTexCoord", 1, 0, 
								 2, GL_FLOAT, GL_FALSE, 
								 offsetof(ModelAsset::Mesh::Vertex, texCoord)))
	{
		SDL_assert(false); return EXIT_FAILURE;
	}
	if (!vaoMesh.defineAttribute("vertexColor", 2, 0, 
								 4, GL_UNSIGNED_BYTE, GL_TRUE, 
								 offsetof(ModelAsset::Mesh::Vertex, color)))
	{
		SDL_assert(false); return EXIT_FAILURE;
	}
	if (!vaoMesh.defineAttribute("vertexNormal", 3, 0, 
								 3, GL_FLOAT, GL_FALSE, 
								 offsetof(ModelAsset::Mesh::Vertex, normal)))
	{
		SDL_assert(false); return EXIT_FAILURE;
	}
	return true;
}
bool k10::loadGlobalAssets()
{
	if (!assetDb.load())
	{
		return false;
	}
	return true;
}
GLenum k10::glCheckError_(char const* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		string errorString;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  
			errorString = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 
			errorString = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             
			errorString = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                
			errorString = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               
			errorString = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 
			errorString = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: 
			errorString = "INVALID_FRAMEBUFFER_OPERATION"; break;
		default:
			errorString = "UNKNOWN_ERROR"; break;
		}
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"%s:%i: OpenGL error:'%s'", file, line, errorString.c_str());
	}
	return errorCode;
}
v3f k10::project(v3f const& vA, v3f const& vB)
{
	const v3f normB = glm::normalize(vB);
	const float scalarProjection = glm::dot(vA, normB);
	return scalarProjection * normB;
}
float k10::radiansBetween(v3f vA, v3f vB, bool normalizedA, bool normalizedB)
{
	if (!normalizedA)
	{
		vA = glm::normalize(vA);
	}
	if (!normalizedB)
	{
		vB = glm::normalize(vB);
	}
	float const dotProd = glm::dot(vA, vB);
	return glm::acos(clamp(dotProd, -1.f, 1.f));
}
float k10::radiansAroundAxis(v3f const& vec, 
							 v3f const& localUp, v3f const& localForward)
{
	const v3f localRight = glm::cross(localForward, localUp);
	const v3f vecUpComp = k10::project(vec, localUp);
	const v3f planarVec = vec - vecUpComp;
	const float radiansFromForward =
		radiansBetween(planarVec, localForward, false, true);
	const bool facingRight = glm::dot(planarVec, localRight) >= 0;
	return radiansFromForward * (facingRight ? -1 : 1);
}
//btVector3 k10::toBt(v3f const& v)
//{
//	return btVector3(v.x, v.y, v.z);
//}
//v3f k10::toGlm(btVector3 const& v)
//{
//	return v3f(v.getX(), v.getY(), v.getZ());
//}
//glm::mat4 k10::toGlm(btTransform const& xform)
//{
//	// slightly modified from this forum post:
//	//	https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=11462
//	glm::mat4 m;
//	btMatrix3x3 const& basis = xform.getBasis();
//	// rotation
//	for (int r = 0; r < 3; r++)
//	{
//		for (int c = 0; c < 3; c++)
//		{
//			m[c][r] = basis[r][c];
//		}
//	}
//	// traslation
//	btVector3 const& origin = xform.getOrigin();
//	m[3][0] = origin.getX();
//	m[3][1] = origin.getY();
//	m[3][2] = origin.getZ();
//	// unit scale
//	m[0][3] = 0.0f;
//	m[1][3] = 0.0f;
//	m[2][3] = 0.0f;
//	m[3][3] = 1.0f;
//	return m;
//}
string k10::getParentPath(string const& filePath)
{
	std::filesystem::path pathToFile(filePath);
	if (!std::filesystem::is_regular_file(pathToFile))
	{
		SDL_assert(false);
		return applicationBasePath;
	}
	return pathToFile.parent_path().string();
}