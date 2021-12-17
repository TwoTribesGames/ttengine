#if !defined(INC_TT_GWEN_BASEPANEL_H)
#define INC_TT_GWEN_BASEPANEL_H


#include <Gwen/Controls/CheckBox.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/GroupBox.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/ScrollControl.h>
#include <Gwen/Controls/Slider.h>
#include <Gwen/Controls/TextBox.h>

#include <tt/gwen/ColorButton.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>

//#include <tooltips.h>


namespace tt {
namespace gwen {


class AdaptiveSlider;
class UserInterface;


class PanelBase : public Gwen::Controls::ScrollControl
{
public:
	GWEN_CONTROL(PanelBase, Gwen::Controls::ScrollControl);
	virtual ~PanelBase() {}

	void createRoot();
	void destroyRoot();

	void setUserInterface(UserInterface* p_ui) { m_userInterface = p_ui; };

	void updateSliders(real p_elapsedTime);

protected:
	Gwen::Controls::Label*   addLabel  (const tt::math::Point2& p_position,
	                                    const tt::math::Point2& p_size,
	                                    const std::string&      p_text);
	Gwen::Controls::Label*   addLabel  (const tt::math::Point2& p_position,
	                                    const tt::math::Point2& p_size,
	                                    const std::string&      p_text,
	                                    Gwen::Controls::Base*   p_root);
	Gwen::Controls::TextBox* addTextBox(const tt::math::Point2& p_position,
	                                    const tt::math::Point2& p_size,
	                                    const std::string&      p_text);
	
	Gwen::Controls::TextBoxNumeric*    addLabeledIntBox  (const std::string&      p_text,
	                                                      const tt::math::Point2& p_labelSize,
	                                                      s32                     p_value = 0);
	Gwen::Controls::TextBox*           addLabeledTextBox (const std::string&      p_text,
	                                                      const tt::math::Point2& p_labelSize,
	                                                      const std::string&      p_value);
	Gwen::Controls::ComboBox*          addLabeledComboBox(const std::string&      p_text,
	                                                      const tt::math::Point2& p_labelSize);
	Gwen::Controls::CheckBoxWithLabel* addLabeledCheckBox(const std::string&      p_text,
	                                                      const tt::math::Point2& p_size,
	                                                      bool                    p_checkState);
	Gwen::Controls::GroupBox*          addGroupBox       (const std::string&      p_title,
	                                                      const tt::math::Point2& p_size,
	                                                      s32                     p_items,
	                                                      s32                     p_margin,
	                                                      const Gwen::Color&      p_textColor);
	ColorButton*                    addLabeledColorButton(const std::string&      p_text,
	                                                      const tt::math::Point2& p_labelSize,
	                                                      const tt::math::Point2& p_buttonSize,
	                                                      const std::string&      p_name = "");
	
	AdaptiveSlider* addLabeledSlider(const std::string&      p_text,
	                                 const tt::math::Point2& p_labelSize,
	                                 real                    p_value = 0.0f);
	
	AdaptiveSlider* addLabeledSlider(const std::string&      p_text,
	                                 const tt::math::Point2& p_labelSize,
	                                 real                    p_value,
	                                 Gwen::Controls::Base*   p_root);

	void setControlEnabled(Gwen::Controls::Base* p_control, bool p_enabled);


	typedef std::vector<AdaptiveSlider*> AdaptiveSliders;
	AdaptiveSliders m_sliders;

	Gwen::Controls::Base* m_root;
	UserInterface*        m_userInterface;
	tt::math::Point2      m_currentPosition;
	real                  m_updateTime;
	real                  m_updateFrequency;
	s32                   m_rightMargin;
};


}
}


#endif // INC_TT_GWEN_BASEPANEL_H
