#pragma once
// Gfx programs use modern OpenGL (4.6+) compiled shader binaries using a
//	format called SPIR-V.  See the following wiki page:
//	https://www.khronos.org/opengl/wiki/SPIR-V
class GfxProgram
{
public:
	static bool use(GfxProgram const*const gProg,
					bool enableDepthTest = true, 
					GLenum depthTestFunc = GL_LESS,
					bool enableWireframe = false, 
					bool enableBackfaceCulling = true,
					GLenum cullFace = GL_BACK);
public:
	bool load(string const& shaderSpirvBinaryFilenameVertex,
			  string const& shaderSpirvBinaryFilenameFragment);
	void free();
	bool setUniform(GLint location, float value) const;
	bool setUniform(GLint location, glm::mat4 value, 
					GLboolean transpose = GL_FALSE) const;
private:
	void printLogProgram(GLuint program);
	void printLogShader(GLuint shader);
	void printLogGeneric(GLuint glObjectId, bool isShader);
private:
	GLuint programId = NULL;
};
