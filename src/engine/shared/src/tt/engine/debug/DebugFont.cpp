
#include <tt/engine/debug/DebugFont.h>

#include <tt/engine/debug/font_data.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>


namespace tt {
namespace engine {
namespace debug {


renderer::QuadBufferPtr       DebugFont::ms_buffer;
renderer::BatchQuadCollection DebugFont::ms_quads;
renderer::ColorRGBA           DebugFont::ms_color = renderer::ColorRGB::white;
u32                           DebugFont::ms_quadIndex = 0;
bool                          DebugFont::ms_initialized = false;
real                          DebugFont::ms_scale = 1.0f;


void DebugFont::initialize()
{
	if(ms_initialized) return;

	ms_buffer.reset(new renderer::QuadBuffer(
	renderer::QuadBuffer::maxBatchSize, buildFontTexture(), renderer::BatchFlagQuad_UseVertexColor));

	//ms_quads.reserve(renderer::QuadBuffer::maxBatchSize);

	ms_initialized = true;
}


void DebugFont::destroy()
{
	ms_initialized = false;
	ms_buffer.reset();
	tt::code::helpers::freeContainer(ms_quads);
}


void DebugFont::setColor(const renderer::ColorRGBA& p_color)
{
	ms_color = p_color;
}


void DebugFont::draw(const std::string& p_msg, s32 p_x, s32 p_y)
{
	if(ms_quadIndex >= static_cast<u32>(renderer::QuadBuffer::maxBatchSize))
	{
		return;
	}

	static const math::Vector2 uvOffset(1.0f / characterWidth, 1.0f / characterHeight);
	const math::Point2 charSize(static_cast<s32>(characterWidth  * ms_scale),
		                        static_cast<s32>(characterHeight * ms_scale));

	math::Vector2 pos(static_cast<real>(p_x), static_cast<real>(p_y));

	for(std::string::const_iterator it = p_msg.begin(); it != p_msg.end(); ++it)
	{
		// Line wrapping
		std::string::value_type character(*it);
		if(character == '\n')
		{
			pos.x = static_cast<real>(p_x);
			pos.y += charSize.y;
		}

		u8 index = static_cast<u8>(character);

		// Compute UV
		const math::Point2 charPosition(index % characterWidth, index / characterWidth);

#ifdef TT_PLATFORM_WIN
		static const math::Vector2 platformUVOffset(0.5f / fontTextureWidth, 0.5f / fontTextureHeight);
#else
		static const math::Vector2 platformUVOffset(0,0);
#endif

		const math::Vector2 uvStart(charPosition.x / static_cast<real>(characterWidth)  + platformUVOffset.x,
			                        charPosition.y / static_cast<real>(characterHeight) - platformUVOffset.y);

		renderer::BatchQuad& quad = getNextQuad();

		// Set texture coordinates
		quad.topLeft.    setTexCoord(uvStart.x, uvStart.y + uvOffset.y);
		quad.topRight.   setTexCoord(uvStart + uvOffset);
		quad.bottomLeft. setTexCoord(uvStart);
		quad.bottomRight.setTexCoord(uvStart.x + uvOffset.x, uvStart.y);

		// Set position
		quad.topLeft.    setPosition(pos.x,              -pos.y,              0);
		quad.topRight.   setPosition(pos.x + charSize.x, -pos.y,              0);
		quad.bottomLeft. setPosition(pos.x,              -pos.y - charSize.y, 0);
		quad.bottomRight.setPosition(pos.x + charSize.x, -pos.y - charSize.y, 0);

		quad.topLeft.    setColor(ms_color);
		quad.topRight.   setColor(ms_color);
		quad.bottomLeft. setColor(ms_color);
		quad.bottomRight.setColor(ms_color);

		pos.x += static_cast<s32>(debugFontSpacing[index] * ms_scale);

		// Screen wrapping
		/*
		const s32 screenWidth = renderer::Renderer::getInstance()->getScreenWidth();
		const s32 rightMargin = static_cast<s32>(0.05f * screenWidth); // 5% margin

		if(pos.x > (screenWidth - rightMargin))
		{
			pos.x = static_cast<real>(p_x);
			pos.y += charSize.y;
		}
		*/
	}
}


void DebugFont::flush()
{
	if(ms_quadIndex > 0)
	{
		TT_NULL_ASSERT(ms_buffer);
		ms_buffer->fillBuffer(ms_quads.begin(), ms_quads.begin() + ms_quadIndex);
		
		renderer::Renderer::getInstance()->beginHud();
		ms_buffer->render();
		renderer::Renderer::getInstance()->endHud();
		
		ms_quadIndex = 0;
	}
}


//--------------------------------------------------------------------------------------------------
// Private


renderer::TexturePtr DebugFont::buildFontTexture()
{
	// Create texture for font
	renderer::TexturePtr fontTexture = renderer::Texture::createForText(fontTextureWidth, fontTextureHeight);

	fontTexture->setMinificationFilter (renderer::FilterMode_Point);
	fontTexture->setMagnificationFilter(renderer::FilterMode_Point);
	fontTexture->setAddressMode(renderer::AddressMode_Clamp, renderer::AddressMode_Clamp);
	
	// Copy font data into engine texture
	{
		renderer::TexturePainter painter(fontTexture->lock());
		
		for (s32 y = 0; y < fontTextureHeight; ++y)
		{
			for (s32 x = 0; x < fontTextureWidth; ++x)
			{
				const u8 value = debugFontData[x + fontTextureHeight * y];
				painter.setPixel(x, y, renderer::ColorRGBA(255, 255, 255, value));
			}
		}
	}

	return fontTexture;
}


renderer::BatchQuad& DebugFont::getNextQuad()
{
	u32 index = ms_quadIndex;

	if (index == ms_quads.size())
	{
		ms_quads.push_back(renderer::BatchQuad());
	}
	++ms_quadIndex;

	return ms_quads[index];
}


}
}
}
