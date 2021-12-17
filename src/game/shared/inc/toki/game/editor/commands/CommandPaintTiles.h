#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDPAINTTILES_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDPAINTTILES_H



#include <tt/math/Point2.h>
#include <tt/math/Rect.h>
#include <tt/undo/UndoCommand.h>

#include <toki/level/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandPaintTiles;
typedef tt_ptr<CommandPaintTiles>::shared CommandPaintTilesPtr;


class CommandPaintTiles : public tt::undo::UndoCommand
{
public:
	static CommandPaintTilesPtr create(const level::AttributeLayerPtr& p_layer,
	                                   level::CollisionType            p_paintType);
	static CommandPaintTilesPtr create(const level::AttributeLayerPtr& p_layer,
	                                   level::ThemeType                p_paintType);
	static CommandPaintTilesPtr createWithRect(const level::AttributeLayerPtr& p_layer,
	                                           level::CollisionType            p_paintType,
	                                           const tt::math::PointRect&      p_tileRectToPaint);
	virtual ~CommandPaintTiles();
	
	virtual void redo();
	virtual void undo();
	
	/*! \brief Adds a tile location to paint (and paints it).
	           Only allowed if the command has not been added to the stack yet. */
	void addTile(const tt::math::Point2& p_pos);
	
	/*! \brief Indicates whether any tiles were added to this command. */
	inline bool hasTiles() const { return m_collisionTiles.empty() == false || m_themeTiles.empty() == false; }
	
private:
	CommandPaintTiles(const level::AttributeLayerPtr& p_layer,
	                  level::CollisionType            p_paintType);
	CommandPaintTiles(const level::AttributeLayerPtr& p_layer,
	                  level::ThemeType                p_paintType);
	bool hasPosition(const tt::math::Point2& p_pos) const;
	
	/*! \brief Adds the specified position to the draw list, without performing any bounds or sanity checks. */
	void uncheckedAddTile(const tt::math::Point2& p_pos);
	
	
	bool                       m_addedToStack; //!< For sanity checking: whether this command has been added to the undo stack
	level::AttributeLayerPtr   m_layer;
	const bool                 m_useCollisionType; // paint collision type or theme type?
	const level::CollisionType m_paintCollisionType;
	const level::ThemeType     m_paintThemeType;
	
	// Maps tile position to the original tile value at that position (before painting overwrote it)
	level::CollisionTiles      m_collisionTiles;
	level::ThemeTiles          m_themeTiles;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDPAINTTILES_H)
