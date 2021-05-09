#include "Image.h"
bool Image::load(string const& filePath)
{
	free();
	// the last param is set to 4 to guarantee data is in RGBA format //
	data = stbi_load(filePath.c_str(), &size.x, &size.y, &channelCount, 4);
	if (!data)
	{
		return false;
	}
	return true;
}
void Image::free()
{
	if (data)
	{
		stbi_image_free(data);
	}
	data = nullptr;
}
v2i const& Image::getSize() const
{
	return size;
}
u8 const* Image::getData() const
{
	return data;
}
void Image::flipHorizontally()
{
	for (int y = 0; y < size.y; y++)
	{
		for (int x = 0; x < size.x / 2; x++)
		{
			const int pixelByteStartLeft = (y * size.x + x) * 4;
			const int pixelByteStartRight = (y * size.x + (size.x - 1 - x)) * 4;
			for (int c = 0; c < 4; c++)
			{
				std::swap(data[pixelByteStartLeft + c],
					data[pixelByteStartRight + c]);
			}
		}
	}
}
void Image::rotate90DegreesCW()
{
	if (size.x != size.y)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Image rotation for non-square dimensions is not supported!\n");
		SDL_assert(false);
		return;
	}
	u8* newData = static_cast<u8*>(malloc(size.x * size.y * 4));
	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			const int pixelByteStartNew = (y * size.x + x) * 4;
			const int pixelByteStartOld = (x * size.x + (size.y - 1 - y)) * 4;
			for (int c = 0; c < 4; c++)
			{
				newData[pixelByteStartNew + c] = data[pixelByteStartOld + c];
			}
		}
	}
	stbi_image_free(data);
	data = newData;
}