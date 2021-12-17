#if !defined(INC_TOKI_GAME_EDITOR_UI_ENTITYPROPERTYLIST_H)
#define INC_TOKI_GAME_EDITOR_UI_ENTITYPROPERTYLIST_H


#include <Gwen/Controls/Label.h>

#include <tt/gwen/GroupedList.h>

#include <toki/game/editor/ui/fwd.h>
#include <toki/game/editor/fwd.h>
#include <toki/level/entity/EntityInstanceObserver.h>
#include <toki/level/entity/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Custom scroll control for editing entity properties. */
class EntityPropertyList : public tt::gwen::GroupedList
{
public:
	GWEN_CONTROL(EntityPropertyList, tt::gwen::GroupedList);
	virtual ~EntityPropertyList();
	
	void    setEditor(Editor* p_editor);
	Editor* getEditor();
	void setEntities(const level::entity::EntityInstanceSet& p_entities);
	
	void entityPicked(const level::entity::EntityInstancePtr& p_entity, const std::string& p_propertyName);
	void entityPickCancelled();
	
	// For EntityPropertyControl:
	void changePropertyValue        (const level::entity::EntityProperty& p_property, const std::string& p_newValue);
	void resetPropertyValueToDefault(const level::entity::EntityProperty& p_property);
	void pickEntityForProperty      (const level::entity::EntityProperty& p_property);
	void pickColorRGBForProperty    (const level::entity::EntityProperty& p_property, const Gwen::Color& p_initialColor);
	void pickColorRGBAForProperty   (const level::entity::EntityProperty& p_property, const Gwen::Color& p_initialColor);
	void removeValueFromArray       (const level::entity::EntityProperty& p_property, const std::string& p_valueToRemove);
	
	void checkConditionals();
	
	virtual void onGroupExpanded(const Gwen::TextObject& p_groupName, bool p_exanded);
	
private:
	class Observer : public level::entity::EntityInstanceObserver
	{
	public:
		explicit Observer(EntityPropertyList* p_target)
		:
		m_target(p_target)
		{ }
		virtual ~Observer() { }
		
		virtual void onEntityInstancePositionChanged(const level::entity::EntityInstancePtr& p_instance)
		{
			m_target->onEntityInstancePositionChanged(p_instance);
		}
		
		virtual void onEntityInstancePropertiesChanged(const level::entity::EntityInstancePtr& p_instance)
		{
			m_target->onEntityInstancePropertiesChanged(p_instance);
		}
		
	private:
		EntityPropertyList* m_target;
	};
	friend class EntityPropertyList::Observer;
	
	typedef std::vector<EntityPropertyControl*> PropertyControls;
	typedef std::vector<Gwen::Controls::Base* > Items;
	
	
	void createUi();
	void destroyUi();
	void setAllControlValues();
	
	void createHeaderUI(const std::wstring& p_entityType, const std::wstring& p_entityID);
	// Returns the value label
	Gwen::Controls::Label* addKeyValuePanel(const std::wstring& p_keyText,
	                                        const std::wstring& p_valueText,
	                                        int                 p_keyWidth,
	                                        const Gwen::Margin& p_panelMargin = Gwen::Margin(0, 0, 0, 0));
	
	static std::wstring getEntityDisplayName(const level::entity::EntityInfo* p_info,
	                                         const std::string&               p_entityType);
	
	void onEntityInstancePositionChanged  (const level::entity::EntityInstancePtr& p_instance);
	void onEntityInstancePropertiesChanged(const level::entity::EntityInstancePtr& p_instance);
	
	void updatePositionLabel(const level::entity::EntityInstancePtr& p_instance);
	void resetPropertyValueToDefault(const std::string& p_propertyName);
	
	EntityPropertyControl* findControlForProperty(const std::string& p_propertyName);
	
	// UI handlers
	void onResetToDefault       (Gwen::Controls::Base* p_sender);
	void onColorRGBPickerOK     (Gwen::Controls::Base* p_sender);
	void onColorRGBPickerCancel (Gwen::Controls::Base* p_sender);
	void onColorRGBAPickerOK    (Gwen::Controls::Base* p_sender);
	void onColorRGBAPickerCancel(Gwen::Controls::Base* p_sender);
	
	// No copying
	EntityPropertyList(const EntityPropertyList&);
	EntityPropertyList& operator=(const EntityPropertyList&);
	
	
	Editor*                                  m_editor;
	level::entity::EntityInstanceObserverPtr m_observer;
	level::entity::EntityInstanceSet         m_entities; //!< Entities for which this property list is populated
	Items                                    m_items;
	PropertyControls                         m_propertyControls;
	Gwen::Controls::Label*                   m_labelPosition; // Label displaying the entity position
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_ENTITYPROPERTYLIST_H)
