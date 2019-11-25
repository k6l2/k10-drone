#include "GfxBuffer.h"
bool GfxBuffer::create(BufferTarget bt, MemoryUsage mu,
	size_t numElements, size_t elementSize)
{
	destroy();
	glCreateBuffers(1, &bufferObjectHandle);
	// Initialize the buffer with no data //
	{
		glNamedBufferData(bufferObjectHandle, elementSize * numElements, 
						  nullptr, decodeGlUsage(mu));
	}
	bufTarget = bt;
	memUsage = mu;
	this->numElements = numElements;
	this->elementSize = elementSize;
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return true;
}
bool GfxBuffer::isCreated() const
{
	const bool retVal = glIsBuffer(bufferObjectHandle);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return retVal;
}
void GfxBuffer::destroy()
{
	if (!bufferObjectHandle)
	{
		return;
	}
	glDeleteBuffers(1, &bufferObjectHandle);
	bufferObjectHandle = NULL;
	numElements = 0;
	elementSize = 0;
}
bool GfxBuffer::upload(size_t elementOffset, size_t elementCount,
					   void const* elements)
{
	if (elementOffset + elementCount > numElements)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"elementOffset=%i + elementCount=%i > numElements=%i\n",
			elementOffset, elementCount, numElements);
		SDL_assert(false); return false;
	}
	glNamedBufferSubData(bufferObjectHandle, elementOffset * elementSize,
						 elementCount * elementSize, elements);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return true;
}
bool GfxBuffer::bind(GLuint bindPointIndex, 
					 size_t elementOffset, size_t elementCount) const
{
	if (bufTarget != BufferTarget::UNIFORMS)
	{
		SDL_assert(false); return false;
	}
	glBindBufferRange(decodeGlTarget(bufTarget), bindPointIndex,
					  bufferObjectHandle, elementOffset * elementSize,
					  elementCount * elementSize);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false); return false;
	}
	return true;
}
GLenum GfxBuffer::decodeGlTarget(BufferTarget bt)
{
	switch (bt)
	{
	case BufferTarget::VERTEX_ATTRIBUTES:
		return GL_ARRAY_BUFFER;
	case BufferTarget::VERTEX_INDICES:
		return GL_ELEMENT_ARRAY_BUFFER;
	case BufferTarget::UNIFORMS:
		return GL_UNIFORM_BUFFER;
	}
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
		"BufferTarget=%i could not be decoded!\n",
		static_cast<int>(bt));
	SDL_assert(false);
	return GL_ARRAY_BUFFER;
}
GLenum GfxBuffer::decodeGlUsage(MemoryUsage mu)
{
	switch (mu)
	{
	case MemoryUsage::STATIC:
		return GL_STATIC_DRAW;
	case MemoryUsage::DYNAMIC:
		return GL_DYNAMIC_DRAW;
	case MemoryUsage::STREAM:
		return GL_STREAM_DRAW;
	}
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
		"MemoryUsage=%i could not be decoded!\n",
		static_cast<int>(mu));
	SDL_assert(false);
	return GL_DYNAMIC_DRAW;
}
GLuint GfxBuffer::getBufferObjectHandle() const
{
	return bufferObjectHandle;
}
GfxBuffer::BufferTarget GfxBuffer::getBufferTarget() const
{
	return bufTarget;
}
size_t GfxBuffer::getNumElements() const
{
	return numElements;
}
size_t GfxBuffer::getElementSize() const
{
	return elementSize;
}