#if !defined(INC_TOKI_GAME_EDITOR_UI_ENTITYPROPERTYCONTROL_H)
#define INC_TOKI_GAME_EDITOR_UI_ENTITYPROPERTYCONTROL_H


#include <string>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/Label.h>

#include <toki/game/editor/ui/fwd.h>
#include <toki/level/entity/EntityProperty.h>
#include <toki/level/entity/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Control that allows editing of an entity property. */
class EntityPropertyControl : public Gwen::Controls::Base
{
public:
	GWEN_CONTROL(EntityPropertyControl, Gwen::Controls::Base);
	virtual ~EntityPropertyControl();
	
	/*! \brief Sets up the control for a property that this control should edit. */
	void create(EntityPropertyList* p_parentList, const level::entity::EntityProperty& p_property);
	
	/*! \brief Sets the value of the edit control based on the entity's property. */
	void setValueFromEntity(const level::entity::EntityInstancePtr& p_entity);
	
	inline const level::entity::EntityProperty& getProperty() const { return m_property; }
	
private:
	Gwen::Controls::Base* createComboBox(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropIntegerControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropFloatControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropBoolControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropStringControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropEntityControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropEntityArrayControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropColorRGBControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	Gwen::Controls::Base* createPropColorRGBAControl(
			Gwen::Controls::Base*                p_parent,
			const level::entity::EntityProperty& p_property);
	
	void changePropertyValue(const std::string& p_newValue);
	
	// UI handlers
	void onResetToDefault                  ();
	void onSelectChoiceValue               (Gwen::Controls::Base* p_sender);
	void onSliderChangeStart               (Gwen::Controls::Base* p_sender);
	void onSliderIntegerValueChanged       (Gwen::Controls::Base* p_sender);
	void onSliderIntegerValueChangeFinished(Gwen::Controls::Base* p_sender);
	void onSliderIntegerTextChangeComplete (Gwen::Controls::Base* p_sender);
	void onSliderFloatValueChanged         (Gwen::Controls::Base* p_sender);
	void onSliderFloatValueChangeFinished  (Gwen::Controls::Base* p_sender);
	void onSliderFloatTextChangeComplete   (Gwen::Controls::Base* p_sender);
	void onCheckBoxChanged                 (Gwen::Controls::Base* p_sender);
	void onTextChangeComplete              (Gwen::Controls::Base* p_sender);
	void onPickEntity                      ();
	void onRemoveSelectedEntityFromArray   (Gwen::Controls::Base* p_sender);
	void onSelectedEntityInArrayChanged    (Gwen::Controls::Base* p_sender);
	void onPickColorRGB                    (Gwen::Controls::Base* p_sender);
	void onPickColorRGBA                   (Gwen::Controls::Base* p_sender);
	void onTextBoxTyped();
	
	// Disable copy and assignment
	EntityPropertyControl(const EntityPropertyControl&);
	EntityPropertyControl& operator=(const EntityPropertyControl&);
	
	
	EntityPropertyList*           m_parentList;
	level::entity::EntityProperty m_property;
	Gwen::Controls::Label*        m_nameLabel;
	Gwen::Controls::Base*         m_editControl;
	Gwen::Controls::Button*       m_resetToDefaultButton;
	bool                          m_userTypedInTextBox;
	
	bool m_ignoreChangeEvents; // to ignore control change events when we're the one making the change
	s32  m_sliderUndoStackSize; // Used to restore the undo stack when finished sliding
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_ENTITYPROPERTYCONTROL_H)
