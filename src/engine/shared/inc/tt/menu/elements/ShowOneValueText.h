#if !defined(INC_TT_MENU_ELEMENTS_SHOWONEVALUETEXT_H)
#define INC_TT_MENU_ELEMENTS_SHOWONEVALUETEXT_H


#include <tt/engine/glyph/GlyphSet.h>
#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

class DynamicLabel;


/*! \brief ShowOneValueText element.
           This container will only show one text string at a time.
           Next or Previous texts can be selected.
           It is also possible to change the text with an index. */
class ShowOneValueText : public MenuElement
{
public:
	ShowOneValueText(const std::string& p_name,
	                 const MenuLayout&  p_layout);
	virtual ~ShowOneValueText();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	/*! \return The value of the shown text. */
	std::string getValue() const;
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void onLayoutDone();
	
	virtual void setSelected(bool p_selected);
	
	virtual void setEnabled(bool p_enabled);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual void setInitialTextByValue(const std::string& p_value);
	
	/*! \brief Selects a direct text by the element's value.
	           Asserts if no direct text with the specified value was found. */
	virtual void selectTextByValue(const std::string& p_value);
	
	virtual void setUserLoopEnable(bool p_enabled);
	
	virtual ShowOneValueText* clone() const;
	
	void addText(const std::string& p_value, const std::string& p_locID);
	void addText(const std::string& p_value, const std::wstring& p_text);
	
	void selectPreviousText();
	void selectNextText();
	void selectTextByIndex(s32 p_index);
	
	
	// Text alignment functions
	engine::glyph::GlyphSet::Alignment getHorizontalAlign() const;
	engine::glyph::GlyphSet::Alignment getVerticalAlign()   const;
	
	void setHorizontalAlign(engine::glyph::GlyphSet::Alignment p_align);
	void setVerticalAlign(engine::glyph::GlyphSet::Alignment p_align);
	
protected:
	ShowOneValueText(const ShowOneValueText& p_rhs);
	
private:
	struct ValueText
	{
		std::string  value;
		std::wstring text;
	};
	
	typedef std::vector<ValueText> TextVector;
	
	
	// No assignment
	const ShowOneValueText& operator=(const ShowOneValueText&);
	
	
	std::string           m_initialTextValue;
	TextVector            m_texts;
	TextVector::size_type m_selectedTextIndex;
	bool                  m_looping;
	DynamicLabel*         m_label;
	bool                  m_shouldInit;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SHOWONEVALUETEXT_H)
