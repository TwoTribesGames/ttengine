#include <tt/engine/renderer/QuadSprite.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/Border.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

BorderPtr Border::create(real p_thickness, const tt::engine::renderer::ColorRGBA& p_color)
{
	if (p_thickness <= 0.0f)
	{
		TT_PANIC("Invalid border thickness specified: %f -- must be larger than 0.", p_thickness);
		return BorderPtr();
	}
	
	return BorderPtr(new Border(p_thickness, p_color));
}


Border::~Border()
{
}


void Border::fitAroundRectangle(const tt::math::VectorRect& p_rect)
{
	typedef tt::math::VectorRect VR;
	typedef tt::math::Vector2    V2;
	
	m_rect = p_rect;
	
	const real w = p_rect.getWidth();
	const real h = p_rect.getHeight();
	const V2   pos(p_rect.getPosition());
	
	const VR rectTop   (pos + V2(-m_thickness, -m_thickness), w + (2 * m_thickness), m_thickness);
	const VR rectBottom(pos + V2(-m_thickness, h),            w + (2 * m_thickness), m_thickness);
	const VR rectLeft  (pos + V2(-m_thickness, 0.0f),         m_thickness,           h);
	const VR rectRight (pos + V2(w,            0.0f),         m_thickness,           h);
	
	m_top->setWidth   (rectTop.getWidth());
	m_top->setHeight  (rectTop.getHeight());
	m_top->setPosition(rectTop.getCenterPosition().x, -rectTop.getCenterPosition().y, 0.0f);
	
	m_bottom->setWidth   (rectBottom.getWidth());
	m_bottom->setHeight  (rectBottom.getHeight());
	m_bottom->setPosition(rectBottom.getCenterPosition().x, -rectBottom.getCenterPosition().y, 0.0f);
	
	m_left->setWidth   (rectLeft.getWidth());
	m_left->setHeight  (rectLeft.getHeight());
	m_left->setPosition(rectLeft.getCenterPosition().x, -rectLeft.getCenterPosition().y, 0.0f);
	
	m_right->setWidth   (rectRight.getWidth());
	m_right->setHeight  (rectRight.getHeight());
	m_right->setPosition(rectRight.getCenterPosition().x, -rectRight.getCenterPosition().y, 0.0f);
	
	
	m_top->update();
	m_bottom->update();
	m_left->update();
	m_right->update();
}


void Border::fitInsideRectangle(const tt::math::VectorRect& p_rect)
{
	tt::math::VectorRect insideRect(p_rect);
	insideRect.translate(tt::math::Vector2(m_thickness, m_thickness));
	insideRect.setWidth (insideRect.getWidth()  - (2.0f * m_thickness));
	insideRect.setHeight(insideRect.getHeight() - (2.0f * m_thickness));
	fitAroundRectangle(insideRect);
}


void Border::render()
{
	m_top->render();
	m_bottom->render();
	m_left->render();
	m_right->render();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Border::Border(real p_thickness, const tt::engine::renderer::ColorRGBA& p_color)
:
m_thickness(p_thickness),
m_color(p_color),
m_rect(tt::math::Vector2(0.0f, 0.0f), 0.0f, 0.0f),
m_top   (tt::engine::renderer::QuadSprite::createQuad(p_thickness, p_thickness, p_color)),
m_bottom(tt::engine::renderer::QuadSprite::createQuad(p_thickness, p_thickness, p_color)),
m_left  (tt::engine::renderer::QuadSprite::createQuad(p_thickness, p_thickness, p_color)),
m_right (tt::engine::renderer::QuadSprite::createQuad(p_thickness, p_thickness, p_color))
{
}

// Namespace end
}
}
