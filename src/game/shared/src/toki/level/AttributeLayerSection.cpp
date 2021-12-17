#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/level/AttributeLayer.h>
#include <toki/level/AttributeLayerSection.h>


namespace toki {
namespace level {

//--------------------------------------------------------------------------------------------------
// Public member functions

AttributeLayerSectionPtr AttributeLayerSection::createFromLayerRect(
		const AttributeLayerPtr&   p_layerToCopyFrom,
		const tt::math::PointRect& p_rectToCopy)
{
	if (p_layerToCopyFrom == 0)
	{
		TT_PANIC("Invalid source layer specified. Use createEmpty if an empty section is needed.");
		return AttributeLayerSectionPtr();
	}
	
	AttributeLayerSectionPtr section = createEmpty(p_rectToCopy);
	if (section == 0)
	{
		return AttributeLayerSectionPtr();
	}
	
	section->copyFromLayer(p_layerToCopyFrom);
	
	return section;
}


AttributeLayerSectionPtr AttributeLayerSection::createFromText(
		const tt::str::Strings& p_lines,
		const tt::math::Point2& p_pos)
{
	// Determine the tile rectangle from the tiles
	tt::math::PointRect tileRect(tt::math::Point2::zero, 0, static_cast<s32>(p_lines.size()));
	for (tt::str::Strings::const_iterator lineIt = p_lines.begin(); lineIt != p_lines.end(); ++lineIt)
	{
		const s32 lineLen = static_cast<s32>((*lineIt).length());
		if (lineLen > tileRect.getWidth())
		{
			tileRect.setWidth(lineLen);
		}
	}
	
	if (tileRect.hasArea() == false)
	{
		TT_PANIC("No tiles were specified.");
		return AttributeLayerSectionPtr();
	}
	
	tileRect.translate(p_pos);
	
	
	AttributeLayerSectionPtr section = createEmpty(tileRect);
	if (section == 0)
	{
		return AttributeLayerSectionPtr();
	}
	
	tt::math::Point2 pos(0, 0);
	
	for (tt::str::Strings::const_reverse_iterator lineIt = p_lines.rbegin();
	     lineIt != p_lines.rend(); ++lineIt, ++pos.y)
	{
		pos.x = 0;
		
		for (std::string::const_iterator colIt = (*lineIt).begin(); colIt != (*lineIt).end(); ++colIt, ++pos.x)
		{
			const level::CollisionType collType = level::getCollisionTypeFromChar(*colIt);
			if (level::isValidCollisionType(collType))
			{
				section->m_attributes->setCollisionType(pos, collType);
			}
			else
			{
				TT_WARN("Unsupported tile: '%c'", *colIt);
			}
		}
	}
	
	return section;
}


AttributeLayerSectionPtr AttributeLayerSection::createFromRect(const tt::math::PointRect& p_tileRect,
                                                               level::CollisionType       p_type)
{
	if (level::isValidCollisionType(p_type) == false)
	{
		TT_PANIC("Unsupported collision type: %d", p_type);
		return AttributeLayerSectionPtr();
	}
	
	if (p_tileRect.hasArea() == false)
	{
		TT_PANIC("No tiles were specified.");
		return AttributeLayerSectionPtr();
	}
	
	AttributeLayerSectionPtr section = createEmpty(p_tileRect);
	if (section == 0)
	{
		return AttributeLayerSectionPtr();
	}
	
	const AttributeLayerPtr& layer = section->getAttributeLayer();
	TT_NULL_ASSERT(layer);
	
	const s32 width  = layer->getWidth();
	const s32 height = layer->getHeight();
	u8* dataPtr      = layer->getRawData();
	
	TT_ASSERT(width == p_tileRect.getWidth());
	TT_ASSERT(height == p_tileRect.getHeight());
	
	for (tt::math::Point2 pos(0, 0); pos.y < height; ++pos.y)
	{
		for (pos.x = 0; pos.x < width; ++pos.x, ++dataPtr)
		{
			level::setCollisionType(*dataPtr, p_type);
		}
	}
	
	return section;
}


AttributeLayerSectionPtr AttributeLayerSection::createEmpty(const tt::math::PointRect& p_rect)
{
	if (p_rect.getWidth() == 0 || p_rect.getHeight() == 0)
	{
		TT_PANIC("Attribute layer sections must be at least 1 x 1 tile (requested %d x %d).",
		         p_rect.getWidth(), p_rect.getHeight());
		return AttributeLayerSectionPtr();
	}
	
	AttributeLayerPtr attribs = AttributeLayer::create(p_rect.getWidth(), p_rect.getHeight());
	if (attribs == 0)
	{
		return AttributeLayerSectionPtr();
	}
	
	attribs->clear();
	
	return AttributeLayerSectionPtr(new AttributeLayerSection(p_rect.getPosition(), attribs));
}


AttributeLayerSection::~AttributeLayerSection()
{
}


void AttributeLayerSection::copyThemeTilesFromText(const tt::str::Strings& p_lines)
{
	// Determine the tile rectangle from the tiles
	tt::math::PointRect tileRect(tt::math::Point2::zero, 0, static_cast<s32>(p_lines.size()));
	for (tt::str::Strings::const_iterator lineIt = p_lines.begin(); lineIt != p_lines.end(); ++lineIt)
	{
		const s32 lineLen = static_cast<s32>((*lineIt).length());
		if (lineLen > tileRect.getWidth())
		{
			tileRect.setWidth(lineLen);
		}
	}
	
	if (tileRect.hasArea() == false)
	{
		TT_PANIC("No tiles were specified.");
		return;
	}
	
	const s32 sectionWidth  = m_attributes->getWidth();
	const s32 sectionHeight = m_attributes->getHeight();
	TT_ASSERTMSG(tileRect.getWidth()  <= sectionWidth &&
	             tileRect.getHeight() <= sectionHeight,
	             "Plain-text theme tiles are too wide (%d) or high (%d) for this layer section (%d x %d).\n"
	             "Not all tiles will be copied.",
	             tileRect.getWidth(), tileRect.getHeight(), sectionWidth, sectionHeight);
	
	tt::math::Point2 pos(0, 0);
	
	for (tt::str::Strings::const_reverse_iterator lineIt = p_lines.rbegin();
	     lineIt != p_lines.rend() && pos.y <= sectionHeight; ++lineIt, ++pos.y)
	{
		pos.x = 0;
		
		for (std::string::const_iterator colIt = (*lineIt).begin();
		     colIt != (*lineIt).end() && pos.x <= sectionWidth; ++colIt, ++pos.x)
		{
			const level::ThemeType themeType = level::getThemeTypeFromChar(*colIt);
			if (level::isValidThemeType(themeType))
			{
				m_attributes->setThemeType(pos, themeType);
			}
			else
			{
				TT_WARN("Unsupported theme tile: '%c'", *colIt);
			}
		}
	}
}


void AttributeLayerSection::copyFromLayer(const AttributeLayerPtr& p_sourceLayer)
{
	tt::math::Point2 pos(0, 0);
	for (pos.y = 0; pos.y < m_attributes->getHeight(); ++pos.y)
	{
		for (pos.x = 0; pos.x < m_attributes->getWidth(); ++pos.x)
		{
			const tt::math::Point2 sourcePos(m_position + pos);
			const bool inSourceBounds = p_sourceLayer->contains(sourcePos);
			
			m_attributes->setCollisionType(
				pos,
				inSourceBounds ?
					p_sourceLayer->getCollisionType(sourcePos) :
					CollisionType_Air);
			
			m_attributes->setThemeType(
				pos,
				inSourceBounds ?
					p_sourceLayer->getThemeType(sourcePos) :
					ThemeType_UseLevelDefault);
		}
	}
}


void AttributeLayerSection::applyToLayer(const AttributeLayerPtr& p_targetLayer,
                                         bool                     p_applyAirTiles) const
{
	tt::math::Point2 pos(0, 0);
	for (pos.y = 0; pos.y < m_attributes->getHeight(); ++pos.y)
	{
		for (pos.x = 0; pos.x < m_attributes->getWidth(); ++pos.x)
		{
			const tt::math::Point2 targetPos(m_position + pos);
			
			if (p_targetLayer->contains(targetPos))
			{
				if (p_applyAirTiles || isAir(m_attributes->getCollisionType(pos)) == false)
				{
					p_targetLayer->setCollisionType(targetPos, m_attributes->getCollisionType(pos));
				}
				
				p_targetLayer->setThemeType(targetPos, m_attributes->getThemeType(pos));
			}
		}
	}
}


tt::math::PointRect AttributeLayerSection::getRect() const
{
	return tt::math::PointRect(m_position, m_attributes->getWidth(), m_attributes->getHeight());
}


bool AttributeLayerSection::contains(const tt::math::Point2& p_pos) const
{
	return p_pos.x >= m_position.x                           &&
	       p_pos.y >= m_position.y                           &&
	       p_pos.x < m_position.x + m_attributes->getWidth() &&
	       p_pos.y < m_position.y + m_attributes->getHeight();
}


AttributeLayerSectionPtr AttributeLayerSection::clone() const
{
	return AttributeLayerSectionPtr();
}


#if !defined(TT_BUILD_FINAL)
void AttributeLayerSection::debugPrint() const
{
	TT_Printf("AttributeLayerSection::debugPrint: Section at pos (%d, %d), size %d x %d:\n",
	          m_position.x, m_position.y, m_attributes->getWidth(), m_attributes->getHeight());
	
	tt::math::Point2 pos;
	TT_Printf("AttributeLayerSection::debugPrint: +%s+\n", std::string(m_attributes->getWidth(), '-').c_str());
	for (pos.y = m_attributes->getHeight() - 1; pos.y >= 0; --pos.y)
	{
		TT_Printf("AttributeLayerSection::debugPrint: |");
		for (pos.x = 0; pos.x < m_attributes->getWidth(); ++pos.x)
		{
			const CollisionType type = m_attributes->getCollisionType(pos);
			if (isValidCollisionType(type))
			{
				TT_Printf("%c", getCollisionTypeAsChar(type));
			}
			else
			{
				TT_Printf("?");
			}
		}
		TT_Printf("|\n");
	}
	TT_Printf("AttributeLayerSection::debugPrint: +%s+\n", std::string(m_attributes->getWidth(), '-').c_str());
}
#endif


//--------------------------------------------------------------------------------------------------
// Private member functions

AttributeLayerSection::AttributeLayerSection(const tt::math::Point2&  p_position,
                                             const AttributeLayerPtr& p_attributes)
:
m_position(p_position),
m_attributes(p_attributes)
{
}

// Namespace end
}
}
