#include "TextureCubemap.h"
#include "Image.h"
bool TextureCubemap::createFromImages(Image const imgFaces[6])
{
	free();
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureHandle);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(textureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	for (GLuint f = 0; f < 6; f++)
	{
		Image const& img = imgFaces[f];
		glTextureImage2DEXT(textureHandle, GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 
							0, GL_RGB, img.getSize().x, img.getSize().y, 0, 
							GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
	}
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false);
		return false;
	}
	return true;
}
void TextureCubemap::free()
{
	if (textureHandle)
	{
		glDeleteTextures(1, &textureHandle);
	}
	textureHandle = NULL;
}
bool TextureCubemap::bind(GLuint textureUnit) const
{
	glBindTextureUnit(textureUnit, textureHandle);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false);
		return false;
	}
	return true;
}