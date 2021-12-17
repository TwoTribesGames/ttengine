#include <tt/code/helpers.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_printf.h>

#include <toki/level/AttributeLayer.h>
#include <toki/level/TileChangedObserver.h>


namespace toki {
namespace level {

//--------------------------------------------------------------------------------------------------
// Public member functions

AttributeLayerPtr AttributeLayer::create(s32 p_width, s32 p_height)
{
	TT_ASSERTMSG(p_width  > 0, "Invalid attribute layer width: %d. Must be at least 1.",  p_width);
	TT_ASSERTMSG(p_height > 0, "Invalid attribute layer height: %d. Must be at least 1.", p_height);
	if (p_width <= 0 || p_height <= 0)
	{
		return AttributeLayerPtr();
	}
	
	return AttributeLayerPtr(new AttributeLayer(p_width, p_height));
}


AttributeLayer::~AttributeLayer()
{
	tt::code::helpers::safeDeleteArray(m_attributes);
}


void AttributeLayer::flipRows()
{
	const tt::mem::size_type rowBytes = static_cast<tt::mem::size_type>(m_width);
	u8* tempRow = new u8[m_width];
	for (s32 y = 0; y < m_height / 2; ++y)
	{
		u8* upperRow = m_attributes + (y                  * m_width);
		u8* lowerRow = m_attributes + ((m_height - y - 1) * m_width);
		
		tt::mem::copy8(tempRow,  upperRow, rowBytes);
		tt::mem::copy8(upperRow, lowerRow, rowBytes);
		tt::mem::copy8(lowerRow, tempRow,  rowBytes);
	}
	delete[] tempRow;
	
	makeDirty();
}


void AttributeLayer::clear()
{
	tt::mem::zero8(m_attributes, static_cast<tt::mem::size_type>(getLength()));
	makeDirty();
}


void AttributeLayer::resizeTo(const tt::math::PointRect& p_newRect)
{
	if (p_newRect.hasArea() == false)
	{
		TT_PANIC("Invalid new layer dimensions: %d x %d. Must be at least 1 x 1.",
		         p_newRect.getWidth(), p_newRect.getHeight());
		return;
	}
	
	if (p_newRect.getPosition()    == tt::math::Point2(0, 0) &&
	    p_newRect.getWidth()  == m_width                &&
	    p_newRect.getHeight() == m_height)
	{
		// No change in rectangle: nothing to do
		return;
	}
	
	// Allocate new buffer
	const s32 newDataLen = p_newRect.getWidth() * p_newRect.getHeight();
	u8* newData = new u8[newDataLen];
	tt::mem::zero8(newData, static_cast<tt::mem::size_type>(newDataLen));
	
	const u8 fillTile = 0;
	
	// Copy over existing data
	s32 srcWidth   = m_width;
	s32 srcHeight  = m_height;
	s32 destWidth  = p_newRect.getWidth();
	s32 destHeight = p_newRect.getHeight();
	
	u8* src  = m_attributes;
	u8* dest = newData;
	
	s32 srcY = 0;
	for (s32 destY = -p_newRect.getMin().y; destY < destHeight; ++destY)
	{
		if (destY >= 0)
		{
			if (srcY < srcHeight)
			{
				// Copy existing tile row
				s32 srcX = 0;
				
				for (s32 destX = -p_newRect.getMin().x; destX < destWidth; ++destX)
				{
					if (destX >= 0)
					{
						if (srcX < srcWidth)
						{
							// Copy existing tile data
							dest[(destY * destWidth) + destX] = src[(srcY * srcWidth) + srcX];
						}
						else
						{
							// Fill with fill tile
							dest[(destY * destWidth) + destX] = fillTile;
						}
					}
					
					++srcX;
				}
			}
			else
			{
				// Fill row with fill tile
				s32 offset = destY * destWidth;
				for (s32 x = 0; x < destWidth; ++x)
				{
					dest[offset] = fillTile;
					++offset;
				}
			}
		}
		
		++srcY;
	}
	
	
	// Destroy old buffer and save new details
	delete[] m_attributes;
	m_attributes = newData;
	m_width      = p_newRect.getWidth();
	m_height     = p_newRect.getHeight();
	
	makeDirty();
}


void AttributeLayer::swap(const AttributeLayerPtr& p_other)
{
	TT_NULL_ASSERT(p_other);
	if (p_other == 0 || p_other.get() == this)
	{
		return;
	}
	
	using std::swap;
	
	swap(m_attributes, p_other->m_attributes);
	swap(m_width,      p_other->m_width);
	swap(m_height,     p_other->m_height);
	
	// Swapping makes both layers dirty
	makeDirty();
	p_other->makeDirty();
}


bool AttributeLayer::equals(const AttributeLayerPtr& p_other) const
{
	if (p_other == 0) return false;
	if (p_other.get() == this) return true;
	if (m_height != p_other->m_height || m_width != p_other->m_width) return false;
	
	for(s32 y = 0; y < m_height; ++y)
	{
		for(s32 x = 0; x < m_width; ++x)
		{
			const s32 index = ((m_height - y - 1) * m_width) + x;
			if (m_attributes[index] != p_other->m_attributes[index])
			{
				return false;
			}
		}
	}
	
	return true;
}


AttributeLayerPtr AttributeLayer::clone() const
{
	AttributeLayerPtr clonedLayer(new AttributeLayer(m_width, m_height));
	tt::mem::copy8(clonedLayer->m_attributes, m_attributes, static_cast<tt::mem::size_type>(getLength()));
	clonedLayer->m_onTileChangedObservers = m_onTileChangedObservers;
	return clonedLayer;
}


void AttributeLayer::setCollisionType(const tt::math::Point2& p_pos, CollisionType p_type)
{
	if (contains(p_pos) == false)
	{
		TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
		         p_pos.x, p_pos.y, m_width - 1, m_height - 1);
		return;
	}
	
	TT_ASSERT(isValidCollisionType(p_type));
	
	const s32 index = (p_pos.y * m_width) + p_pos.x;
	level::setCollisionType(m_attributes[index], p_type);
	
	notifyTileChanged(p_pos);
}


void AttributeLayer::setThemeType(const tt::math::Point2& p_pos, ThemeType p_type)
{
	if (contains(p_pos) == false)
	{
		TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
		         p_pos.x, p_pos.y, m_width - 1, m_height - 1);
		return;
	}
	
	TT_ASSERT(isValidThemeType(p_type));
	
	const s32 index = (p_pos.y * m_width) + p_pos.x;
	level::setThemeType(m_attributes[index], p_type);
	
	makeDirty();
	// FIXME: Does changing the theme of a tile need to notify tile changed observers as well?
}


void AttributeLayer::registerObserver(const TileChangedObserverPtr& p_observer)
{
	TT_NULL_ASSERT(p_observer);
	
	for (TileChangedObservers::iterator it = m_onTileChangedObservers.begin(); 
	     it != m_onTileChangedObservers.end(); ++it)
	{
		if ((*it).lock() == p_observer)
		{
			TT_PANIC("Observer 0x%08X is already registered.", p_observer.get());
			return;
		}
	}
	
	m_onTileChangedObservers.push_back(p_observer);
}


void AttributeLayer::unregisterObserver(const TileChangedObserverPtr& p_observer)
{
	TT_NULL_ASSERT(p_observer);
	
	for (TileChangedObservers::iterator it = m_onTileChangedObservers.begin(); 
	     it != m_onTileChangedObservers.end(); ++it)
	{
		if ((*it).lock() == p_observer)
		{
			m_onTileChangedObservers.erase(it);
			return;
		}
	}
	
	TT_PANIC("Observer 0x%08X was not registered.", p_observer.get());
}


void AttributeLayer::print() const
{
#ifndef TT_BUILD_FINAL
	for(s32 y = 0; y < m_height; ++y)
	{
		for(s32 x = 0; x < m_width; ++x)
		{
			const s32 index = ((m_height - y - 1) * m_width) + x;
			TT_Printf("%3u ", m_attributes[index]);
		}
		TT_Printf("\n");
	}
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

AttributeLayer::AttributeLayer(s32 p_width, s32 p_height)
:
m_attributes(new u8[p_width * p_height]),
m_width(p_width),
m_height(p_height),
m_onTileChangedObservers()
{
}


void AttributeLayer::makeDirty()
{
	// Notify observers that tile layer became dirty
	for (TileChangedObservers::iterator it(m_onTileChangedObservers.begin());
	     it != m_onTileChangedObservers.end(); )
	{
		TileChangedObserverPtr observer((*it).lock());
		if (observer == 0)
		{
			it = m_onTileChangedObservers.erase(it);
		}
		else
		{
			observer->onTileLayerDirty();
			++it;
		}
	}
}


void AttributeLayer::notifyTileChanged(const tt::math::Point2& p_tilePos)
{
	// Notify observers of tile change
	for (TileChangedObservers::iterator it(m_onTileChangedObservers.begin());
	     it != m_onTileChangedObservers.end(); )
	{
		TileChangedObserverPtr observer((*it).lock());
		if (observer == 0)
		{
			it = m_onTileChangedObservers.erase(it);
		}
		else
		{
			observer->onTileLayerDirty(); // a tile change also means the layer as a whole has been changed
			observer->onTileChange(p_tilePos);
			++it;
		}
	}
}

// Namespace end
}
}
