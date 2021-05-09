#pragma once
class Image
{
public:
	bool load(string const& filePath);
	void free();
	v2i const& getSize() const;
	u8 const* getData() const;
	void flipHorizontally();
	void rotate90DegreesCW();
private:
	u8* data = nullptr;
	v2i size;
	int channelCount;
};
