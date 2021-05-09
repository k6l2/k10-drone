#include "Texture.h"
#include "Image.h"
bool Texture::createFromImage(Image const& img)
{
	free();
	glCreateTextures(GL_TEXTURE_2D, 1, &textureHandle);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureImage2DEXT(textureHandle, GL_TEXTURE_2D, 0, GL_RGBA, 
					    img.getSize().x, img.getSize().y, 0, GL_RGBA, 
						GL_UNSIGNED_BYTE, img.getData());
	glGenerateTextureMipmap(textureHandle);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false);
		return false;
	}
	return true;
}
void Texture::free()
{
	if (textureHandle)
	{
		glDeleteTextures(1, &textureHandle);
	}
	textureHandle = NULL;
}
v2i Texture::getSize() const
{
	v2i retVal;
	glGetTextureLevelParameteriv(textureHandle, 0, GL_TEXTURE_WIDTH , &retVal.x);
	glGetTextureLevelParameteriv(textureHandle, 0, GL_TEXTURE_HEIGHT, &retVal.y);
	return retVal;
}
bool Texture::bind(GLuint textureUnit) const
{
	glBindTextureUnit(textureUnit, textureHandle);
	if (glCheckError() != GL_NO_ERROR)
	{
		SDL_assert(false);
		return false;
	}
	return true;
}