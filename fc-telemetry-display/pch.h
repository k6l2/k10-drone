#pragma once
#include <SDL.h>
using u8 = Uint8;
using u16 = Uint16;
using u32 = Uint32;
using u64 = Uint64;
using i8 = Sint8;
using i16 = Sint16;
using i32 = Sint32;
using i64 = Sint64;
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
//#define GLM_FORCE_ALIGNED_GENTYPES
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/type_ptr.hpp"
//#include "glm/gtc/type_aligned.hpp"
using v2f = glm::vec2;
using v2i = glm::ivec2;
using v2u = glm::uvec2;
using v3f = glm::vec3;
using v3i = glm::ivec3;
using v3u = glm::uvec3;
//#define OPTICK_ENABLE_GPU_VULKAN false
//#define OPTICK_ENABLE_GPU_D3D12  false
//#include "optick/optick.h"
#include "stb/stb_image.h"
#include <cstdlib>
#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <chrono>
#include <filesystem>
#include <limits>
using std::numeric_limits;
class Window;
class GuiDebugFrameMetrics;
#include "AssetDatabase.h"
#include "GfxVertexArray.h"
class Input;
#include "Time.h"
namespace k10
{
	// GLOBAL CONSTANTS ///////////////////////////////////////////////////////
	static const Time FIXED_FRAME_TIME = Time::seconds(1.f / 60);
	static const float PI = glm::pi<float>();
	static const GLint SHADER_BINARY_FORMAT = GL_SHADER_BINARY_FORMAT_SPIR_V;
	static const v3f FORWARD  = {  0, -1,  0 };
	static const v3f RIGHT    = { -1,  0,  0 };
	static const v3f WORLD_UP = {  0,  0,  1 };
	// GLOBAL HELPER FUNCTIONS ////////////////////////////////////////////////
	template<class T>
	inline T clamp(T value, T min, T max)
	{
		return value < min ? min :
			value > max ? max :
			value;
	}
	vector<u8> readFile(string const& fileName);
	bool initializeGlobalVariables();
	bool loadGlobalAssets();
	// don't call this function, just use the glCheckError() call defined above
	GLenum glCheckError_(char const* file, int line);
	// calculate the projection of vA onto vB
	v3f project(v3f const& vA, v3f const& vB);
	float radiansBetween(v3f vA, v3f vB, bool normalizedA = false, bool normalizedB = false);
	// calculate the signed radians around the localUp axis with respect to a
	//	local flat coordinate plane that localForward lies on.
	// positive radians means vec is "to the left" of the localForward, while
	//	negative radians implies it is "to the right"
	float radiansAroundAxis(v3f const& vec, 
							v3f const& localUp, v3f const& localForward);
//	btVector3 toBt(v3f const& v);
//	v3f toGlm(btVector3 const& v);
//	glm::mat4 toGlm(btTransform const& xform);
	string getParentPath(string const& filePath);
	// GLOBAL APPLICATION DATA ////////////////////////////////////////////////
	extern Window* window;
	extern GuiDebugFrameMetrics* guiDebugFrameMetrics;
	extern Input* input;
	extern string applicationBasePath;
	// Requirements:
	//	UBO[0] = GlobalMatrixBlock
	//	VBO[0] = array of ModelAsset::Mesh::Vertex
	//	sampler[0] = diffuse texture
	extern GfxVertexArray vaoMesh;
	// GLOBAL APPLICATION ASSETS //////////////////////////////////////////////
	extern AssetDatabase assetDb;
}
#define glCheckError() k10::glCheckError_(__FILE__, __LINE__)
// c++ defer statement support courtesy of:
//	https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/ ///////////////
template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};
template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}
#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})
// END C++ defer statement support ////////////////////////////////////////////
