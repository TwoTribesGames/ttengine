#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_ENTITYPAINTTOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_ENTITYPAINTTOOL_H


#include <toki/game/editor/commands/CommandAddRemoveEntities.h>
#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class EntityPaintTool : public Tool
{
public:
	explicit EntityPaintTool(Editor* p_editor);
	virtual ~EntityPaintTool();
	
	virtual void onActivate();
	virtual void onDeactivate();
	virtual bool canDeactivate() const;
	
	virtual void onPointerHover(const PointerState& p_pointerState);
	
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerLeftDown    (const PointerState& p_pointerState);
	virtual void onPointerLeftReleased(const PointerState& p_pointerState);
	
	virtual void onPointerRightPressed (const PointerState& p_pointerState);
	virtual void onPointerRightDown    (const PointerState& p_pointerState);
	virtual void onPointerRightReleased(const PointerState& p_pointerState);
	
	virtual std::wstring getHelpText() const;
	
private:
	void commitUndoCommand();
	
	
	level::entity::EntityInstancePtr      m_placingEntity; //!< Entity that was recently placed by pressing left button.
	commands::CommandAddRemoveEntitiesPtr m_activeCommand;
	bool                                  m_inDrawMode;    //!< Whether drawing or erasing.
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_ENTITYPAINTTOOL_H)
