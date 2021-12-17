#include <algorithm>

#include <tt/platform/tt_error.h>

#include <tt/menu/elements/ProgressBar.h>
#include <tt/menu/elements/SkinElementIDs.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuSkin.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;
using math::Vector2;


//------------------------------------------------------------------------------
// Public member functions

ProgressBar::ProgressBar(const std::string& p_name,
                         const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout),
m_shouldUpdate(true),
m_overlayWidth(-1.0f),
m_value(1.0f),
m_maxValue(1.0f),
m_backgroundTextureRect(math::Vector2(0.0f, 0.0f), 1.0f, 1.0f),
m_overlayTextureRect(math::Vector2(0.0f, 0.0f), 1.0f, 1.0f)
{
	// Create the bars
	createBars();
	
	// Perform an initial update of the overlay bar
	updateOverlayBar();
	
	// Set minimum and requested dimensions
	real width  = std::max(m_backgroundSegmentSize.x * 2,
	                       m_overlaySegmentSize.x    * 2);
	real height = std::max(m_backgroundSegmentSize.y,
	                       m_overlaySegmentSize.y);
	setMinimumWidth(static_cast<s32>(width));
	setMinimumHeight(static_cast<s32>(height));
	setRequestedWidth(static_cast<s32>(width));
	setRequestedHeight(static_cast<s32>(height));
}


ProgressBar::~ProgressBar()
{
}


void ProgressBar::render(const PointRect& p_rect, s32 p_z)
{
	// Only render if visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Position and render the background bar segments
	{
		// Left edge
		{
			// FIXME: Should use VectorRect here.
			PointRect leftRect(p_rect);
			leftRect.setWidth (static_cast<s32>(m_backgroundSegmentSize.x));
			leftRect.setHeight(static_cast<s32>(m_backgroundSegmentSize.y));
			
			m_backgroundSegments[BarSegment_LeftEdge]->setPosition(
				static_cast<real>(leftRect.getCenterPosition().x),
				static_cast<real>(leftRect.getCenterPosition().y),
				static_cast<real>(p_z));
			m_backgroundSegments[BarSegment_LeftEdge]->setWidth (m_backgroundSegmentSize.x);
			m_backgroundSegments[BarSegment_LeftEdge]->setHeight(m_backgroundSegmentSize.y);
			
			m_backgroundSegments[BarSegment_LeftEdge]->update();
			m_backgroundSegments[BarSegment_LeftEdge]->render();
		}
		
		
		// Center block
		real centerWidth = p_rect.getWidth() -
		                   (2 * m_backgroundSegmentSize.x);
		if (centerWidth > 0)
		{
			math::VectorRect centerRect(math::Vector2(p_rect.getPosition()),
				static_cast<real>(p_rect.getWidth()),
				static_cast<real>(p_rect.getHeight()));
			centerRect.translate(math::Vector2(m_backgroundSegmentSize.x, 0.0f));
			centerRect.setWidth (centerWidth);
			centerRect.setHeight(m_backgroundSegmentSize.y);
			
			m_backgroundSegments[BarSegment_Center]->setPosition(
				centerRect.getCenterPosition().x,
				centerRect.getCenterPosition().y,
				static_cast<real>(p_z));
			m_backgroundSegments[BarSegment_Center]->setWidth (centerWidth);
			m_backgroundSegments[BarSegment_Center]->setHeight(
				centerRect.getHeight());
			
			m_backgroundSegments[BarSegment_Center]->update();
			m_backgroundSegments[BarSegment_Center]->render();
		}
		
		// Right edge
		math::VectorRect rightRect(math::Vector2(p_rect.getPosition()),
			static_cast<real>(p_rect.getWidth()),
			static_cast<real>(p_rect.getHeight()));
		rightRect.translate(math::Vector2(
			m_backgroundSegmentSize.x + centerWidth, 0.0f));
		rightRect.setWidth (m_backgroundSegmentSize.x);
		rightRect.setHeight(m_backgroundSegmentSize.y);
		
		m_backgroundSegments[BarSegment_RightEdge]->setPosition(
			rightRect.getCenterPosition().x,
			rightRect.getCenterPosition().y,
			static_cast<real>(p_z));
		m_backgroundSegments[BarSegment_RightEdge]->setWidth(
			rightRect.getWidth());
		m_backgroundSegments[BarSegment_RightEdge]->setHeight(
			rightRect.getHeight());
		
		m_backgroundSegments[BarSegment_RightEdge]->update();
		m_backgroundSegments[BarSegment_RightEdge]->render();
	}
	
	
	// Update the overlay bar width
	s32  elemWidth  = p_rect.getWidth();
	real innerwidth = elemWidth - (2 * m_overlaySegmentSize.x);
	
	if (m_shouldUpdate)
	{
		// Recalculate overlay bar width
		m_shouldUpdate = false;
		m_overlayWidth = (m_value * innerwidth) / m_maxValue;
	}
	
	
	// Position and render the overlay bar segments
	{
		// Left edge
		{
			math::VectorRect leftRect(math::Vector2(p_rect.getPosition()),
				m_overlaySegmentSize.x,
				m_overlaySegmentSize.y);
			
			m_overlaySegments[BarSegment_LeftEdge]->setPosition(
				leftRect.getCenterPosition().x,
				leftRect.getCenterPosition().y,
				static_cast<real>(p_z - 1));
			m_overlaySegments[BarSegment_LeftEdge]->setWidth(
				leftRect.getWidth());
			m_overlaySegments[BarSegment_LeftEdge]->setHeight(
				leftRect.getHeight());
			
			m_overlaySegments[BarSegment_LeftEdge]->update();
			m_overlaySegments[BarSegment_LeftEdge]->render();
		}
		
		// Center block
		real centerWidth = m_overlayWidth;
		if (centerWidth > 0)
		{
			// FIXME: Should use VectorRect here.
			math::VectorRect centerRect(math::Vector2(p_rect.getPosition()),
				static_cast<real>(p_rect.getWidth()),
				static_cast<real>(p_rect.getHeight()));
			centerRect.translate(math::Vector2(m_overlaySegmentSize.x, 0.0f));
			centerRect.setWidth (centerWidth);
			centerRect.setHeight(m_overlaySegmentSize.y);
			
			m_overlaySegments[BarSegment_Center]->setPosition(
				centerRect.getCenterPosition().x,
				centerRect.getCenterPosition().y,
				static_cast<real>(p_z - 1));
			m_overlaySegments[BarSegment_Center]->setWidth(
				centerRect.getWidth());
			m_overlaySegments[BarSegment_Center]->setHeight(
				centerRect.getHeight());
			
			m_overlaySegments[BarSegment_Center]->update();
			m_overlaySegments[BarSegment_Center]->render();
		}
		
		// Right edge
		math::VectorRect rightRect(math::Vector2(p_rect.getPosition()),
			static_cast<real>(p_rect.getWidth()),
			static_cast<real>(p_rect.getHeight()));
		rightRect.translate(math::Vector2(
			m_overlaySegmentSize.x + centerWidth - 1, 0.0f));
		rightRect.setWidth (m_overlaySegmentSize.x);
		rightRect.setHeight(m_overlaySegmentSize.y);
		
		m_overlaySegments[BarSegment_RightEdge]->setPosition(
			rightRect.getCenterPosition().x,
			rightRect.getCenterPosition().y,
			static_cast<real>(p_z - 1));
		m_overlaySegments[BarSegment_RightEdge]->setWidth(
			rightRect.getWidth());
		m_overlaySegments[BarSegment_RightEdge]->setHeight(
			rightRect.getHeight());
		
		m_overlaySegments[BarSegment_RightEdge]->update();
		m_overlaySegments[BarSegment_RightEdge]->render();
	}
}


bool ProgressBar::doAction(const MenuElementAction& p_action)
{
	// Allow base to handle action first
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	if (p_action.getCommand() == "update")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "FileBlocksBar '%s': "
		             "Command '%s' takes no parameters.",
		             getName().c_str(), p_action.getCommand().c_str());
		updateOverlayBar();
		return true;
	}
	
	return false;
}


ProgressBar* ProgressBar::clone() const
{
	return new ProgressBar(*this);
}


void ProgressBar::setValue(real p_value)
{
	m_value = p_value;
	updateOverlayBar();
}


void ProgressBar::setMaxValue(real p_maxValue)
{
	m_maxValue = p_maxValue;
	updateOverlayBar();
}


//------------------------------------------------------------------------------
// Protected member functions

ProgressBar::ProgressBar(const ProgressBar& p_rhs)
:
MenuElement(p_rhs),
m_shouldUpdate(p_rhs.m_shouldUpdate),
m_overlayWidth(p_rhs.m_overlayWidth),
m_value(p_rhs.m_value),
m_maxValue(p_rhs.m_maxValue),
m_backgroundTexture(p_rhs.m_backgroundTexture),
m_backgroundSegmentSize(p_rhs.m_backgroundSegmentSize),
m_backgroundTextureRect(p_rhs.m_backgroundTextureRect),
m_overlayTexture(p_rhs.m_overlayTexture),
m_overlaySegmentSize(p_rhs.m_overlaySegmentSize),
m_overlayTextureRect(p_rhs.m_overlayTextureRect)
{
	// Clone the segment quads
	using engine::renderer::QuadSprite;
	for (int i = 0; i < BarSegment_Count; ++i)
	{
		if (p_rhs.m_backgroundSegments[i] != 0)
		{
			m_backgroundSegments[i].reset(
				new QuadSprite(*(p_rhs.m_backgroundSegments[i])));
		}
		
		if (p_rhs.m_overlaySegments[i] != 0)
		{
			m_overlaySegments[i].reset(
				new QuadSprite(*(p_rhs.m_overlaySegments[i])));
		}
	}
}


//------------------------------------------------------------------------------
// Private member functions

void ProgressBar::createBars()
{
	// Get skinning information for Window
	MenuSystem*     sys  = MenuSystem::getInstance();
	const MenuSkin* skin = sys->getSkin();
	
	TT_ASSERTMSG(skin != 0, "Cannot create menu elements without a menu skin.");
	TT_ASSERTMSG(skin->hasElementSkin(SkinElement_ProgressBar),
	             "Skin does not provide skinning information for ProgressBar.");
	
	const MenuSkin::ElementSkin& element(
		skin->getElementSkin(SkinElement_ProgressBar));
	
	// Load the background texture
	const MenuSkin::SkinTexture& backgroundTexture(
		element.getTexture(ProgressBarSkin_BackgroundTexture));

	// FIXME: MUST SUPPORT NAMESPACES
	m_backgroundTexture = engine::renderer::TextureCache::get(backgroundTexture.getFilename(), "");
	if(m_backgroundTexture == 0)
	{
		TT_PANIC("Loading progress bar background texture '%s' failed.",
		         backgroundTexture.getFilename().c_str());
		return;
	}
	
	m_backgroundTextureRect.setPosition(math::Vector2(
		backgroundTexture.getU(), backgroundTexture.getV()));
	m_backgroundTextureRect.setWidth (backgroundTexture.getWidth());
	m_backgroundTextureRect.setHeight(backgroundTexture.getHeight());
	
	// Load the overlay texture
	const MenuSkin::SkinTexture& overlayTexture(
		element.getTexture(ProgressBarSkin_OverlayTexture));

	// FIXME: MUST SUPPORT NAMESPACES
	m_overlayTexture = engine::renderer::TextureCache::get(overlayTexture.getFilename(), "");
	if(m_overlayTexture == 0)
	{
		TT_PANIC("Loading progress bar overlay texture '%s' failed.",
		         overlayTexture.getFilename().c_str());
		m_backgroundTexture.reset();
		m_overlayTexture.reset();
		return;
	}
	
	m_overlayTextureRect.setPosition(math::Vector2(
		overlayTexture.getU(), overlayTexture.getV()));
	m_overlayTextureRect.setWidth (overlayTexture.getWidth());
	m_overlayTextureRect.setHeight(overlayTexture.getHeight());
	
	
	// Calculate block sizes
	m_backgroundSegmentSize.x = m_backgroundTextureRect.getWidth() / 3.0f;
	m_backgroundSegmentSize.y = m_backgroundTextureRect.getHeight();
	
	m_overlaySegmentSize.x = m_overlayTextureRect.getWidth() / 3.0f;
	m_overlaySegmentSize.y = m_overlayTextureRect.getHeight();
	
	
	// Create the quads for the background bar
	using engine::renderer::QuadSprite;
	
	// - Left edge
	m_backgroundSegments[BarSegment_LeftEdge] = QuadSprite::createQuad(
		m_backgroundTexture,
		element.getVertexColor(ProgressBarSkin_BackgroundColor));
	setBarUVs(m_backgroundSegments[BarSegment_LeftEdge],
	          m_backgroundTextureRect, m_backgroundSegmentSize, 0, 0);
	
	// - Center block
	m_backgroundSegments[BarSegment_Center] = QuadSprite::createQuad(
		m_backgroundTexture,
		element.getVertexColor(ProgressBarSkin_BackgroundColor));
	setBarUVs(m_backgroundSegments[BarSegment_Center], m_backgroundTextureRect,
	          m_backgroundSegmentSize, 1, 0);
	
	// - Right edge
	m_backgroundSegments[BarSegment_RightEdge] = QuadSprite::createQuad(
		m_backgroundTexture,
		element.getVertexColor(ProgressBarSkin_BackgroundColor));
	setBarUVs(m_backgroundSegments[BarSegment_RightEdge],
	          m_backgroundTextureRect, m_backgroundSegmentSize, 2, 0);
	
	
	// Create the quads for the overlay bar
	// - Left edge
	m_overlaySegments[BarSegment_LeftEdge] = QuadSprite::createQuad(
		m_overlayTexture,
		element.getVertexColor(ProgressBarSkin_OverlayColor));
	setBarUVs(m_overlaySegments[BarSegment_LeftEdge], m_overlayTextureRect,
	          m_overlaySegmentSize, 0, 0);
	
	// - Center block
	m_overlaySegments[BarSegment_Center] = QuadSprite::createQuad(
		m_overlayTexture,
		element.getVertexColor(ProgressBarSkin_OverlayColor));
	setBarUVs(m_overlaySegments[BarSegment_Center], m_overlayTextureRect,
	          m_overlaySegmentSize, 1, 0);
	
	// - Right edge
	m_overlaySegments[BarSegment_RightEdge] = QuadSprite::createQuad(
		m_overlayTexture,
		element.getVertexColor(ProgressBarSkin_OverlayColor));
	setBarUVs(m_overlaySegments[BarSegment_RightEdge], m_overlayTextureRect,
	          m_overlaySegmentSize, 2, 0);
}


void ProgressBar::setBarUVs(const engine::renderer::QuadSpritePtr& p_quad,
                            const math::VectorRect& p_uvRect,
                            const math::Vector2&    p_blockSize,
                            s32                     p_blockX,
                            s32                     p_blockY)
{
	real blockW = p_blockSize.x;
	real blockH = p_blockSize.y;
	
	TT_ASSERTMSG(blockW > 0.0f && blockH > 0.0f,
	             "Attempt to construct bar with fishy UV bounds.");
	
	/*
	p_quad->setTexcoordLeft(p_uvRect.getPosition().x + (blockW * p_blockX));
	p_quad->setTexcoordTop(p_uvRect.getPosition().y + (blockH * p_blockY));
	
	p_quad->setTexcoordRight(p_uvRect.getPosition().x +
		(blockW * (p_blockX + 1)) - 1);
	p_quad->setTexcoordBottom(p_uvRect.getPosition().y +
		(blockH * (p_blockY + 1)) - 1);
	*/ (void)blockW; (void)blockH;
}


void ProgressBar::updateOverlayBar()
{
	m_shouldUpdate = true;
}

// Namespace end
}
}
}
