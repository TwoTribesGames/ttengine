#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_ENTITYMOVETOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_ENTITYMOVETOOL_H


#include <map>

#include <toki/game/editor/commands/CommandMoveEntities.h>
#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class EntityMoveTool : public Tool
{
public:
	explicit EntityMoveTool(Editor* p_editor);
	virtual ~EntityMoveTool();
	
	virtual void onActivate();
	virtual void onDeactivate();
	
	virtual void onPointerHover       (const PointerState& p_pointerState);
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerLeftDown    (const PointerState& p_pointerState);
	virtual void onPointerLeftReleased(const PointerState& p_pointerState);
	
	virtual std::wstring getHelpText() const;
	
private:
	// Entity to move -> offset within entity (that the move started with)
	typedef std::map<level::entity::EntityInstancePtr, tt::math::Vector2> MoveEntities;
	
	
	void commitUndoCommand();
	
	
	MoveEntities                     m_entitiesToMove;
	commands::CommandMoveEntitiesPtr m_activeCommand;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_ENTITYMOVETOOL_H)
