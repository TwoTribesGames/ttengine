#include <toki/game/editor/commands/CommandPaintTiles.h>
#include <toki/level/AttributeLayer.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandPaintTilesPtr CommandPaintTiles::create(const level::AttributeLayerPtr& p_layer,
                                               level::CollisionType            p_paintType)
{
	TT_NULL_ASSERT(p_layer);
	if (p_layer == 0)
	{
		return CommandPaintTilesPtr();
	}
	
	return CommandPaintTilesPtr(new CommandPaintTiles(p_layer, p_paintType));
}


CommandPaintTilesPtr CommandPaintTiles::create(const level::AttributeLayerPtr& p_layer,
                                               level::ThemeType                p_paintType)
{
	TT_NULL_ASSERT(p_layer);
	if (p_layer == 0)
	{
		return CommandPaintTilesPtr();
	}
	
	return CommandPaintTilesPtr(new CommandPaintTiles(p_layer, p_paintType));
}


CommandPaintTilesPtr CommandPaintTiles::createWithRect(const level::AttributeLayerPtr& p_layer,
                                                       level::CollisionType            p_paintType,
                                                       const tt::math::PointRect&      p_tileRectToPaint)
{
	// FIXME: Create valid rect (entirely within layer bounds) from input rect
	tt::math::PointRect validRect(p_tileRectToPaint);
	
	
	CommandPaintTilesPtr command(create(p_layer, p_paintType));
	if (command != 0)
	{
		// Add tiles for each location in the rect, using unchecked function (faster)
		const tt::math::Point2 minPos(validRect.getMin());
		const tt::math::Point2 maxPos(validRect.getMaxEdge());
		tt::math::Point2 pos;
		for (pos.y = minPos.y; pos.y < maxPos.y; ++pos.y)
		{
			for (pos.x = minPos.x; pos.x < maxPos.x; ++pos.x)
			{
				command->uncheckedAddTile(pos);
			}
		}
	}
	
	return command;
}


CommandPaintTiles::~CommandPaintTiles()
{
}


void CommandPaintTiles::redo()
{
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
	
	// Paint all locations with the specified paint type
	if (m_useCollisionType)
	{
		for (level::CollisionTiles::iterator it = m_collisionTiles.begin(); it != m_collisionTiles.end(); ++it)
		{
			m_layer->setCollisionType((*it).first, m_paintCollisionType);
		}
	}
	else
	{
		for (level::ThemeTiles::iterator it = m_themeTiles.begin(); it != m_themeTiles.end(); ++it)
		{
			m_layer->setThemeType((*it).first, m_paintThemeType);
		}
	}
}


void CommandPaintTiles::undo()
{
	// Restore all locations to the original values
	if (m_useCollisionType)
	{
		for (level::CollisionTiles::iterator it = m_collisionTiles.begin(); it != m_collisionTiles.end(); ++it)
		{
			m_layer->setCollisionType((*it).first, (*it).second);
		}
	}
	else
	{
		for (level::ThemeTiles::iterator it = m_themeTiles.begin(); it != m_themeTiles.end(); ++it)
		{
			m_layer->setThemeType((*it).first, (*it).second);
		}
	}
}


void CommandPaintTiles::addTile(const tt::math::Point2& p_pos)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call addTile after this command has already been added to an undo stack.");
		return;
	}
	
	// Ignore this position if it is out of bounds or if the position was already painted
	if (m_layer->contains(p_pos) == false ||
	    hasPosition(p_pos))
	{
		return;
	}
	
	uncheckedAddTile(p_pos);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandPaintTiles::CommandPaintTiles(const level::AttributeLayerPtr& p_layer,
                                     level::CollisionType            p_paintType)
:
tt::undo::UndoCommand(L"Paint Collision Tiles"),
m_addedToStack(false),
m_layer(p_layer),
m_useCollisionType(true),
m_paintCollisionType(p_paintType),
m_paintThemeType(level::ThemeType_Invalid),
m_collisionTiles(),
m_themeTiles()
{
}


CommandPaintTiles::CommandPaintTiles(const level::AttributeLayerPtr& p_layer,
                                     level::ThemeType                p_paintType)
:
tt::undo::UndoCommand(L"Paint Theme Tiles"),
m_addedToStack(false),
m_layer(p_layer),
m_useCollisionType(false),
m_paintCollisionType(level::CollisionType_Invalid),
m_paintThemeType(p_paintType),
m_collisionTiles(),
m_themeTiles()
{
}


bool CommandPaintTiles::hasPosition(const tt::math::Point2& p_pos) const
{
	return m_useCollisionType ?
			m_collisionTiles.find(p_pos) != m_collisionTiles.end() :
			m_themeTiles    .find(p_pos) != m_themeTiles.end();
}


void CommandPaintTiles::uncheckedAddTile(const tt::math::Point2& p_pos)
{
	if (m_useCollisionType)
	{
		// Add the location to the list and remember the existing layer value
		m_collisionTiles[p_pos] = m_layer->getCollisionType(p_pos);
		
		// Also immediately apply the change to the layer (for instant visual feedback)
		m_layer->setCollisionType(p_pos, m_paintCollisionType);
	}
	else
	{
		m_themeTiles[p_pos] = m_layer->getThemeType(p_pos);
		m_layer->setThemeType(p_pos, m_paintThemeType);
	}
}

// Namespace end
}
}
}
}
