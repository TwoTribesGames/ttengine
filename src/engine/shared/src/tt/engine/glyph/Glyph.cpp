#include <tt/platform/tt_printf.h>

#include <tt/engine/glyph/Glyph.h>


namespace tt {
namespace engine {
namespace glyph {

u8 Glyph::ms_alphaThreshold = 96;

Glyph::Glyph()
:
m_char(0),
m_width(0),
m_height(0),
m_charwidth(0)
{
}


Glyph::~Glyph()
{
}

s32 Glyph::getXMin() const
{
	s32 minX = getWidth();
	for (s32 row = 0; row < getHeight(); ++row)
	{
		s32 x = getXMin(row);
		
		if (x < minX)
		{
			minX = x;
		}
	}
	return minX;
}


s32 Glyph::getXMax(bool p_checkAlpha) const
{
	s32 maxX = getWidth();
	for (s32 row = 0; row < getHeight(); ++row)
	{
		s32 x = getXMax(row,p_checkAlpha);
		
		if (x < maxX)
		{
			maxX = x;
		}
	}
	return maxX;
}


void Glyph::printPixels() const
{
	TT_Printf("/");
	s32 counter = 0; // Used to print the 0 through 9 and then start at 0 again.
	for (s32 column = 0; column < getWidth(); ++column)
	{
		TT_Printf("%d", counter);
		++counter;
		if (counter > 9)
		{
			counter = 0;
		}
	}
	TT_Printf("\\\n");
	for (s32 row = 0; row < getHeight(); ++row)
	{
		TT_Printf("|");
		printPixelRow(row);
		TT_Printf("|%d\n", row);
	}
	TT_Printf("\\");
	counter = 0;
	for (s32 column = 0; column < getWidth(); ++column)
	{
		TT_Printf("%d", counter);
		++counter;
		if (counter > 9)
		{
			counter = 0;
		}
	}
	TT_Printf("/\n");
}


// Namespace end
}
}
}

