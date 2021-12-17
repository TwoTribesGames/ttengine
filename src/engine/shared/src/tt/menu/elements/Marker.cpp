#include <tt/platform/tt_error.h>
#include <tt/menu/elements/Marker.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/elements/ElementLayout.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>

namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

Marker::Marker(const std::string& p_name,
               const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout)
{
	MENU_CREATION_Printf("Marker::Marker: Element '%s': New Marker.\n",
	                     getName().c_str());
	
	// FIXME: Remove hard-coded filename.
	const std::string filename("MenuElements\\elements");
	m_texture = engine::renderer::TextureCache::get(filename, "");
	if(m_texture == 0)
	{
		TT_PANIC("Loading marker texture '%s' failed.", filename.c_str());
	}
	
	// FIXME: Remove hard-coded texture UV settings.
	setTexture(MARKER_X, MARKER_Y, MARKER_W, MARKER_H);
	
	// Set the minimum and requested dimensions
	setMinimumWidth(static_cast<s32>(m_blockSize.x));
	setMinimumHeight(static_cast<s32>(m_blockSize.y));
	setRequestedWidth(static_cast<s32>(m_blockSize.x));
	setRequestedHeight(static_cast<s32>(m_blockSize.y));
}


Marker::~Marker()
{
	MENU_CREATION_Printf("Marker::~Marker: Element '%s': Freeing resources.\n",
	                     getName().c_str());
}


void Marker::render(const math::PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Readability shorts
	const real xp = p_rect.getWidth()  * 0.5f;
	const real yp = p_rect.getHeight() * 0.5f;
	
	using math::Vector3;
	Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
	            static_cast<real>(p_rect.getCenterPosition().y),
	            static_cast<real>(p_z));
	
	
	// Position the corners
	m_corners[Corner_TopLeft ]->setPosition(pos + Vector3(-xp, -yp, 0.0f));
	m_corners[Corner_TopRight]->setPosition(pos + Vector3(xp,  -yp, 0.0f));
	m_corners[Corner_BotLeft ]->setPosition(pos + Vector3(-xp, yp,  0.0f));
	m_corners[Corner_BotRight]->setPosition(pos + Vector3(xp,  yp,  0.0f));
	
	// Update and render the corners
	for (int i = 0; i < Corner_Count; ++i)
	{
		m_corners[i]->update();
		m_corners[i]->render();
	}
}


Marker* Marker::clone() const
{
	return new Marker(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

Marker::Marker(const Marker& p_rhs)
:
MenuElement(p_rhs),
m_texture(p_rhs.m_texture),
m_topLeft(p_rhs.m_topLeft),
m_botRight(p_rhs.m_botRight),
m_blockSize(p_rhs.m_blockSize)
{
	// Clone the marker segments
	using engine::renderer::QuadSprite;
	for (int i = 0; i < Corner_Count; ++i)
	{
		m_corners[i].reset(new QuadSprite(*(p_rhs.m_corners[i])));
	}
}


//------------------------------------------------------------------------------
// Private member functions

void Marker::setTexture(real p_x0, real p_y0, real p_x1, real p_y1)
{
	m_topLeft.x = p_x0;
	m_topLeft.y = p_y0;
	
	m_botRight.x = p_x0 + p_x1;
	m_botRight.y = p_y0 + p_y1;
	
	m_blockSize.x = (m_botRight.x - m_topLeft.x) * 0.5f;
	m_blockSize.y = (m_botRight.y - m_topLeft.y) * 0.5f;
	
	// All zero? Use texture size
	if (m_topLeft == m_botRight)
	{
		m_botRight.x = 1.0f;
		m_botRight.y = 1.0f;
	}
	
	// Build the quads
	using engine::renderer::QuadSprite;
	
	for (int i = 0; i < Corner_Count; ++i)
	{
		m_corners[i] = QuadSprite::createQuad(m_texture);
		m_corners[i]->setWidth(m_blockSize.x);
		m_corners[i]->setHeight(m_blockSize.y);
	}
	
	setUVs(m_corners[Corner_TopLeft ], 0, 0);
	setUVs(m_corners[Corner_TopRight], 1, 0);
	setUVs(m_corners[Corner_BotLeft ], 0, 1);
	setUVs(m_corners[Corner_BotRight], 1, 1);
}


void Marker::setUVs(const engine::renderer::QuadSpritePtr& p_quad,
                    s32 p_blockX,
                    s32 p_blockY)
{
	real blockW = m_blockSize.x;
	real blockH = m_blockSize.y;
	
	TT_ASSERTMSG(blockW > 0.0f && blockH > 0.0f,
	             "Attempt to construct Marker with fishy UV bounds.");
	
	/*
	p_quad->setTexcoordLeft(m_topLeft.x + (blockW * p_blockX));
	p_quad->setTexcoordTop(m_topLeft.y + (blockH * p_blockY));
	
	p_quad->setTexcoordRight(m_topLeft.x + (blockW * (p_blockX + 1)));
	p_quad->setTexcoordBottom(m_topLeft.y + (blockH * (p_blockY + 1)));
	*/ (void)blockW; (void)blockH;
}

// Namespace end
}
}
}
