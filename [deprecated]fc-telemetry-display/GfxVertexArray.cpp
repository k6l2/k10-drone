#include "GfxVertexArray.h"
#include "GfxBuffer.h"
bool GfxVertexArray::use(GfxVertexArray const* const vao)
{
	if (!vao)
	{
		glBindVertexArray(NULL);
		return true;
	}
	glBindVertexArray(vao->vertexArrayObjectHandle);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return true;
}
bool GfxVertexArray::create()
{
	destroy();
	glCreateVertexArrays(1, &vertexArrayObjectHandle);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return true;
}
void GfxVertexArray::destroy()
{
	if (!vertexArrayObjectHandle)
	{
		return;
	}
	glDeleteVertexArrays(1, &vertexArrayObjectHandle);
	vertexArrayObjectHandle = NULL;
}
bool GfxVertexArray::defineAttribute(char const* name, GLuint attributeIndex,
									 GLuint bufferBindIndex, GLint size,
									 GLenum type, GLboolean normalized, 
									 GLuint relativeoffset)
{
	glEnableVertexArrayAttrib(vertexArrayObjectHandle, attributeIndex);
	glVertexArrayAttribFormat(vertexArrayObjectHandle, attributeIndex,
							  size, type, normalized, relativeoffset);
	glVertexArrayAttribBinding(vertexArrayObjectHandle, 
							   attributeIndex, bufferBindIndex);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	if (attributes.find(name) != attributes.end())
	{
		SDL_assert(false); return false;
	}
	Attribute newAttribute;
	newAttribute.attributeIndex  = attributeIndex;
	newAttribute.bufferBindIndex = bufferBindIndex;
	attributes.insert({name, newAttribute});
	return true;
}
bool GfxVertexArray::bindVertexBuffer(GfxBuffer const& vbo, 
									  GLuint bufferBindIndex, 
									  size_t elementOffset) const
{
	if (vbo.getBufferTarget() != GfxBuffer::BufferTarget::VERTEX_ATTRIBUTES)
	{
		SDL_assert(false); return false;
	}
	glVertexArrayVertexBuffer(vertexArrayObjectHandle,
							  bufferBindIndex, //attribIt->second.bufferBindIndex, 
							  vbo.getBufferObjectHandle(), 
							  static_cast<GLsizei>(vbo.getElementSize())*elementOffset, 
							  static_cast<GLsizei>(vbo.getElementSize()));
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return true;
}
bool GfxVertexArray::bindElementBuffer(GfxBuffer const& ebo) const
{
	if (ebo.getBufferTarget() != GfxBuffer::BufferTarget::VERTEX_INDICES)
	{
		SDL_assert(false); return false;
	}
	glVertexArrayElementBuffer(vertexArrayObjectHandle,
							   ebo.getBufferObjectHandle());
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return true;
}