#pragma once
class TextureCubemap
{
public:
	// each index of imgFaces corresponds to a face of the cube map in order:
	//	[+X, -X, +Y, -Y, +Z, -Z]
	bool createFromImages(class Image const imgFaces[6]);
	void free();
	//v2i getSize() const;
	bool bind(GLuint textureUnit) const;
private:
	GLuint textureHandle = NULL;
};
