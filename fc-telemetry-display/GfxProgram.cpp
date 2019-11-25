#include "GfxProgram.h"
bool GfxProgram::use(GfxProgram const*const gProg,
					 bool enableDepthTest,
					 GLenum depthTestFunc,
					 bool enableWireframe,
					 bool enableBackfaceCulling,
					 GLenum cullFace)
{
	if (!gProg)
	{
		glUseProgram(NULL);
		return true;
	}
	glUseProgram(gProg->programId);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (enableDepthTest)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(depthTestFunc);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
	if (enableBackfaceCulling)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(cullFace);
		glFrontFace(GL_CCW);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
	if (enableWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (glCheckError() != GL_NO_ERROR)
	{
		return false;
	}
	return true;
}
bool GfxProgram::load(string const& shaderSpirvBinaryFilenameVertex,
					  string const& shaderSpirvBinaryFilenameFragment)
{
	free();
	// Create and compile the VERTEX shader! //
	const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (vertexShader == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to create shader object!\n");
		return false;
	}
	{
		vector<Uint8> shaderBin = k10::readFile(shaderSpirvBinaryFilenameVertex);
		glShaderBinary(1, &vertexShader, k10::SHADER_BINARY_FORMAT,
					   shaderBin.data(), static_cast<GLsizei>(shaderBin.size()));
		if (glCheckError() != GL_NO_ERROR)
		{
			printLogShader(vertexShader);
			return false;
		}
		glSpecializeShader(vertexShader, "main", 0, nullptr, nullptr);
		GLint vShaderCompiled = GL_FALSE;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
		if (vShaderCompiled != GL_TRUE)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
						 "Unable to compile vertex shader %d!\n", vertexShader);
			printLogShader(vertexShader);
			return false;
		}
	}
	// Create and compile the FRAGMENT shader! //
	const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragmentShader == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to create shader object!\n");
		return false;
	}
	{
		vector<Uint8> shaderBin = k10::readFile(shaderSpirvBinaryFilenameFragment);
		glShaderBinary(1, &fragmentShader, k10::SHADER_BINARY_FORMAT,
					   shaderBin.data(), static_cast<GLsizei>(shaderBin.size()));
		if (glCheckError() != GL_NO_ERROR)
		{
			printLogShader(fragmentShader);
			return false;
		}
		glSpecializeShader(fragmentShader, "main", 0, nullptr, nullptr);
		GLint fShaderCompiled = GL_FALSE;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
		if (fShaderCompiled != GL_TRUE)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
						 "Unable to compile vertex shader %d!\n", fragmentShader);
			printLogShader(fragmentShader);
			return false;
		}
	}
	// Create the shader program and link the vertex/fragment shaders! //
	programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);
	GLint programSuccess = GL_TRUE;
	glGetProgramiv(programId, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, 
					 "Error linking program %d!\n", programId);
		printLogProgram(programId);
		return false;
	}
	// cleanup shaders since program is now loaded //
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	if (glCheckError() != GL_NO_ERROR)
	{
		return false;
	}
	return true;
}
void GfxProgram::free()
{
	if (programId != NULL)
	{
		glDeleteProgram(programId);
	}
	programId = NULL;
}
void GfxProgram::printLogProgram(GLuint program)
{
	printLogGeneric(program, false);
}
void GfxProgram::printLogShader(GLuint shader)
{
	printLogGeneric(shader, true);
}
void GfxProgram::printLogGeneric(GLuint glObjectId, bool isShader)
{
	if (( isShader && !glIsShader (glObjectId)) ||
		(!isShader && !glIsProgram(glObjectId)))
	{
		SDL_Log("Name %d is not a %s\n",
				glObjectId, isShader?"shader":"program");
		return;
	}
	int infoLogLength = 0;
	int maxLength = infoLogLength;
	if (isShader)
	{
		glGetShaderiv(glObjectId, GL_INFO_LOG_LENGTH, &maxLength);
	}
	else
	{
		glGetProgramiv(glObjectId, GL_INFO_LOG_LENGTH, &maxLength);
	}
	char* infoLog = new char[maxLength];
	if (isShader)
	{
		glGetShaderInfoLog(glObjectId, maxLength, &infoLogLength, infoLog);
	}
	else
	{
		glGetProgramInfoLog(glObjectId, maxLength, &infoLogLength, infoLog);
	}
	if (infoLogLength > 0)
	{
		SDL_Log("%s\n", infoLog);
	}
	delete[] infoLog;
}
bool GfxProgram::setUniform(GLint location, float value) const
{
	glProgramUniform1f(programId, location, value);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false);
		return false;
	}
	return true;
}
bool GfxProgram::setUniform(GLint location, glm::mat4 value, 
							GLboolean transpose) const
{
	glProgramUniformMatrix4fv(programId, location, 1, transpose, 
							  glm::value_ptr(value));
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false);
		return false;
	}
	return true;
}