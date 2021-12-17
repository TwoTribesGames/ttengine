#include <math.h>

#include "Gwen/Renderers/TTRenderer.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"

// Include FontData from src dir.
#include "DebugFont/FontData.h"


#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/str/manip.h>
#include <tt/engine/file/TextureHeader.h>


namespace Gwen {
namespace Renderer {


// Wrapper to keep smart pointer alive
struct TextureWrapper
{
	tt::engine::renderer::TexturePtr texture;
	
	explicit inline TextureWrapper(const tt::engine::renderer::TexturePtr& p_texture)
	:
	texture(p_texture)
	{ }
};

static const s32 fontTextureSize = 256;
static const s32 quadBufferSize  = 256;

#if defined(TT_PLATFORM_WIN)
	static const tt::math::Vector3 renderOffset(-0.5f, 0.5f, 0.0f);
#else
	static const tt::math::Vector3 renderOffset(0.0f, 0.0f, 0.0f);
#endif


TTRenderer::TTRenderer()
:
m_fontTexture(0),
m_letterSpacing(0),
m_currentTexture(0),
m_color(tt::engine::renderer::ColorRGB::white)
{
	// Setup default quad
	m_quad.topLeft.setColor(m_color);
	m_quad.topRight.setColor(m_color);
	m_quad.bottomLeft.setColor(m_color);
	m_quad.bottomRight.setColor(m_color);
	
	// ---------------------------------------------------------------------------------
	// FONT
	
	m_letterSpacing = 1.0f / 16.0f;
	m_fontScale[0]  = 1.5f;
	m_fontScale[1]  = 1.5f;
	
	m_fontTexture = new Gwen::Texture;
	
	using namespace tt::engine;
	
	// Create texture for font
	renderer::TexturePtr engineTexture = renderer::Texture::createForText(fontTextureSize, fontTextureSize);
	
	// Wrap our engine texture to prevent smart pointer from being released
	m_fontTexture->data   = new TextureWrapper(engineTexture);
	m_fontTexture->width  = fontTextureSize;
	m_fontTexture->height = fontTextureSize;
	
	// Copy font data into engine texture
	{
		renderer::TexturePainter painter(engineTexture->lock());
		
		// FIXME: Need a Greyscale format for text rendering so we can do this:
		//painter.copyRaw(sGwenFontData);
		
		for (s32 x = 0; x < fontTextureSize; ++x)
		{
			for (s32 y = 0; y < fontTextureSize; ++y)
			{
				const u8 value = sGwenFontData[x + fontTextureSize * y];
				painter.setPixel(x, y, renderer::ColorRGBA(value, value, value, value));
			}
		}
	}
	
	m_quadBuffer.reset(
		new renderer::QuadBuffer(quadBufferSize, renderer::TexturePtr(), renderer::BatchFlagQuad_UseVertexColor));
}


TTRenderer::~TTRenderer()
{
	FreeTexture(m_fontTexture);
	delete m_fontTexture;
}


void TTRenderer::Begin()
{
	using tt::engine::renderer::Renderer;
	using tt::engine::renderer::MatrixStack;
	
	Renderer::getInstance()->beginHud();
	Renderer::getInstance()->resetMultiTexture();
	MatrixStack::getInstance()->resetTextureMatrix();
	MatrixStack::getInstance()->push();
	MatrixStack::getInstance()->translate(renderOffset);
}


void TTRenderer::End()
{
	Flush();
	
	using tt::engine::renderer::Renderer;
	using tt::engine::renderer::MatrixStack;
	
	MatrixStack::getInstance()->pop();
	Renderer::getInstance()->endHud();
}


void TTRenderer::SetDrawColor(Gwen::Color color)
{
	m_color.setColor(color.r, color.g, color.b, color.a);
	
	m_quad.topLeft.setColor(m_color);
	m_quad.topRight.setColor(m_color);
	m_quad.bottomLeft.setColor(m_color);
	m_quad.bottomRight.setColor(m_color);
}


void TTRenderer::DrawFilledRect( Gwen::Rect rect )
{
	if (m_currentTexture != 0)
	{
		Flush();
		m_quadBuffer->setTexture(tt::engine::renderer::TexturePtr());
		m_currentTexture = 0;
	}
	
	Translate(rect);
	AddToBatch(rect);
}


void TTRenderer::LoadFont( Gwen::Font* /*font*/ )
{
}


void TTRenderer::FreeFont( Gwen::Font* /*pFont*/ )
{
}


void TTRenderer::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text )
{
	float fSize = pFont->size * Scale();
	
	if (text.empty())
	{
		return;
	}
	
	const Gwen::String converted_string = Gwen::Utility::UnicodeToString( text );
	
	float yOffset = 0.0f;
	for (Gwen::String::const_iterator it = converted_string.begin(); it != converted_string.end(); ++it)
	{
		//wchar_t chr = text[it];
		const s32 charIndex = static_cast<s32>(*it);
		TT_ASSERT(charIndex >= 0);
		float curSpacing = sGwenDebugFontSpacing[charIndex] * m_letterSpacing * fSize * m_fontScale[0];
		Gwen::Rect r( pos.x + yOffset, pos.y - fSize * 0.2f, 
		              (fSize * m_fontScale[0]), fSize * m_fontScale[1] );
		
		if (m_fontTexture != 0)
		{
			float uv_texcoords[8] = { 0.0f, 0.0f, 1.0f, 1.0f };
			
			{
				float cx = (charIndex % 16) / 16.0f;
				float cy = (charIndex / 16) / 16.0f;
				uv_texcoords[0] = cx;
				uv_texcoords[1] = cy;
				uv_texcoords[4] = float(cx + 1.0f / 16.0f);
				uv_texcoords[5] = float(cy + 1.0f / 16.0f);
			}
			
			DrawTexturedRect( m_fontTexture, r, uv_texcoords[0], uv_texcoords[5], uv_texcoords[4], uv_texcoords[1] );
			yOffset += curSpacing;
		}
		else
		{
			DrawFilledRect( r );
			yOffset += curSpacing;
		}
	}
}


Gwen::Point TTRenderer::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
{
	Gwen::Point p;
	float fSize = pFont->size * Scale();
	
	const Gwen::String converted_string = Gwen::Utility::UnicodeToString( text );
	float spacing = 0.0f;
	
	for (Gwen::String::const_iterator it = converted_string.begin(); it != converted_string.end(); ++it)
	{
		char ch = *it;
		spacing += sGwenDebugFontSpacing[(int)ch];
	}
	
	p.x = spacing * m_letterSpacing * fSize * m_fontScale[0];
	p.y = pFont->size * Scale() * m_fontScale[1];
	return p;
}


void TTRenderer::StartClip()
{
	Flush();
	const Gwen::Rect& rect = ClipRegion();
	tt::engine::renderer::Renderer::getInstance()->setScissorRect(
			tt::math::PointRect(tt::math::Point2(rect.x, rect.y), rect.w, rect.h));
}


void TTRenderer::EndClip()
{
	Flush();
	tt::engine::renderer::Renderer::getInstance()->resetScissorRect();
}


void TTRenderer::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 )
{
	TT_NULL_ASSERT(pTexture);
	if (pTexture->failed)
	{
		return;
	}
	
	TT_NULL_ASSERT(pTexture->data);
	TextureWrapper* textureWrapper = reinterpret_cast<TextureWrapper*>(pTexture->data);
	TT_NULL_ASSERT(textureWrapper->texture);
	
	if ( m_currentTexture != pTexture )
	{
		Flush();
		m_quadBuffer->setTexture(textureWrapper->texture);
		m_currentTexture = pTexture;
	}
	
	// Set UV coords
	m_quad.topLeft.setTexCoord    (u1, v1);
	m_quad.topRight.setTexCoord   (u2, v1);
	m_quad.bottomLeft.setTexCoord (u1, v2);
	m_quad.bottomRight.setTexCoord(u2, v2);
	
	Translate(rect);
	AddToBatch(rect);
}


void TTRenderer::LoadTexture( Gwen::Texture* pTexture, bool p_paintable)
{
	TT_NULL_ASSERT(pTexture);
	if (pTexture->data != 0)
	{
		TextureWrapper* oldTex = reinterpret_cast<TextureWrapper*>(pTexture->data);
		delete oldTex;
		pTexture->data = 0;
	}
	
	const std::string fullTextureName(pTexture->name.Get());
	
	tt::str::Strings splitName = tt::str::explode(fullTextureName, ".", true);
	const std::string textureName(splitName.back());
	splitName.pop_back();
	const std::string namespaceName("textures" + ((splitName.empty() ? "" : ".") + tt::str::implode(splitName, ".")));
	
	tt::engine::renderer::TexturePtr engineTexture = tt::engine::renderer::TextureCache::get(textureName, namespaceName, true, 
		p_paintable ? tt::engine::file::TextureFlag_Paintable : 0);
	if (engineTexture == 0)
	{
		TT_PANIC("Failed to load texture '%s' (namespace: '%s') for GWEN.",
		         textureName.c_str(), namespaceName.c_str());
		pTexture->failed = true;
		return;
	}
	else
	{
		engineTexture->setPaintable(true);
	}
	
	TextureWrapper* tttexture = new TextureWrapper(engineTexture);
	
	pTexture->data   = tttexture;
	pTexture->width  = engineTexture->getWidth();
	pTexture->height = engineTexture->getHeight();
}


void TTRenderer::FreeTexture( Gwen::Texture* pTexture )
{
	if (pTexture == 0)
	{
		return;
	}
	
	TextureWrapper* tttexture = reinterpret_cast<TextureWrapper*>(pTexture->data);
	if (tttexture != 0)
	{
		delete tttexture;
	}
	pTexture->data = 0;
}


Gwen::Color TTRenderer::PixelColour( Gwen::Texture* p_texture, unsigned int p_x, unsigned int p_y, const Gwen::Color& p_default )
{
	TT_NULL_ASSERT(p_texture);
	
#if (TT_OPENGLES_VERSION == 0)
	TextureWrapper* textureWrapper = reinterpret_cast<TextureWrapper*>(p_texture->data);
	TT_NULL_ASSERT(textureWrapper->texture);
	
	tt::engine::renderer::TexturePainter painter(textureWrapper->texture->lock());
	tt::engine::renderer::ColorRGBA colorFound;
	
	if (painter.getPixel(p_x, p_y, colorFound))
	{
		return Gwen::Color(colorFound.r, colorFound.g, colorFound.b, colorFound.a);
	}
#endif
	
	(void)p_x;
	(void)p_y;
	(void)p_default;
	return Gwen::Color(0, 0, 0, 255);
}


//////////////////////////////////////////
// Private

void TTRenderer::AddToBatch(const Gwen::Rect& p_rect)
{
	m_quad.topLeft.setPosition    (p_rect.x,            -p_rect.y,            0.0f);
	m_quad.topRight.setPosition   (p_rect.x + p_rect.w, -p_rect.y,            0.0f);
	m_quad.bottomLeft.setPosition (p_rect.x,            -p_rect.y - p_rect.h, 0.0f);
	m_quad.bottomRight.setPosition(p_rect.x + p_rect.w, -p_rect.y - p_rect.h, 0.0f);
	
	m_quadCollection.push_back(m_quad);
	
	if (static_cast<s32>(m_quadCollection.size()) == quadBufferSize)
	{
		Flush();
	}
}


void TTRenderer::Flush()
{
	using namespace tt::engine::renderer;

	m_quadBuffer->setCollection(m_quadCollection);
	m_quadBuffer->applyChanges();
	m_quadBuffer->render();

	m_quadBuffer->clear();

	m_quadCollection.clear();
}

// Namespace end
}
}
