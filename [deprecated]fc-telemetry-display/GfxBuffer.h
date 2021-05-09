#pragma once
class GfxBuffer
{
public:
	enum class BufferTarget : u8
	{
		VERTEX_ATTRIBUTES,
		VERTEX_INDICES,
		UNIFORMS
	};
	enum class MemoryUsage : u8
	{
		STATIC,
		DYNAMIC,
		STREAM
	};
public:
	bool create(BufferTarget bt, MemoryUsage mu, 
				size_t numElements, size_t elementSize);
	bool isCreated() const;
	void destroy();
	bool upload(size_t elementOffset, size_t elementCount, 
				void const* elements);
	GLuint getBufferObjectHandle() const;
	BufferTarget getBufferTarget() const;
	size_t getNumElements() const;
	size_t getElementSize() const;
	bool bind(GLuint bindPointIndex, 
			  size_t elementOffset, size_t elementCount) const;
private:
	static GLenum decodeGlTarget(BufferTarget bt);
	static GLenum decodeGlUsage(MemoryUsage mu);
private:
	GLuint bufferObjectHandle = NULL;
	BufferTarget bufTarget;
	MemoryUsage memUsage;
	size_t numElements = 0;
	size_t elementSize = 0;
};
