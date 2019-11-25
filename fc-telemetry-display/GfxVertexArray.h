#pragma once
class GfxVertexArray
{
public:
	static bool use(GfxVertexArray const* const vao);
public:
	bool create();
	void destroy();
	// This should theoretically only be required to be called ONCE for each
	//	attribute for the entire lifespan of a VAO.
	// enables vertex attrib array for this attribute index
	//	establishes a vertex attribute <=> buffer index binding (hopefully)
	//	set up data format description for this attribute (hopefully)
	bool defineAttribute(char const* name, GLuint attributeIndex, 
						 GLuint bufferBindIndex, GLint size,
						 GLenum type, GLboolean normalized, 
						 GLuint relativeoffset);
	// I think this might need to be called once per frame per bufferBindIndex?
	bool bindVertexBuffer(class GfxBuffer const& vbo, 
						  GLuint bufferBindIndex,
						  size_t elementOffset) const;
	bool bindElementBuffer(class GfxBuffer const& ebo) const;
private:
	struct Attribute
	{
		GLuint attributeIndex;
		GLuint bufferBindIndex;
	};
private:
	GLuint vertexArrayObjectHandle = NULL;
	map<string, Attribute> attributes;
};
