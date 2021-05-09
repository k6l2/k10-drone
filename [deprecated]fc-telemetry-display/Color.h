#pragma once
class Color : public glm::u8vec4
{
public:
	static const Color Transparent;
	static const Color Black;
	static const Color White;
	static const Color Red;
	static const Color Green;
	static const Color Yellow;
	static const Color Blue;
public:
	Color(glm::u8vec4 glColor);
	Color(u8 r = 255, u8 g = 255, u8 b = 255, u8 a = 255);
	Color(float fr, float fg, float fb, float fa);
	Color(Color const& other);
	float fR() const;
	float fG() const;
	float fB() const;
	float fA() const;
};