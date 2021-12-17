#if !defined(INC_TOKI_GAME_EDITOR_UI_LEVELNAMETEXTBOX_H)
#define INC_TOKI_GAME_EDITOR_UI_LEVELNAMETEXTBOX_H


#include <Gwen/Controls/TextBox.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Text box that only allows valid level name characters. */
class LevelNameTextBox : public Gwen::Controls::TextBox
{
public:
	GWEN_CONTROL_INLINE(LevelNameTextBox, TextBox)
	{ }
	virtual ~LevelNameTextBox() { }
	
protected:
	virtual bool IsTextAllowed(const Gwen::UnicodeString& p_typedText, int p_insertionPos)
	{
		if (p_typedText.empty())
		{
			return true;
		}
		
		// Scan for disallowed characters
		if (p_typedText.find_first_of(L"/\\*?<>|:;^") != Gwen::UnicodeString::npos)
		{
			return false;
		}
		
		// Check new full text correctness
		Gwen::UnicodeString newFullText(GetText().GetUnicode());
		newFullText.insert(p_insertionPos, p_typedText);
		
		// - Limit level names to a specific length (which the target file systems can handle)
		if (newFullText.length() > 128)
		{
			return false;
		}
		
		// - Disallow leading periods and whitespace
		if (newFullText[0] == L'.' ||
		    newFullText[0] == L' ' ||
		    newFullText[0] == L'\t')
		{
			return false;
		}
		
		return true;
	}
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_LEVELNAMETEXTBOX_H)
