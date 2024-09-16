#ifndef COLORHELPER_H
#define COLORHELPER_H
typedef unsigned char BYTE;

class ColorHelper
{
public:
	ColorHelper();
	ColorHelper(unsigned int val);
	ColorHelper(BYTE r, BYTE g, BYTE b);
	ColorHelper(BYTE r, BYTE g, BYTE b, BYTE a);
	ColorHelper(const ColorHelper& src);

	ColorHelper& operator=(const ColorHelper& src);
	bool operator==(const ColorHelper& rhs) const;
	bool operator!=(const ColorHelper& rhs) const;

	constexpr BYTE GetR() const;
	void SetR(BYTE r);

	constexpr BYTE GetG() const;
	void SetG(BYTE g);

	constexpr BYTE GetB() const;
	void SetB(BYTE b);

	constexpr BYTE GetA() const;
	void SetA(BYTE a);

private:
	union
	{
		BYTE rgba[4];
		unsigned int color;
	};
};

namespace ColorHelpers
{
	const ColorHelper UnloadedTextureColor(100, 100, 100);
	const ColorHelper UnhandledTextureColor(250, 0, 0);
}

#endif
