#include "Color.h"
const Color Color::Transparent = glm::u8vec4{   0,   0,   0,   0 };
const Color Color::Black       = glm::u8vec4{   0,   0,   0, 255 };
const Color Color::Red         = glm::u8vec4{ 255,   0,   0, 255 };
const Color Color::Green       = glm::u8vec4{   0, 255,   0, 255 };
const Color Color::Yellow      = glm::u8vec4{ 255, 255,   0, 255 };
const Color Color::Blue        = glm::u8vec4{   0,   0, 255, 255 };
const Color Color::White       = glm::u8vec4{ 255, 255, 255, 255 };
Color::Color(glm::u8vec4 glColor)
{
	r = glColor.r;
	g = glColor.g;
	b = glColor.b;
	a = glColor.a;
}
Color::Color(u8 r, u8 g, u8 b, u8 a)
{
	SDL_assert(r >= 0 && r <= 255);
	SDL_assert(g >= 0 && g <= 255);
	SDL_assert(b >= 0 && b <= 255);
	SDL_assert(a >= 0 && a <= 255);
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}
Color::Color(float fr, float fg, float fb, float fa)
{
	SDL_assert(fr >= 0 && fr <= 1);
	SDL_assert(fg >= 0 && fg <= 1);
	SDL_assert(fb >= 0 && fb <= 1);
	SDL_assert(fa >= 0 && fa <= 1);
	this->r = static_cast<glm::u8>(fr * 255);
	this->g = static_cast<glm::u8>(fg * 255);
	this->b = static_cast<glm::u8>(fb * 255);
	this->a = static_cast<glm::u8>(fa * 255);
}
Color::Color(Color const& other)
{
	r = other.r;
	g = other.g;
	b = other.b;
	a = other.a;
}
float Color::fR() const
{
	return r / 255.f;
}
float Color::fG() const
{
	return g / 255.f;
}
float Color::fB() const
{
	return b / 255.f;
}
float Color::fA() const
{
	return a / 255.f;
}