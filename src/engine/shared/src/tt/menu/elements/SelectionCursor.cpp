#include <tt/platform/tt_error.h>

#include <tt/menu/elements/SelectionCursor.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/elements/LinearTranslationAnimation.h>
#include <tt/menu/elements/BouncePositionAnimation.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

SelectionCursor::SelectionCursor(const std::string& p_name,
                                 const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout),
m_selectionRect(math::Point2(0, 0), 1, 1),
m_selectionType(SelectionCursorType_Both),
m_leftTranslation(0),
m_rightTranslation(0),
m_leftPosition(0),
m_rightPosition(0)
{
	MENU_CREATION_Printf("SelectionCursor::SelectionCursor: Element '%s': "
	                     "New selection cursor.\n", getName().c_str());
	
	const std::string filename("MenuElements\\hands");

	// FIXME: MUST SUPPORT NAMESPACES
	m_texture = engine::renderer::TextureCache::get(filename, "");
	if(m_texture == 0)
	{
		TT_PANIC("Loading selection cursor texture '%s' failed.",
		         filename.c_str());
	}
	else
	{
		// Create the quads for the cursors
		createCursorQuads();
	}
	
	// Set the minimum and requested dimensions
	setMinimumWidth(0);
	setMinimumHeight(0);
	setRequestedWidth(0);
	setRequestedHeight(0);
}


SelectionCursor::~SelectionCursor()
{
	MENU_CREATION_Printf("SelectionCursor::~SelectionCursor: Element '%s': "
	                     "Freeing resources.\n", getName().c_str());
	
	delete m_leftTranslation;
	delete m_rightTranslation;
	
	delete m_leftPosition;
	delete m_rightPosition;
}


void SelectionCursor::render(const math::PointRect& /* p_rect */, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	real z = static_cast<real>(p_z);
	
	if (m_leftTranslation != 0)
	{
		m_leftTranslation->update();
		math::Vector2 pos(m_leftTranslation->getPos());
		m_leftCursor->setPosition(math::Vector3(pos.x, pos.y, z));
	}
	
	if (m_leftPosition != 0)
	{
		m_leftPosition->update();
		m_leftCursor->setPositionX(m_leftCursor->getPosition().x +
		                           static_cast<real>(m_leftPosition->getX()));
	}
	
	if (m_rightTranslation != 0)
	{
		m_rightTranslation->update();
		math::Vector2 pos(m_rightTranslation->getPos());
		m_rightCursor->setPosition(math::Vector3(pos.x, pos.y, z));
	}
	
	if (m_rightPosition != 0)
	{
		m_rightPosition->update();
		m_rightCursor->setPositionX(m_rightCursor->getPosition().x -
		                            static_cast<real>(m_rightPosition->getX()));
	}
	
	// Render the cursors (position has been set previously by setSelectionRect)
	m_leftCursor->setPositionZ(z);
	m_rightCursor->setPositionZ(z);
	
	m_leftCursor->update();
	m_rightCursor->update();
	
	if (m_selectionType == SelectionCursorType_Left ||
	    m_selectionType == SelectionCursorType_Both)
	{
		m_leftCursor->render();
	}
	if (m_selectionType == SelectionCursorType_Right ||
	    m_selectionType == SelectionCursorType_Both)
	{
		m_rightCursor->render();
	}
}


SelectionCursor* SelectionCursor::clone() const
{
	return new SelectionCursor(*this);
}


void SelectionCursor::setSelectionRect(const math::PointRect& p_rect)
{
	if (m_leftTranslation == 0)
	{
		// Position the left cursor to the left of the rectangle
		// + 6 collin pixels for hand positioning (-2 daniel pixels)
		s32 leftX = p_rect.getPosition().x - (CURSOR_LEFT_WIDTH / 2);
		s32 leftY = p_rect.getCenterPosition().y;
		m_leftCursor->setPositionX(static_cast<real>(leftX) - 2.0f);
		m_leftCursor->setPositionY(static_cast<real>(leftY) + 4.0f);
	}
	else
	{
		math::Vector2 startPos(m_leftTranslation->getPos().x, 0.0f);
		math::Vector2 destPos(
			(p_rect.getPosition().x - (CURSOR_LEFT_WIDTH * 0.5f)) - 2.0f,
			0.0f);
		
		math::PointRect r(getRectangle());
		// FIXME: Remove hard-coded dimensions!
		if (r.getPosition() == math::Point2(0, 0) &&
		    r.getWidth() == 256 && r.getHeight() == 192)
		{
			startPos.y = p_rect.getCenterPosition().y + 4.0f;
		}
		else
		{
			startPos.y = m_leftTranslation->getPos().y;
		}
		
		// FIXME: Remove hard-coded dimensions!
		if (p_rect.getPosition() == math::Point2(0, 0) &&
		    p_rect.getWidth() == 256 && p_rect.getHeight() == 192)
		{
			destPos.y = m_leftTranslation->getPos().y;
		}
		else
		{
			destPos.y = p_rect.getCenterPosition().y + 4.0f;
		}
		
		m_leftTranslation->setStartPos(startPos);
		m_leftTranslation->setDestinationPos(destPos);
		m_leftTranslation->setDuration(3);
		m_leftTranslation->start();
	}
	
	if (m_rightTranslation == 0)
	{
		// Position the right cursor to the right of the rectangle
		// + 6 Collin pixels for hand positioning (-2 Daniel pixels)
		s32 rightX = p_rect.getRight() + (CURSOR_RIGHT_WIDTH / 2);
		s32 rightY = p_rect.getCenterPosition().y;
		m_rightCursor->setPositionX(static_cast<real>(rightX) + 2.0f);
		m_rightCursor->setPositionY(static_cast<real>(rightY) + 4.0f);
	}
	else
	{
		math::Vector2 startPos(m_rightTranslation->getPos().x, 0.0f);
		math::Vector2 destPos(
			(p_rect.getRight() + (CURSOR_RIGHT_WIDTH * 0.5f)) + 2.0f,
			0.0f);
		
		math::PointRect r(getRectangle());
		// FIXME: Remove hard-coded dimensions!
		if (r.getPosition() == math::Point2(0, 0) &&
		    r.getWidth() == 256 && r.getHeight() == 192)
		{
			startPos.y = p_rect.getCenterPosition().y + 4.0f;
		}
		else
		{
			startPos.y = m_rightTranslation->getPos().y;
		}
		
		// FIXME: Remove hard-coded dimensions!
		if (p_rect.getPosition() == math::Point2(0, 0) &&
		    p_rect.getWidth() == 256 && p_rect.getHeight() == 192)
		{
			destPos.y = m_rightTranslation->getPos().y;
		}
		else
		{
			destPos.y = p_rect.getCenterPosition().y + 4.0f;
		}
		
		m_rightTranslation->setStartPos(startPos);
		m_rightTranslation->setDestinationPos(destPos);
		m_rightTranslation->setDuration(3);
		m_rightTranslation->start();
	}
	
	// Update the element rectangle with the selection rectangle
	setRectangle(p_rect);
	m_selectionRect = p_rect;
}


void SelectionCursor::forceSelectionRect(const math::PointRect& p_rect)
{
	if (m_leftTranslation == 0)
	{
		// Position the left cursor to the left of the rectangle
		// + 6 Collin pixels for hand positioning (-2 Daniel pixels)
		s32 leftX = p_rect.getPosition().x - (CURSOR_LEFT_WIDTH / 2);
		s32 leftY = p_rect.getCenterPosition().y;
		m_leftCursor->setPositionX(static_cast<real>(leftX) - 2.0f);
		m_leftCursor->setPositionY(static_cast<real>(leftY) + 4.0f);
	}
	else
	{
		m_leftTranslation->setStartPos(math::Vector2(
			(p_rect.getPosition().x - (CURSOR_LEFT_WIDTH * 0.5f)) - 2.0f,
			p_rect.getCenterPosition().y + 4.0f));
		
		m_leftTranslation->setDestinationPos(math::Vector2(
			(p_rect.getPosition().x - (CURSOR_LEFT_WIDTH * 0.5f)) - 2.0f,
			p_rect.getCenterPosition().y + 4.0f));
		
		m_leftTranslation->setDuration(3);
		m_leftTranslation->start();
	}
	
	if (m_rightTranslation == 0)
	{
		// Position the right cursor to the right of the rectangle
		// + 6 Collin pixels for hand positioning (-2 Daniel pixels)
		s32 rightX = p_rect.getRight() + (CURSOR_RIGHT_WIDTH / 2);
		s32 rightY = p_rect.getCenterPosition().y;
		m_rightCursor->setPositionX(static_cast<real>(rightX) + 2.0f);
		m_rightCursor->setPositionY(static_cast<real>(rightY) + 4.0f);
	}
	else
	{
		m_rightTranslation->setStartPos(math::Vector2(
			(p_rect.getRight() + (CURSOR_RIGHT_WIDTH * 0.5f)) + 2.0f,
			p_rect.getCenterPosition().y + 4.0f));
		
		m_rightTranslation->setDestinationPos(math::Vector2(
			(p_rect.getRight() + (CURSOR_RIGHT_WIDTH * 0.5f)) + 2.0f,
			p_rect.getCenterPosition().y + 4.0f));
		
		m_rightTranslation->setDuration(3);
		m_rightTranslation->start();
	}
	
	// Update the element rectangle with the selection rectangle
	setRectangle(p_rect);
	m_selectionRect = p_rect;
}


//------------------------------------------------------------------------------
// Protected member functions

SelectionCursor::SelectionCursor(const SelectionCursor& p_rhs)
:
MenuElement(p_rhs),
m_texture(p_rhs.m_texture),
m_selectionRect(p_rhs.m_selectionRect)
{
	using engine::renderer::QuadSprite;
	if (p_rhs.m_leftCursor != 0)
	{
		m_leftCursor.reset(new QuadSprite(*(p_rhs.m_leftCursor)));
	}
	
	if (p_rhs.m_rightCursor != 0)
	{
		m_rightCursor.reset(new QuadSprite(*(p_rhs.m_rightCursor)));
	}
}


//------------------------------------------------------------------------------
// Private member functions

void SelectionCursor::createCursorQuads()
{
	using engine::renderer::QuadSprite;
	
	// Create the left cursor quad
	m_leftCursor = QuadSprite::createQuad(m_texture);
	
	m_leftCursor->setWidth(CURSOR_LEFT_WIDTH);
	m_leftCursor->setHeight(CURSOR_LEFT_HEIGHT);
	
	/*
	m_leftCursor->setTexcoordLeft(CURSOR_LEFT_X);
	m_leftCursor->setTexcoordTop(CURSOR_LEFT_Y);
	m_leftCursor->setTexcoordRight(CURSOR_LEFT_X + CURSOR_LEFT_WIDTH);
	m_leftCursor->setTexcoordBottom(CURSOR_LEFT_Y + CURSOR_LEFT_HEIGHT);
	*/
	
	
	// Create the right cursor quad
	m_rightCursor = QuadSprite::createQuad(m_texture);
	
	m_rightCursor->setWidth(CURSOR_RIGHT_WIDTH);
	m_rightCursor->setHeight(CURSOR_RIGHT_HEIGHT);
	
	/*
	m_rightCursor->setTexcoordLeft(CURSOR_RIGHT_X);
	m_rightCursor->setTexcoordTop(CURSOR_RIGHT_Y);
	m_rightCursor->setTexcoordRight(CURSOR_RIGHT_X + CURSOR_RIGHT_WIDTH);
	m_rightCursor->setTexcoordBottom(CURSOR_RIGHT_Y + CURSOR_RIGHT_HEIGHT);
	*/
	
	
	// Create translation and position animations
	m_leftTranslation  = new LinearTranslationAnimation;
	m_rightTranslation = new LinearTranslationAnimation;
	
	m_leftPosition = new BouncePositionAnimation;
	m_leftPosition->setSpeed(10);
	m_leftPosition->start();
	m_rightPosition = new BouncePositionAnimation;
	m_rightPosition->setSpeed(10);
	m_rightPosition->start();
}

// Namespace end
}
}
}
