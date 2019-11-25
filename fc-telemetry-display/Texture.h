#pragma once
class Texture
{
public:
	bool createFromImage(class Image const& img);
	void free();
	v2i getSize() const;
	bool bind(GLuint textureUnit) const;
private:
	GLuint textureHandle = NULL;
};
