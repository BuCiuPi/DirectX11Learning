#include "ColorHelper.h"

ColorHelper::ColorHelper()
	:color(0)
{}

ColorHelper::ColorHelper(unsigned int val)
	: color(val)
{}

ColorHelper::ColorHelper(BYTE r, BYTE g, BYTE b)
	: ColorHelper(r, g, b, 255)
{
}

ColorHelper::ColorHelper(BYTE r, BYTE g, BYTE b, BYTE a)
{
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}

ColorHelper::ColorHelper(const ColorHelper& src)
	:color(src.color)
{}

ColorHelper& ColorHelper::operator=(const ColorHelper& src)
{
	this->color = src.color;
	return *this;
}

bool ColorHelper::operator==(const ColorHelper& rhs) const
{
	return (this->color == rhs.color);
}

bool ColorHelper::operator!=(const ColorHelper& rhs) const
{
	return !(*this == rhs);
}

constexpr BYTE ColorHelper::GetR() const
{
	return this->rgba[0];
}
void ColorHelper::SetR(BYTE r)
{
	this->rgba[0] = r;
}

constexpr BYTE ColorHelper::GetG() const
{
	return this->rgba[1];
}
void ColorHelper::SetG(BYTE g)
{
	this->rgba[1] = g;
}

constexpr BYTE ColorHelper::GetB() const
{
	return this->rgba[2];
}
void ColorHelper::SetB(BYTE b)
{
	this->rgba[2] = b;
}

constexpr BYTE ColorHelper::GetA() const
{
	return this->rgba[3];
}
void ColorHelper::SetA(BYTE a)
{
	this->rgba[3] = a;
}
