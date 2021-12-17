#if !defined(INC_TOKI_GAME_EDITOR_UI_DIALOGBOXBASE_H)
#define INC_TOKI_GAME_EDITOR_UI_DIALOGBOXBASE_H


#include <vector>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/WindowControl.h>

#include <tt/input/Button.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Base class for level editor dialog boxes. Cannot be used on its own. */
class DialogBoxBase : public Gwen::Controls::WindowControl
{
public:
	enum Result
	{
		Result_OK,
		Result_Cancel,
		
		Result_Yes,
		Result_No,
		
		Result_Save,
		Result_Discard,
		
		Result_Retry,
		
		Result_DialogStillOpen,  // no result yet: dialog is still open
		
		Result_Count
	};
	
	
	GWEN_CONTROL(DialogBoxBase, Gwen::Controls::WindowControl);
	virtual ~DialogBoxBase();
	
	void handleKeyInput(const tt::input::Button* p_allKeyboardKeys);
	
	void setPromptText(const Gwen::TextObject& p_text);
	
	inline Result getResult() const { return m_dialogResult; }
	
	inline static bool isValidResult(Result p_result) { return p_result >= 0 && p_result < Result_Count; }
	
protected:
	virtual void doHandleKeyInput(const tt::input::Button* p_allKeyboardKeys,
	                              bool                     p_controlDown,
	                              bool                     p_altDown,
	                              bool                     p_shiftDown,
	                              bool                     p_noModifiersDown);
	virtual void handleResultSet();
	
	// Sets the dialog result and closes the dialog
	void setResult(Result p_result);
	
	// Creates an action button (button that dismisses the dialog) for the specified result
	// and adds it to internal bookkeeping
	Gwen::Controls::Button* addActionButton(Gwen::Controls::Base*   p_parent,
	                                        const Gwen::TextObject& p_title,
	                                        Result                  p_result);
	
	// Turns the specified button into a button that dismisses the dialog with the specified result
	void makeDialogCloser(Gwen::Controls::Button* p_button, Result p_result);
	
	// Convenience GWEN event handler that sets the dialog result to the value of
	// the "dialogResult" piece of user data (must be a Result enum)
	void setResultToSendersUserData(Gwen::Controls::Base* p_sender);
	
	// Verifies that the specified result is a valid value to dismiss the dialog with.
	// Panics and returns false if not.
	static bool verifyValidCloseResult(Result p_result);
	
	// Destroys the UI elements that were automatically created by DialogBoxBase
	void destroyAutoCreatedUi();
	
private:
	// No copying
	DialogBoxBase(const DialogBoxBase&);
	DialogBoxBase& operator=(const DialogBoxBase&);
	
	
	typedef std::vector<Gwen::Controls::Button*> ActionButtons;
	
	Result                 m_dialogResult;
	Gwen::Controls::Label* m_labelPrompt;
	ActionButtons          m_actionButtons;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_DIALOGBOXBASE_H)
