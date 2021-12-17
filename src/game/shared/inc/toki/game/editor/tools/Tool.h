#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_TOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_TOOL_H


#include <string>

#include <tt/input/Button.h>
#include <tt/input/Pointer.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>

#include <toki/game/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class Tool
{
public:
	struct PointerState
	{
		tt::math::Vector2  worldPos;
		tt::input::Pointer screenPosCur;
		tt::input::Pointer screenPosPrev;
		tt::input::Button  ctrl;
		tt::input::Button  alt;
		tt::input::Button  shift;
		bool               capsLockOn;
		bool               scrollLockOn;
		bool               numLockOn;
		
		
		inline PointerState()
		:
		worldPos(tt::math::Vector2::zero),
		screenPosCur(),
		screenPosPrev(),
		ctrl(),
		alt(),
		shift(),
		capsLockOn(false),
		scrollLockOn(false),
		numLockOn(false)
		{ }
	};
	
	
	/*! \param p_editor The editor to which this tool belongs. */
	explicit Tool(Editor* p_editor);
	virtual ~Tool();
	
	/*! \brief Called when this tool is activated. */
	virtual void onActivate();
	
	/*! \brief Called when this tool is deactivated. */
	virtual void onDeactivate();
	
	/*! \brief Indicates whether this tool can currently be activated */
	virtual bool canActivate(const PointerState& p_pointerState) const;
	
	/*! \brief Indicates whether this tool can currently be deactivated (e.g. to switch to a different tool). */
	virtual bool canDeactivate() const;
	
	virtual void onPointerHover(const PointerState& p_pointerState);
	
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerLeftDown    (const PointerState& p_pointerState);
	virtual void onPointerLeftReleased(const PointerState& p_pointerState);
	
	virtual void onPointerMiddlePressed (const PointerState& p_pointerState);
	virtual void onPointerMiddleDown    (const PointerState& p_pointerState);
	virtual void onPointerMiddleReleased(const PointerState& p_pointerState);
	
	virtual void onPointerRightPressed (const PointerState& p_pointerState);
	virtual void onPointerRightDown    (const PointerState& p_pointerState);
	virtual void onPointerRightReleased(const PointerState& p_pointerState);
	
	/*! \brief Called when pointer enters the drawing area. */
	virtual void onPointerEnter();
	
	/*! \brief Called when the pointer leaves the drawing area (e.g. when over the editor GUI). */
	virtual void onPointerLeave();
	
	/*! \brief This function should return a short help text for this tool, which will be displayed in the status bar. */
	virtual std::wstring getHelpText() const;
	
protected:
	inline Editor*       getEditor()       { return m_editor; }
	inline const Editor* getEditor() const { return m_editor; }
	
private:
	// No copying
	Tool(const Tool&);
	Tool& operator=(const Tool&);
	
	
	Editor* m_editor;
};

typedef tt_ptr<Tool>::shared ToolPtr;

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_TOOL_H)
