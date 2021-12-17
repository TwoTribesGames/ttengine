#include <iomanip>
#include <sstream>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/RGBColorPicker.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/Menu.h>

#include <tt/code/ErrorStatus.h>
#include <tt/gwen/ColorRGBAPicker.h>
#include <tt/gwen/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>

#include <toki/game/editor/commands/CommandChangeEntityProperties.h>
#include <toki/game/editor/ui/EntityPropertyControl.h>
#include <toki/game/editor/ui/EntityPropertyList.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/entity/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( EntityPropertyList )
,
m_editor          (0),
m_entities        (),
m_items           (),
m_propertyControls(),
m_labelPosition   (0)
{
	m_observer.reset(new Observer(this));
	
	SetScroll(false, true);
	SetAutoHideBars(true);
	if (Inner() != 0)
	{
		Inner()->SetPadding(Gwen::Padding(0, 0, 4, 0));
	}
}


EntityPropertyList::~EntityPropertyList()
{
}


void EntityPropertyList::setEditor(Editor* p_editor)
{
	TT_NULL_ASSERT(p_editor);
	m_editor = p_editor;
}


Editor* EntityPropertyList::getEditor()
{
	TT_NULL_ASSERT(m_editor);
	return m_editor;
}


void EntityPropertyList::setEntities(const level::entity::EntityInstanceSet& p_entities)
{
	if (p_entities == m_entities)
	{
		return;
	}
	
	// Unregister as observer with previous entities
	for (level::entity::EntityInstanceSet::iterator it = m_entities.begin();
	     it != m_entities.end(); ++it)
	{
		(*it)->unregisterObserver(m_observer);
	}
	
	m_entities = p_entities;
	
	// Register as observer with new entities
	for (level::entity::EntityInstanceSet::iterator it = m_entities.begin();
	     it != m_entities.end(); ++it)
	{
		(*it)->registerObserver(m_observer);
	}
	
	createUi();
}


void EntityPropertyList::entityPicked(const level::entity::EntityInstancePtr& p_entity,
                                      const std::string&                      p_propertyName)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity == 0 || p_propertyName.empty() || m_entities.empty())
	{
		return;
	}
	
	EntityPropertyControl* control = findControlForProperty(p_propertyName);
	if (control == 0)
	{
		TT_PANIC("UI has no edit control for property '%s': cannot update picked entity.",
		         p_propertyName.c_str());
		return;
	}
	
	commands::CommandChangeEntityPropertiesPtr cmd =
		commands::CommandChangeEntityProperties::create(m_entities);
	
	const std::string valueStr(tt::str::toStr(p_entity->getID()));
	
	const level::entity::EntityProperty& prop(control->getProperty());
	switch (prop.getType())
	{
	case level::entity::EntityProperty::Type_Entity:
	case level::entity::EntityProperty::Type_EntityID:
	case level::entity::EntityProperty::Type_DelayedEntityID:
		cmd->setPropertyValue(p_propertyName, valueStr);
		break;
	
	case level::entity::EntityProperty::Type_EntityArray:
	case level::entity::EntityProperty::Type_EntityIDArray:
	case level::entity::EntityProperty::Type_DelayedEntityIDArray:
		// Append the entity ID to each selected entity's entity array
		for (level::entity::EntityInstanceSet::iterator it = m_entities.begin();
		     it != m_entities.end(); ++it)
		{
			std::string newValue;
			if ((*it)->hasProperty(p_propertyName))
			{
				newValue = (*it)->getPropertyValue(p_propertyName);
			}
			
			if (newValue.empty() == false) newValue += ",";
			newValue += valueStr;
			
			cmd->setPropertyValue(*it, p_propertyName, newValue);
		}
		break;
	
	default:
		TT_PANIC("Property '%s' of entity type '%s' is not an entity reference or entity array.",
		         p_propertyName.c_str(), p_entity->getType().c_str());
		break;
	}
	
	// Only add the command to the stack if any changes were actually made
	if (cmd->hasPropertyChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
	else
	{
		// Property wasn't changed, but button still needs to be un-toggled:
		// make all controls update their value so that the button will be un-toggled
		setAllControlValues();
	}
}


void EntityPropertyList::entityPickCancelled()
{
	// Make all controls update their value so that the button will be un-toggled
	setAllControlValues();
}


void EntityPropertyList::changePropertyValue(const level::entity::EntityProperty& p_property,
                                             const std::string&                   p_newValue)
{
	if (m_entities.empty())
	{
		TT_PANIC("Cannot change property '%s' if no entities are selected.",
		         p_property.getName().c_str());
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	if (m_editor == 0)
	{
		return;
	}
	
	commands::CommandChangeEntityPropertiesPtr cmd =
		commands::CommandChangeEntityProperties::create(m_entities);
	
	cmd->setPropertyValue(p_property.getName(), p_newValue);
	
	// Only add the command to the stack if any changes were actually made
	if (cmd->hasPropertyChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
	
	checkConditionals();
}


void EntityPropertyList::resetPropertyValueToDefault(const level::entity::EntityProperty& p_property)
{
	resetPropertyValueToDefault(p_property.getName());
}


void EntityPropertyList::pickEntityForProperty(const level::entity::EntityProperty& p_property)
{
	if (m_entities.empty())
	{
		TT_PANIC("Cannot change property '%s' if no entities are selected.", p_property.getName().c_str());
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	if (m_editor == 0)
	{
		return;
	}
	
	TT_ASSERT(level::entity::EntityProperty::isEntityType(p_property.getType()));
	
	level::entity::EntityIDSet disallowedIDs;
	if (p_property.getType() == level::entity::EntityProperty::Type_EntityArray ||
	    p_property.getType() == level::entity::EntityProperty::Type_EntityIDArray ||
	    p_property.getType() == level::entity::EntityProperty::Type_DelayedEntityIDArray)
	{
		// For entity arrays, do not allow the user to pick entities that are already in the array
		for (level::entity::EntityInstanceSet::iterator it = m_entities.begin();
		     it != m_entities.end(); ++it)
		{
			if ((*it)->hasProperty(p_property.getName()) == false)
			{
				continue;
			}
			
			const tt::str::Strings values(tt::str::explode(
					(*it)->getPropertyValue(p_property.getName()), ","));
			for (tt::str::Strings::const_iterator valueIt = values.begin();
			     valueIt != values.end(); ++valueIt)
			{
				disallowedIDs.insert(tt::str::parseS32(*valueIt, 0));
			}
		}
	}
	
	{
		// You should not be able to select the selecting entity
		const level::entity::EntityInstanceSet& selectedEntities(m_editor->getLevelData()->getSelectedEntities());
		TT_ASSERT(selectedEntities.size() == 1);
		for (level::entity::EntityInstanceSet::const_iterator it = selectedEntities.begin();
			 it != selectedEntities.end(); ++it)
		{
			disallowedIDs.insert((*it)->getID());
		}
	}
	
	m_editor->enterEntityPickMode(p_property, disallowedIDs);
}


void EntityPropertyList::pickColorRGBForProperty(const level::entity::EntityProperty& p_property,
                                                 const Gwen::Color&                   p_initialColor)
{
	Gwen::Controls::Menu* menu = new Gwen::Controls::Menu(GetCanvas());
	menu->SetSize(230, 180);
	menu->SetDeleteOnClose(true);
	menu->SetDisableIconMargin(true);
	
	Gwen::Controls::RGBColorPicker* picker = new Gwen::Controls::RGBColorPicker(menu);
	picker->Dock(Gwen::Pos::Fill);
	picker->SetSize(230, 140);
	
	picker->SetColor(p_initialColor, false, true);
	
	{
		Gwen::Controls::Base* dock = new Gwen::Controls::Base(menu);
		
		Gwen::Controls::Button* buttonCancel = new Gwen::Controls::Button(dock);
		buttonCancel->SetText(translateString("BUTTON_CANCEL"));
		buttonCancel->SetWidth(75);
		buttonCancel->UserData.Set("colorPicker", picker);
		buttonCancel->UserData.Set("parentMenu",  menu);
		buttonCancel->onPress.Add(this, &EntityPropertyList::onColorRGBPickerCancel);
		buttonCancel->Dock(Gwen::Pos::Right);
		
		Gwen::Controls::Button* buttonOk = new Gwen::Controls::Button(dock);
		buttonOk->SetText(translateString("BUTTON_OK"));
		buttonOk->SetWidth(75);
		buttonOk->SetMargin(Gwen::Margin(0, 0, 5, 0));
		buttonOk->UserData.Set("colorPicker", picker);
		buttonOk->UserData.Set("parentMenu",  menu);
		buttonOk->UserData.Set("propName",    p_property.getName());
		buttonOk->onPress.Add(this, &EntityPropertyList::onColorRGBPickerOK);
		buttonOk->Dock(Gwen::Pos::Right);
		
		dock->SetPadding(Gwen::Padding(0, 5, 5, 5));
		dock->SetHeight(30);
		dock->Dock(Gwen::Pos::Bottom);
	}
	
	// FIXME: Open to the left if no room remains on the right (same for the opposite situation, also top/bottom)
	menu->Open(Gwen::Pos::Right | Gwen::Pos::Top);
	//menu->Open(Gwen::Pos::Left | Gwen::Pos::Top);
}


void EntityPropertyList::pickColorRGBAForProperty(const level::entity::EntityProperty& p_property,
                                                  const Gwen::Color&                   p_initialColor)
{
	Gwen::Controls::Menu* menu = new Gwen::Controls::Menu(GetCanvas());
	menu->SetSize(256, 180);
	menu->SetDeleteOnClose(true);
	menu->SetDisableIconMargin(true);
	
	tt::gwen::ColorRGBAPicker* picker = new tt::gwen::ColorRGBAPicker(menu);
	picker->Dock(Gwen::Pos::Fill);
	picker->SetSize(256, 140);
	
	picker->SetColor(p_initialColor, false, true);
	
	{
		Gwen::Controls::Base* dock = new Gwen::Controls::Base(menu);
		
		Gwen::Controls::Button* buttonCancel = new Gwen::Controls::Button(dock);
		buttonCancel->SetText(translateString("BUTTON_CANCEL"));
		buttonCancel->SetWidth(75);
		buttonCancel->UserData.Set("colorPicker", picker);
		buttonCancel->UserData.Set("parentMenu",  menu);
		buttonCancel->onPress.Add(this, &EntityPropertyList::onColorRGBAPickerCancel);
		buttonCancel->Dock(Gwen::Pos::Right);
		
		Gwen::Controls::Button* buttonOk = new Gwen::Controls::Button(dock);
		buttonOk->SetText(translateString("BUTTON_OK"));
		buttonOk->SetWidth(75);
		buttonOk->SetMargin(Gwen::Margin(0, 0, 5, 0));
		buttonOk->UserData.Set("colorPicker", picker);
		buttonOk->UserData.Set("parentMenu",  menu);
		buttonOk->UserData.Set("propName",    p_property.getName());
		buttonOk->onPress.Add(this, &EntityPropertyList::onColorRGBAPickerOK);
		buttonOk->Dock(Gwen::Pos::Right);
		
		dock->SetPadding(Gwen::Padding(0, 5, 5, 5));
		dock->SetHeight(30);
		dock->Dock(Gwen::Pos::Bottom);
	}
	
	// FIXME: Open to the left if no room remains on the right (same for the opposite situation, also top/bottom)
	menu->Open(Gwen::Pos::Right | Gwen::Pos::Top);
	//menu->Open(Gwen::Pos::Left | Gwen::Pos::Top);
}


void EntityPropertyList::removeValueFromArray(const level::entity::EntityProperty& p_property,
                                              const std::string&                   p_valueToRemove)
{
	const std::string& propName(p_property.getName());
	if (m_entities.empty())
	{
		TT_PANIC("Cannot change property '%s' if no entities are selected.", propName.c_str());
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	if (m_editor == 0)
	{
		return;
	}
	
	if (level::entity::EntityProperty::isArrayType(p_property.getType()) == false)
	{
		TT_PANIC("Entity property '%s' is not an array type: cannot remove a value the array. Property type: %s",
		         propName.c_str(), level::entity::EntityProperty::getTypeName(p_property.getType()));
		return;
	}
	
	commands::CommandChangeEntityPropertiesPtr cmd =
		commands::CommandChangeEntityProperties::create(m_entities);
	
	for (level::entity::EntityInstanceSet::iterator instanceIt = m_entities.begin();
	     instanceIt != m_entities.end(); ++instanceIt)
	{
		if ((*instanceIt)->hasProperty(propName) == false)
		{
			continue;
		}
		
		tt::str::Strings values(tt::str::explode((*instanceIt)->getPropertyValue(propName), ","));
		
		for (tt::str::Strings::iterator valueIt = values.begin(); valueIt != values.end(); )
		{
			if (*valueIt == p_valueToRemove)
			{
				valueIt = values.erase(valueIt);
			}
			else
			{
				++valueIt;
			}
		}
		
		if (values.size() == 0)
		{
			cmd->removeProperty(*instanceIt, propName);
		}
		else
		{
			cmd->setPropertyValue(*instanceIt, propName, tt::str::implode(values, ","));
		}
	}
	
	// Only add the command to the stack if any changes were actually made
	if (cmd->hasPropertyChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
}


void EntityPropertyList::checkConditionals()
{
	if (m_entities.size() > 1)
	{
		return;
	}
	
	using namespace level::entity;
	EntityInstance::Properties defaults;
	
	// Make list of all properties and their defaults
	for (PropertyControls::const_iterator it = m_propertyControls.begin(); it != m_propertyControls.end(); ++it)
	{
		const EntityProperty& prop((*it)->getProperty());
		
		// FIXME: Currently only works for non-array types; do we need that as well?
		if (EntityProperty::isArrayType(prop.getType()) == false)
		{
			defaults.insert(std::make_pair(prop.getName(), prop.getDefault().getAsString()));
		}
	}
	
	// Now show/hide based on value
	// FIXME: Use EntityInstance::isPropertyVisible here?
	for (PropertyControls::iterator it = m_propertyControls.begin(); it != m_propertyControls.end(); ++it)
	{
		const EntityProperty& prop((*it)->getProperty());
		if (EntityProperty::isArrayType(prop.getType()) || prop.hasConditional() == false)
		{
			continue;
		}
		
		const EntityProperty::Conditional& conditional(prop.getConditional());
		
		const EntityInstancePtr& instance(*m_entities.begin());
		const std::string& targetPropertyName(conditional.getTargetPropertyName());
		
		const std::string& value = instance->hasProperty(targetPropertyName) ?
			instance->getPropertyValue(targetPropertyName) : defaults[targetPropertyName];
		
		conditional.test(value) ? (*it)->Show() : (*it)->Hide();
	}
}


void EntityPropertyList::onGroupExpanded(const Gwen::TextObject& /*p_groupName*/, bool p_exanded)
{
	if (p_exanded)
	{
		checkConditionals();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void EntityPropertyList::createUi()
{
	// Remove previous GUI elements
	destroyUi();
	
	// If no entities, no GUI to create
	if (m_entities.empty())
	{
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	
	/* FIXME: Figure the following things out:
	- How to reduce the massive amounts of code that are currently needed for all the different types?
	
	Split UI updates up into two passes?
	- First pass: determine shared properties (+ types), create controls for them.
	- Second pass: set correct values for each property edit control.
	*/
	
	if (m_entities.size() > 1)
	{
		const std::wstring multiple(translateString("ENTITY_TYPE_MULTIPLE"));
		
		// Editing multiple entities at once is not currently supported
		const std::string entityType((*m_entities.begin())->getType());
		for (level::entity::EntityInstanceSet::iterator it = m_entities.begin();
		     it != m_entities.end(); ++it)
		{
			if ((*it)->getType() != entityType)
			{
				createHeaderUI(multiple, multiple);
				return;
			}
		}
		
		const level::entity::EntityInfo* info = AppGlobal::getEntityLibrary().getEntityInfo(entityType);
		createHeaderUI(getEntityDisplayName(info, entityType), multiple);
		return;
	}
	
	// Create the interface for the single selected entity
	level::entity::EntityInstancePtr entity(*m_entities.begin());
	
	const level::entity::EntityInfo* info = AppGlobal::getEntityLibrary().getEntityInfo(entity->getType());
	
	// Entity info header
	createHeaderUI(getEntityDisplayName(info, entity->getType()), tt::str::widen(tt::str::toStr(entity->getID())));
	updatePositionLabel(entity);
	
	// All editable properties
	static const Gwen::Margin panelMargin(0, 0, 0, 8);
	tt::str::StringSet typePropNames;
	
	if (info != 0)
	{
		const level::entity::EntityProperties& typeProps(info->getProperties());
		
		// First the properties without a group
		for (level::entity::EntityProperties::const_iterator it = typeProps.begin(); it != typeProps.end(); ++it)
		{
			const level::entity::EntityProperty& prop(*it);
			if (prop.hasGroup())
			{
				continue;
			}
			typePropNames.insert(prop.getName());
			
			EntityPropertyControl* control = new EntityPropertyControl(this);
			control->create(this, prop);
			control->setValueFromEntity(entity);
			m_items.push_back(control);
			m_propertyControls.push_back(control);
			control->SetMargin(panelMargin);
			
			control->Dock(Gwen::Pos::Top);
		}
		
		// Then the properties with a group
		for (level::entity::EntityProperties::const_iterator it = typeProps.begin(); it != typeProps.end(); ++it)
		{
			const level::entity::EntityProperty& prop(*it);
			if (prop.hasGroup() == false)
			{
				continue;
			}
			typePropNames.insert(prop.getName());
			
			EntityPropertyControl* control = new EntityPropertyControl(this);
			control->create(this, prop);
			control->setValueFromEntity(entity);
			m_items.push_back(control);
			m_propertyControls.push_back(control);
			control->SetMargin(panelMargin);
			
			AddItem(tt::str::widen(prop.getGroup()), control);
		}
	}
	
	// Scan the instance properties for any properties that are not part of EntityInfo
	bool createdSeparator = false;
	const level::entity::EntityInstance::Properties& instanceProps(entity->getProperties());
	for (level::entity::EntityInstance::Properties::const_iterator it = instanceProps.begin();
	     it != instanceProps.end(); ++it)
	{
		const std::string& propName((*it).first);
		if (typePropNames.find(propName) != typePropNames.end())
		{
			// Property is part of EntityInfo: no action needed for this entry
			continue;
		}
		
		// FIXME: Store the controls created below, so that they can be updated when properties change
		
		// This property is not part of EntityInfo!
		
		Gwen::Controls::Label* label = 0;
		if (createdSeparator == false)
		{
			createdSeparator = true;
			
			label = new Gwen::Controls::Label(this);
			m_items.push_back(label);
			label->SetText(translateString("ENTITY_PROPERTY_UNRECOGNIZED"));
			label->SizeToContents();
			label->Dock(Gwen::Pos::Top);
			label->SetMargin(Gwen::Margin(0, 30, 0, 5));
		}
		
		// Property name label
		label = new Gwen::Controls::Label(this);
		m_items.push_back(label);
		label->SetText(propName + ":");
		label->SetTextColor(Gwen::Color(17, 102, 182, 255));
		label->SizeToContents();
		label->Dock(Gwen::Pos::Top);
		
		Gwen::Controls::Base* panel = new Gwen::Controls::Base(this);
		
		// Property value label (cannot edit unsupported properties)
		label = new Gwen::Controls::Label(panel);
		label->SetText((*it).second);
		label->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);
		label->Dock(Gwen::Pos::Fill);
		
		// "Reset to default" button, which erases the unsupported property
		static const int defaultsButtonSize = 15;
		Gwen::Controls::Button* defaultsButton = new Gwen::Controls::Button(panel);
		defaultsButton->UserData.Set("propName", propName);
		defaultsButton->SetText("X");
		defaultsButton->SetToolTip(translateString("ENTITY_PROPERTY_RESET_TO_DEFAULT"));
		defaultsButton->SetSize(defaultsButtonSize, defaultsButtonSize);
		defaultsButton->onPress.Add(this, &EntityPropertyList::onResetToDefault);
		defaultsButton->Dock(Gwen::Pos::Right);
		
		m_items.push_back(panel);
		panel->SetHeight(defaultsButton->Height());
		panel->Dock(Gwen::Pos::Top);
		panel->SetMargin(panelMargin);
	}
	
	checkConditionals();
}


void EntityPropertyList::destroyUi()
{
	for (Items::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		(*it)->DelayedDelete();
	}
	m_items.clear();
	m_propertyControls.clear();
	m_labelPosition = 0;
	Clear();
	
	// Force update to make sure all items are deleted
	GetCanvas()->DoThink();
}


void EntityPropertyList::setAllControlValues()
{
	// Ignore multiple entity selections
	if (m_entities.size() != 1)
	{
		return;
	}
	
	level::entity::EntityInstancePtr entity(*m_entities.begin());
	for (PropertyControls::iterator it = m_propertyControls.begin(); it != m_propertyControls.end(); ++it)
	{
		(*it)->setValueFromEntity(entity);
	}
	
	checkConditionals();
}


void EntityPropertyList::createHeaderUI(const std::wstring& p_entityType,
                                        const std::wstring& p_entityID)
{
	static const int keyLabelWidth = 50;
	
	addKeyValuePanel(translateString("ENTITY_LABEL_TYPE"), p_entityType, keyLabelWidth);
	addKeyValuePanel(translateString("ENTITY_LABEL_ID"),   p_entityID,   keyLabelWidth);
	m_labelPosition = addKeyValuePanel(translateString("ENTITY_LABEL_POSITION"),
			translateString("ENTITY_TYPE_MULTIPLE"), keyLabelWidth, Gwen::Margin(0, 0, 0, 7));
}


Gwen::Controls::Label* EntityPropertyList::addKeyValuePanel(const std::wstring& p_keyText,
                                                            const std::wstring& p_valueText,
                                                            int                 p_keyWidth,
                                                            const Gwen::Margin& p_panelMargin)
{
	using namespace Gwen::Controls;
	Base*  panel      = new Base(this);
	Label* keyLabel   = new Label(panel);
	Label* valueLabel = new Label(panel);
	
	keyLabel->SetText(p_keyText);
	keyLabel->SizeToContents();
	keyLabel->SetWidth(p_keyWidth);
	keyLabel->Dock(Gwen::Pos::Left);
	
	valueLabel->SetText(p_valueText);
	valueLabel->SizeToContents();
	valueLabel->Dock(Gwen::Pos::Fill);
	
	panel->SetHeight(std::max(keyLabel->Height(), valueLabel->Height()));
	panel->SetMargin(p_panelMargin);
	panel->Dock(Gwen::Pos::Top);
	m_items.push_back(panel);
	
	return valueLabel;
}


std::wstring EntityPropertyList::getEntityDisplayName(const level::entity::EntityInfo* p_info,
                                                      const std::string&               p_entityType)
{
	// No info available or display name is same as type: only display type
	if (p_info == 0 || p_info->getDisplayName() == p_entityType)
	{
		return tt::str::widen(p_entityType);
	}
	
	return tt::str::widen(p_entityType + " (" + p_info->getDisplayName() + ")");
}


void EntityPropertyList::onEntityInstancePositionChanged(const level::entity::EntityInstancePtr& p_instance)
{
	if (p_instance                  != 0 &&
	    m_entities.size()           == 1 &&
	    m_entities.find(p_instance) != m_entities.end())
	{
		updatePositionLabel(p_instance);
	}
}


void EntityPropertyList::onEntityInstancePropertiesChanged(const level::entity::EntityInstancePtr& p_instance)
{
	//TT_Printf("EntityPropertyList::onEntityInstancePropertiesChanged: Properties of entity 0x%08X have changed.\n",
	//          p_instance.get());
	
	if (m_entities.find(p_instance) != m_entities.end())
	{
		// FIXME: What to do for multiple selections? Ignore or recreate UI?
		setAllControlValues();
		
		// FIXME: Also update the "non-existent properties" controls here! Properties might have been added or removed.
	}
}


void EntityPropertyList::updatePositionLabel(const level::entity::EntityInstancePtr& p_instance)
{
	if (p_instance != 0 && m_labelPosition != 0)
	{
		std::ostringstream oss;
		oss << "("
		    << std::setprecision(4) << p_instance->getPosition().x
		    << ", "
		    << std::setprecision(4) << p_instance->getPosition().y
		    << ")";
		m_labelPosition->SetText(oss.str());
	}
}


void EntityPropertyList::resetPropertyValueToDefault(const std::string& p_propertyName)
{
	if (m_entities.empty())
	{
		TT_PANIC("Cannot reset property '%s' to default if no entities are selected.",
		         p_propertyName.c_str());
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	if (m_editor == 0)
	{
		return;
	}
	
	commands::CommandChangeEntityPropertiesPtr cmd =
		commands::CommandChangeEntityProperties::create(m_entities);
	
	cmd->removeProperty(p_propertyName);
	
	// Only add the command to the stack if any changes were actually made
	if (cmd->hasPropertyChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
	
	checkConditionals();
}


EntityPropertyControl* EntityPropertyList::findControlForProperty(const std::string& p_propertyName)
{
	for (PropertyControls::iterator it = m_propertyControls.begin(); it != m_propertyControls.end(); ++it)
	{
		if ((*it)->getProperty().getName() == p_propertyName)
		{
			return *it;
		}
	}
	
	return 0;
}


void EntityPropertyList::onResetToDefault(Gwen::Controls::Base* p_sender)
{
	const std::string propName(p_sender->UserData.Get<std::string>("propName"));
	if (propName.empty())
	{
		TT_PANIC("Invalid 'reset to default' button: button has no property name set.");
		return;
	}
	
	if (m_entities.empty())
	{
		TT_PANIC("Cannot reset property '%s' to default if no entities are selected.",
		         propName.c_str());
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	if (m_editor == 0)
	{
		return;
	}
	
	commands::CommandChangeEntityPropertiesPtr cmd =
		commands::CommandChangeEntityProperties::create(m_entities);
	
	cmd->removeProperty(propName);
	
	// Only add the command to the stack if any changes were actually made
	if (cmd->hasPropertyChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
}


void EntityPropertyList::onColorRGBPickerOK(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
	if (button == 0)
	{
		TT_PANIC("Sender is not a Button.");
		return;
	}
	
	Gwen::Controls::Menu*           parentMenu  = button->UserData.Get<Gwen::Controls::Menu*          >("parentMenu");
	Gwen::Controls::RGBColorPicker* colorPicker = button->UserData.Get<Gwen::Controls::RGBColorPicker*>("colorPicker");
	const std::string               propName     (button->UserData.Get<std::string                    >("propName"));
	
	if (parentMenu == 0 || colorPicker == 0)
	{
		TT_PANIC("Button does not have 'parentMenu' or 'colorPicker' user data.");
		return;
	}
	
	// Get the selected color from the picker, close the picker, change the property value via an undo command
	const tt::engine::renderer::ColorRGB selectedColor(tt::gwen::toEngineColor(colorPicker->GetColor()).rgb());
	
	parentMenu->Close();
	
	if (m_entities.empty())
	{
		TT_PANIC("Cannot change property '%s' if no entities are selected.",
		         propName.c_str());
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	if (m_editor == 0)
	{
		return;
	}
	
	TT_ASSERT(m_entities.size() == 1);
	
	const level::entity::EntityInstancePtr& instance(*m_entities.begin());
	
	const level::entity::EntityInfo* info = AppGlobal::getEntityLibrary().getEntityInfo(instance->getType());
	TT_NULL_ASSERT(info);
	
	const level::entity::EntityProperty& prop(info->getProperty(propName));
	TT_ASSERT(prop.getType() == level::entity::EntityProperty::Type_ColorRGB);
	
	commands::CommandChangeEntityPropertiesPtr cmd =
		commands::CommandChangeEntityProperties::create(m_entities);
	
	cmd->setPropertyValue(propName, level::entity::makePropertyString(selectedColor));
	
	// Only add the command to the stack if any changes were actually made
	if (cmd->hasPropertyChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
}


void EntityPropertyList::onColorRGBPickerCancel(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
	if (button == 0)
	{
		TT_PANIC("Sender is not a Button.");
		return;
	}
	
	Gwen::Controls::Menu* parentMenu = button->UserData.Get<Gwen::Controls::Menu*>("parentMenu");
	TT_ASSERTMSG(parentMenu != 0, "Button does not have 'parentMenu' user data.");
	if (parentMenu != 0)
	{
		parentMenu->Close();
	}
}


void EntityPropertyList::onColorRGBAPickerOK(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
	if (button == 0)
	{
		TT_PANIC("Sender is not a Button.");
		return;
	}
	
	Gwen::Controls::Menu*      parentMenu  = button->UserData.Get<Gwen::Controls::Menu*     >("parentMenu");
	tt::gwen::ColorRGBAPicker* colorPicker = button->UserData.Get<tt::gwen::ColorRGBAPicker*>("colorPicker");
	const std::string          propName     (button->UserData.Get<std::string               >("propName"));
	
	if (parentMenu == 0 || colorPicker == 0)
	{
		TT_PANIC("Button does not have 'parentMenu' or 'colorPicker' user data.");
		return;
	}
	
	// Get the selected color from the picker, close the picker, change the property value via an undo command
	const tt::engine::renderer::ColorRGBA selectedColor(tt::gwen::toEngineColor(colorPicker->GetColor()));
	
	parentMenu->Close();
	
	if (m_entities.empty())
	{
		TT_PANIC("Cannot change property '%s' if no entities are selected.",
		         propName.c_str());
		return;
	}
	
	TT_NULL_ASSERT(m_editor);
	if (m_editor == 0)
	{
		return;
	}
	
	TT_ASSERT(m_entities.size() == 1);
	
	const level::entity::EntityInstancePtr& instance(*m_entities.begin());
	
	const level::entity::EntityInfo* info = AppGlobal::getEntityLibrary().getEntityInfo(instance->getType());
	TT_NULL_ASSERT(info);
	
	const level::entity::EntityProperty& prop(info->getProperty(propName));
	TT_ASSERT(prop.getType() == level::entity::EntityProperty::Type_ColorRGBA);
	
	commands::CommandChangeEntityPropertiesPtr cmd =
		commands::CommandChangeEntityProperties::create(m_entities);
	
	cmd->setPropertyValue(propName, level::entity::makePropertyString(selectedColor));
	
	// Only add the command to the stack if any changes were actually made
	if (cmd->hasPropertyChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
}


void EntityPropertyList::onColorRGBAPickerCancel(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
	if (button == 0)
	{
		TT_PANIC("Sender is not a Button.");
		return;
	}
	
	Gwen::Controls::Menu* parentMenu = button->UserData.Get<Gwen::Controls::Menu*>("parentMenu");
	TT_ASSERTMSG(parentMenu != 0, "Button does not have 'parentMenu' user data.");
	if (parentMenu != 0)
	{
		parentMenu->Close();
	}
}

// Namespace end
}
}
}
}
