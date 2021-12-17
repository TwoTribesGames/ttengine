#include <Gwen/Controls/Layout/Position.h>
#include <Gwen/Controls/CheckBox.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/HorizontalSlider.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Controls/TextBox.h>
#include <Gwen/ToolTip.h>

#include <tt/code/ErrorStatus.h>
#include <tt/gwen/ColorButton.h>
#include <tt/gwen/EqualSizes.h>
#include <tt/gwen/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/game/editor/ui/EntityPropertyControl.h>
#include <toki/game/editor/ui/EntityPropertyList.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/entity/helpers.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

static const std::string g_defaultIsNullText("<select>");


// Helper for easily adding items to a combo box
template<typename T>
inline Gwen::Controls::MenuItem* addItemsToComboBox(Gwen::Controls::ComboBox* p_combo,
                                                    const std::vector<T>&     p_values,
                                                    const std::string&        p_valueToReturn)
{
	Gwen::Controls::MenuItem* itemToReturn = 0;
	
	for (typename std::vector<T>::const_iterator valIt = p_values.begin();
	     valIt != p_values.end(); ++valIt)
	{
		const std::string valStr(tt::str::toStr(*valIt));
		Gwen::Controls::MenuItem* item = p_combo->AddItem(valStr);
		if (valStr == p_valueToReturn)
		{
			itemToReturn = item;
		}
	}
	
	return itemToReturn;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( EntityPropertyControl )
,
m_parentList          (0),
m_property            (),
m_nameLabel           (0),
m_editControl         (0),
m_resetToDefaultButton(0),
m_userTypedInTextBox  (false),
m_ignoreChangeEvents  (false),
m_sliderUndoStackSize (0)
{
}


EntityPropertyControl::~EntityPropertyControl()
{
}


void EntityPropertyControl::create(EntityPropertyList*                  p_parentList,
                                   const level::entity::EntityProperty& p_property)
{
	TT_NULL_ASSERT(p_parentList);
	TT_ASSERTMSG(m_nameLabel == 0 && m_editControl == 0 && m_resetToDefaultButton == 0,
	             "This EntityPropertyControl has already been initialized for an entity property.");
	
	m_parentList = p_parentList;
	m_property   = p_property;
	m_userTypedInTextBox = false;
	
	int totalHeight = 0;
	
	// Property name label (except for Boolean properties)
	if (p_property.hasChoice() ||
	    p_property.getType() != level::entity::EntityProperty::Type_Bool)
	{
		m_nameLabel = new Gwen::Controls::Label(this);
		m_nameLabel->SetText(p_property.getName() + ":");
		if (p_property.getDescription().empty() == false)
		{
			m_nameLabel->SetToolTip(p_property.getDescription());
		}
		m_nameLabel->SizeToContents();
		m_nameLabel->Dock(Gwen::Pos::Top);
		totalHeight += m_nameLabel->Height();
	}
	
	// Property edit control
	Gwen::Controls::Base* parent = new Gwen::Controls::Base(this);
	
	if (p_property.hasChoice())
	{
		// Combo box
		m_editControl = createComboBox(parent, p_property);
	}
	else
	{
		// Some other control
		switch (p_property.getType())
		{
		case level::entity::EntityProperty::Type_Integer:
			m_editControl = createPropIntegerControl(parent, p_property);
			break;
			
		case level::entity::EntityProperty::Type_Float:
			m_editControl = createPropFloatControl(parent, p_property);
			break;
			
		case level::entity::EntityProperty::Type_Bool:
			m_editControl = createPropBoolControl(parent, p_property);
			break;
			
		case level::entity::EntityProperty::Type_String:
			m_editControl = createPropStringControl(parent, p_property);
			break;
			
			/*
		case level::entity::EntityProperty::Type_IntegerArray:
			// FIXME: How to edit this?
			break;
			
		case level::entity::EntityProperty::Type_FloatArray:
			// FIXME: How to edit this?
			break;
			
		case level::entity::EntityProperty::Type_BoolArray:
			// FIXME: How to edit this?
			break;
			
		case level::entity::EntityProperty::Type_StringArray:
			// FIXME: How to edit this?
			break;
			// */
			
		case level::entity::EntityProperty::Type_Entity:
		case level::entity::EntityProperty::Type_EntityID:
		case level::entity::EntityProperty::Type_DelayedEntityID:
			m_editControl = createPropEntityControl(parent, p_property);
			break;
			
		case level::entity::EntityProperty::Type_EntityArray:
		case level::entity::EntityProperty::Type_EntityIDArray:
		case level::entity::EntityProperty::Type_DelayedEntityIDArray:
			m_editControl = createPropEntityArrayControl(parent, p_property);
			break;
			
		case level::entity::EntityProperty::Type_ColorRGB:
			m_editControl = createPropColorRGBControl(parent, p_property);
			break;
			
		case level::entity::EntityProperty::Type_ColorRGBA:
			m_editControl = createPropColorRGBAControl(parent, p_property);
			break;
			
		default:
			break;
		}
	}
	
	if (m_editControl == 0)
	{
		// Create a placeholder control to indicate this can't be edited yet
		Gwen::Controls::Label* label = new Gwen::Controls::Label(parent);
		m_editControl = label;
		label->SetText(translateString("ENTITY_PROPERTY_CANNOT_EDIT"));
		label->SizeToContents();
		label->Dock(Gwen::Pos::Fill);
	}
	
	TT_NULL_ASSERT(m_editControl);
	if (m_editControl != 0)
	{
		if (p_property.getDescription().empty() == false)
		{
			m_editControl->SetToolTip(p_property.getDescription());
		}
		
		static const int defaultsButtonSize = 15;
		
		Gwen::Controls::Layout::Center* defaultsButtonPanel = new Gwen::Controls::Layout::Center(parent);
		
		m_resetToDefaultButton = new Gwen::Controls::Button(defaultsButtonPanel);
		m_resetToDefaultButton->SetText("X");
		m_resetToDefaultButton->SetSize(defaultsButtonSize, defaultsButtonSize);
		m_resetToDefaultButton->onPress.Add(this, &EntityPropertyControl::onResetToDefault);
		m_resetToDefaultButton->SetDisabled(true);
		
		defaultsButtonPanel->SetWidth(defaultsButtonSize);
		defaultsButtonPanel->SetMargin(Gwen::Margin(4, 0, 0, 0));
		defaultsButtonPanel->Dock(Gwen::Pos::Right);
		
		parent->SetHeight(m_editControl->Height());
		
		totalHeight += m_editControl->Height();
	}
	
	parent->Dock(Gwen::Pos::Top);
	
	SetHeight(totalHeight);
}


void EntityPropertyControl::setValueFromEntity(const level::entity::EntityInstancePtr& p_entity)
{
	TT_NULL_ASSERT(p_entity);
	TT_NULL_ASSERT(m_parentList);
	
	m_ignoreChangeEvents = true;
	
	const bool valueIsOverridden = p_entity->hasProperty(m_property.getName());
	const std::string propertyValue(valueIsOverridden ? p_entity->getPropertyValue(m_property.getName()) : std::string());
	
	TT_NULL_ASSERT(m_resetToDefaultButton);  // every property edit control should have a "reset to default" button
	m_resetToDefaultButton->SetDisabled(valueIsOverridden == false);
	if (valueIsOverridden)
	{
		m_resetToDefaultButton->SetToolTip(translateString("ENTITY_PROPERTY_RESET_TO_DEFAULT"));
	}
	else if (m_resetToDefaultButton->GetToolTip() != 0)
	{
		ToolTip::Disable(m_resetToDefaultButton);
		m_resetToDefaultButton->GetToolTip()->DelayedDelete();
		m_resetToDefaultButton->SetToolTip(0);
	}
	
	if (m_nameLabel != 0)
	{
		m_nameLabel->SetTextColorOverride(valueIsOverridden ?
				Gwen::Color(17, 102, 182, 255) : Gwen::Color(255, 255, 255, 0));
	}
	
	using namespace Gwen::Controls;
	if (m_property.hasChoice())
	{
		// Edit control is a ComboBox
		ComboBox* combo = gwen_cast<ComboBox>(m_editControl);
		TT_NULL_ASSERT(combo);
		if (combo != 0)
		{
			// ComboBox steals focus when adding new items, so work around that
			Gwen::Controls::Base* prevKeyboardFocus = Gwen::KeyboardFocus;
			
			combo->CloseList();
			combo->ClearItems();
			
			Gwen::Controls::MenuItem* itemToSelect = 0;
			std::string               valueToSelect;
			if (valueIsOverridden)
			{
				valueToSelect = p_entity->getPropertyValue(m_property.getName());
			}
			else
			{
				if (m_property.getDefault().isNull())
				{
					// Add extra "select" item if default is null and is not overridden
					// (user must make a selection from the list; there is no default)
					itemToSelect = combo->AddItem(g_defaultIsNullText);
				}
				else
				{
					valueToSelect = m_property.getDefault().getAsString();
				}
			}
			
			Gwen::Controls::MenuItem* itemFromValues = 0;
			switch (m_property.getType())
			{
			case level::entity::EntityProperty::Type_Integer:
				itemFromValues = addItemsToComboBox(combo, m_property.getChoice().getIntegerArray(), valueToSelect);
				break;
				
			case level::entity::EntityProperty::Type_Float:
				itemFromValues = addItemsToComboBox(combo, m_property.getChoice().getFloatArray(), valueToSelect);
				break;
				
			case level::entity::EntityProperty::Type_Bool:
				itemFromValues = addItemsToComboBox(combo, m_property.getChoice().getBoolArray(), valueToSelect);
				break;
				
			case level::entity::EntityProperty::Type_String:
				itemFromValues = addItemsToComboBox(combo, m_property.getChoice().getStringArray(), valueToSelect);
				break;
				
			default:
				// Unsupported
				// FIXME: Report this somehow!
				break;
			}
			
			if (itemFromValues != 0)
			{
				// An item was added that matches the value that needs to be selected:
				// override whatever item may have been set as default selection
				itemToSelect = itemFromValues;
			}
			
			// FIXME: What happened if there is no item to select? Does that mean the value is invalid, missing, what?
			if (itemToSelect != 0)
			{
				combo->SelectItem(itemToSelect, false);
			}
			
			// Restore the original keyboard focus (ComboBox may have stolen focus)
			if (prevKeyboardFocus != 0)
			{
				prevKeyboardFocus->Focus();
			}
			else
			{
				Gwen::Input::Blur();
			}
		}
	}
	else
	{
		switch (m_property.getType())
		{
		case level::entity::EntityProperty::Type_Integer:
			{
				// Edit control is a HorizontalSlider
				HorizontalSlider* slider = gwen_cast<HorizontalSlider>(m_editControl);
				TT_NULL_ASSERT(slider);
				if (slider != 0)
				{
					s32 sliderValue = m_property.getDefault().getInteger();
					if (valueIsOverridden)
					{
						TT_ERR_CREATE("");
						const s32 parsedValue = tt::str::parseS32(propertyValue, &errStatus);
						if (errStatus.hasError())
						{
							// FIXME: Instead of panicking, mark the invalid value in the GUI somehow?
							TT_PANIC("Value '%s' of property '%s' is not a valid integer.",
							         propertyValue.c_str(), m_property.getName().c_str());
						}
						else
						{
							// FIXME: Add range check (and somehow indicate invalid value)
							sliderValue = parsedValue;
						}
					}
					
					m_ignoreChangeEvents = sliderValue <= slider->GetMax() && sliderValue >= slider->GetMin();
					slider->SetFloatValue(static_cast<float>(sliderValue));
				}
			}
			break;
			
		case level::entity::EntityProperty::Type_Float:
			{
				// Edit control is a HorizontalSlider
				HorizontalSlider* slider = gwen_cast<HorizontalSlider>(m_editControl);
				TT_NULL_ASSERT(slider);
				if (slider != 0)
				{
					real sliderValue = m_property.getDefault().getFloat();
					if (valueIsOverridden)
					{
						TT_ERR_CREATE("");
						const real parsedValue = tt::str::parseReal(propertyValue, &errStatus);
						if (errStatus.hasError())
						{
							// FIXME: Instead of panicking, mark the invalid value in the GUI somehow?
							TT_PANIC("Value '%s' of property '%s' is not a valid float.",
							         propertyValue.c_str(), m_property.getName().c_str());
						}
						else
						{
							// FIXME: Add range check (and somehow indicate invalid value)
							sliderValue = parsedValue;
						}
					}
					
					m_ignoreChangeEvents = sliderValue <= slider->GetMax() && sliderValue >= slider->GetMin();
					slider->SetFloatValue(sliderValue);
				}
			}
			break;
			
		case level::entity::EntityProperty::Type_Bool:
			{
				// Edit control is a CheckBoxWithLabel
				CheckBoxWithLabel* checkBox = gwen_cast<CheckBoxWithLabel>(m_editControl);
				TT_NULL_ASSERT(checkBox);
				if (checkBox != 0)
				{
					bool checkBoxValue = m_property.getDefault().getBool();
					if (valueIsOverridden)
					{
						TT_ERR_CREATE("");
						const bool parsedValue = tt::str::parseBool(propertyValue, &errStatus);
						if (errStatus.hasError())
						{
							// FIXME: Instead of panicking, mark the invalid value in the GUI somehow?
							TT_PANIC("Value '%s' of property '%s' is not a valid Boolean.",
							         propertyValue.c_str(), m_property.getName().c_str());
						}
						else
						{
							checkBoxValue = parsedValue;
						}
					}
					
					checkBox->Checkbox()->SetChecked(checkBoxValue);
				}
			}
			break;
			
		case level::entity::EntityProperty::Type_String:
			{
				// Edit control is a TextBox
				TextBox* textBox = gwen_cast<TextBox>(m_editControl);
				TT_NULL_ASSERT(textBox);
				if (textBox != 0)
				{
					textBox->SetText(valueIsOverridden ? propertyValue : m_property.getDefault().getString(), false);
				}
			}
			break;
			
			/*
		case level::entity::EntityProperty::Type_IntegerArray:
			// FIXME: How to edit this?
			break;
			
		case level::entity::EntityProperty::Type_FloatArray:
			// FIXME: How to edit this?
			break;
			
		case level::entity::EntityProperty::Type_BoolArray:
			// FIXME: How to edit this?
			break;
			
		case level::entity::EntityProperty::Type_StringArray:
			// FIXME: How to edit this?
			break;
			// */
			
		case level::entity::EntityProperty::Type_Entity:
		case level::entity::EntityProperty::Type_EntityID:
		case level::entity::EntityProperty::Type_DelayedEntityID:
			{
				// Edit control is a Button
				Button* button = gwen_cast<Button>(m_editControl);
				TT_NULL_ASSERT(button);
				if (button != 0)
				{
					std::wstring buttonText;
					Gwen::Color textColor(Gwen::Color(255, 255, 255, 0));
					
					if (valueIsOverridden)
					{
						// Parse the value as an s32 (entity ID)
						TT_ERR_CREATE("");
						const s32 entityID = tt::str::parseS32(propertyValue, &errStatus);
						if (errStatus.hasError())
						{
							TT_WARN("Value '%s' of property '%s' is not a valid entity ID.",
							        propertyValue.c_str(), m_property.getName().c_str());
							
							buttonText = translateString("ENTITY_PROPERTY_INVALID", propertyValue);
							textColor  = Gwen::Colors::Red;
						}
						else
						{
							// Look up the entity with this ID
							level::entity::EntityInstancePtr targetEntity =
									m_parentList->getEditor()->getLevelData()->getEntityByID(entityID);
							if (targetEntity == 0)
							{
								buttonText = translateString("ENTITY_PROPERTY_REFERENCE_PICK");
								// Not required anymore to make a distinction between no entity and an invalid entity
								//buttonText = translateString("ENTITY_PROPERTY_REFERENCE_INVALID", propertyValue);
								//textColor  = Gwen::Colors::Red;
							}
							else
							{
								buttonText = translateString("ENTITY_PROPERTY_REFERENCE",
										targetEntity->getType(), propertyValue);
							}
						}
					}
					else
					{
						buttonText = translateString("ENTITY_PROPERTY_REFERENCE_PICK");
					}
					
					button->SetText(buttonText);
					button->SetTextColorOverride(textColor);
					button->SetToggleState(false);
				}
			}
			break;
			
		case level::entity::EntityProperty::Type_EntityArray:
		case level::entity::EntityProperty::Type_EntityIDArray:
		case level::entity::EntityProperty::Type_DelayedEntityIDArray:
			{
				// Edit control is a ListBox
				ListBox* listBox = gwen_cast<ListBox>(m_editControl);
				TT_NULL_ASSERT(listBox);
				if (listBox != 0)
				{
					const std::string selectedName(listBox->GetSelectedRowName());
					
					listBox->Clear();
					
					if (valueIsOverridden)
					{
						const tt::str::Strings ids(tt::str::explode(propertyValue, ","));
						for (tt::str::Strings::const_iterator it = ids.begin(); it != ids.end(); ++it)
						{
							std::wstring itemText;
							Gwen::Color textColor(Gwen::Color(255, 255, 255, 0));
							
							// Parse the value as an s32 (entity ID)
							TT_ERR_CREATE("");
							const s32 entityID = tt::str::parseS32(*it, &errStatus);
							if (errStatus.hasError())
							{
								TT_WARN("Value '%s' of property '%s' is not a valid entity ID.",
								        (*it).c_str(), m_property.getName().c_str());
								
								itemText  = translateString("ENTITY_PROPERTY_INVALID", *it);
								textColor = Gwen::Colors::Red;
							}
							else
							{
								// Look up the entity with this ID
								level::entity::EntityInstancePtr targetEntity =
										m_parentList->getEditor()->getLevelData()->getEntityByID(entityID);
								if (targetEntity == 0)
								{
									// Skip missing entity references (will not be saved anyway)
									continue;
									
									//itemText  = translateString("ENTITY_PROPERTY_REFERENCE_INVALID", *it);
									//textColor = Gwen::Colors::Red;
								}
								else
								{
									itemText = translateString("ENTITY_PROPERTY_REFERENCE",
											targetEntity->getType(), *it);
								}
							}
							
							Layout::TableRow* item = listBox->AddItem(itemText, *it);
							if (textColor.a != 0)
							{
								item->SetTextColor(textColor);
							}
							if (*it == selectedName)
							{
								listBox->SetSelectedRow(item);
							}
						}
					}
					
					Button* buttonAdd    = listBox->UserData.Get<Button*>("buttonAdd");
					Button* buttonRemove = listBox->UserData.Get<Button*>("buttonRemove");
					TT_ASSERTMSG(buttonAdd    != 0, "Internal error: entity array list box does not have a reference to the Add button.");
					TT_ASSERTMSG(buttonRemove != 0, "Internal error: entity array list box does not have a reference to the Remove button.");
					
					if (buttonAdd != 0)
					{
						buttonAdd->SetToggleState(false);
					}
					if (buttonRemove != 0)
					{
						buttonRemove->SetDisabled(listBox->GetSelectedRow() == 0);
					}
				}
			}
			break;
			
		case level::entity::EntityProperty::Type_ColorRGB:
			{
				// Edit control is a ColorButton
				tt::gwen::ColorButton* colorButton = gwen_cast<tt::gwen::ColorButton>(m_editControl);
				TT_NULL_ASSERT(colorButton);
				if (colorButton != 0)
				{
					Gwen::Color buttonColor(Gwen::Colors::GreyLight);
					
					if (valueIsOverridden)
					{
						TT_ERR_CREATE("");
						const tt::engine::renderer::ColorRGB color(
								level::entity::parseColorRGBProperty(propertyValue, &errStatus));
						if (errStatus.hasError())
						{
							// FIXME: Instead of panicking, mark the invalid value in the GUI somehow?
							TT_PANIC("Value '%s' of property '%s' is not a valid RGB color.\nReason:\n%s",
							         propertyValue.c_str(), m_property.getName().c_str(),
							         errStatus.getErrorMessage().c_str());
						}
						else
						{
							buttonColor = tt::gwen::toGwenColor(color);
						}
						colorButton->SetText("");
					}
					else
					{
						colorButton->SetText(g_defaultIsNullText);
					}
					
					colorButton->SetColor(buttonColor);
				}
			}
			break;
			
		case level::entity::EntityProperty::Type_ColorRGBA:
			{
				// Edit control is a ColorButton
				tt::gwen::ColorButton* colorButton = gwen_cast<tt::gwen::ColorButton>(m_editControl);
				TT_NULL_ASSERT(colorButton);
				if (colorButton != 0)
				{
					Gwen::Color buttonColor(Gwen::Colors::GreyLight);
					
					if (valueIsOverridden)
					{
						TT_ERR_CREATE("");
						const tt::engine::renderer::ColorRGBA color(
								level::entity::parseColorRGBAProperty(propertyValue, &errStatus));
						if (errStatus.hasError())
						{
							// FIXME: Instead of panicking, mark the invalid value in the GUI somehow?
							TT_PANIC("Value '%s' of property '%s' is not a valid RGBA color.\nReason:\n%s",
							         propertyValue.c_str(), m_property.getName().c_str(),
							         errStatus.getErrorMessage().c_str());
						}
						else
						{
							buttonColor = tt::gwen::toGwenColor(color);
						}
						colorButton->SetText("");
					}
					else
					{
						colorButton->SetText(g_defaultIsNullText);
					}
					
					colorButton->SetColor(buttonColor);
				}
			}
			break;
			
		default:
			break;
		}
	}
	
	m_userTypedInTextBox = false;
	m_ignoreChangeEvents = false;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Gwen::Controls::Base* EntityPropertyControl::createComboBox(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& /*p_property*/)
{
	Gwen::Controls::ComboBox* combo = new Gwen::Controls::ComboBox(p_parent);
	combo->onSelection.Add(this, &EntityPropertyControl::onSelectChoiceValue);
	combo->SetHeight(18);
	combo->SetMargin(Gwen::Margin(0, 0, 0, 0));
	combo->Dock(Gwen::Pos::Fill);
	return combo;
}


Gwen::Controls::Base* EntityPropertyControl::createPropIntegerControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& p_property)
{
	TT_ASSERT(p_property.hasMin() && p_property.hasMax());
	if (p_property.hasMin() == false || p_property.hasMax() == false)
	{
		return 0;
	}
	
	Gwen::Controls::Base* panel = new Gwen::Controls::Base(p_parent);
	
	Gwen::Controls::HorizontalSlider* slider = new Gwen::Controls::HorizontalSlider(panel);
	slider->Dock(Gwen::Pos::Fill);
	slider->SetHeight(20);
	slider->SetRange(static_cast<float>(p_property.getMin().getInteger()),
	                 static_cast<float>(p_property.getMax().getInteger()));
	slider->SetClampToNotches(false);
	//slider->SetNotchCount(p_property.getMax().getInteger() - p_property.getMin().getInteger());
	slider->SetFloatValue(static_cast<float>(p_property.getDefault().getInteger()));
	
	Gwen::Controls::TextBoxNumeric* valueLabel = new Gwen::Controls::TextBoxNumeric(panel);
	valueLabel->SetIntegerMode(true);
	valueLabel->SetText(p_property.getDefault().getAsString());
	valueLabel->SetWidth(40);  // FIXME: What is a good width?
	valueLabel->SetMargin(Gwen::Margin(2, 0, 0, 0));
	valueLabel->Dock(Gwen::Pos::Right);
	
	slider->UserData.Set("valueLabel", valueLabel);
	slider->onValueChanged       .Add(this, &EntityPropertyControl::onSliderIntegerValueChanged);
	slider->onValueChangeStart   .Add(this, &EntityPropertyControl::onSliderChangeStart);
	slider->onValueChangeFinished.Add(this, &EntityPropertyControl::onSliderIntegerValueChangeFinished);
	
	valueLabel->UserData.Set("slider", slider);
	valueLabel->onReturnPressed    .Add(this, &EntityPropertyControl::onSliderIntegerTextChangeComplete);
	valueLabel->onLostKeyboardFocus.Add(this, &EntityPropertyControl::onSliderIntegerTextChangeComplete);
	valueLabel->onTextChanged      .Add(this, &EntityPropertyControl::onTextBoxTyped);
	
	panel->SetHeight(slider->Height());
	panel->Dock(Gwen::Pos::Fill);
	
	return slider;
}


Gwen::Controls::Base* EntityPropertyControl::createPropFloatControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& p_property)
{
	TT_ASSERT(p_property.hasMin() && p_property.hasMax());
	if (p_property.hasMin() == false || p_property.hasMax() == false)
	{
		return 0;
	}
	
	Gwen::Controls::Base* panel = new Gwen::Controls::Base(p_parent);
	
	Gwen::Controls::HorizontalSlider* slider = new Gwen::Controls::HorizontalSlider(panel);
	slider->Dock(Gwen::Pos::Fill);
	slider->SetHeight(20);
	slider->SetRange(p_property.getMin().getFloat(), p_property.getMax().getFloat());
	slider->SetFloatValue(p_property.getDefault().getFloat());
	
	Gwen::Controls::TextBoxNumeric* valueLabel = new Gwen::Controls::TextBoxNumeric(panel);
	valueLabel->SetIntegerMode(false);
	valueLabel->SetText(p_property.getDefault().getAsString());
	valueLabel->SetWidth(40);  // FIXME: What is a good width?
	valueLabel->SetMargin(Gwen::Margin(2, 0, 0, 0));
	valueLabel->Dock(Gwen::Pos::Right);
	
	slider->UserData.Set("valueLabel", valueLabel);
	slider->onValueChanged       .Add(this, &EntityPropertyControl::onSliderFloatValueChanged);
	slider->onValueChangeStart   .Add(this, &EntityPropertyControl::onSliderChangeStart);
	slider->onValueChangeFinished.Add(this, &EntityPropertyControl::onSliderFloatValueChangeFinished);
	
	valueLabel->UserData.Set("slider", slider);
	valueLabel->onReturnPressed    .Add(this, &EntityPropertyControl::onSliderFloatTextChangeComplete);
	valueLabel->onLostKeyboardFocus.Add(this, &EntityPropertyControl::onSliderFloatTextChangeComplete);
	valueLabel->onTextChanged      .Add(this, &EntityPropertyControl::onTextBoxTyped);
	
	panel->SetHeight(slider->Height());
	panel->Dock(Gwen::Pos::Fill);
	
	return slider;
}


Gwen::Controls::Base* EntityPropertyControl::createPropBoolControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& p_property)
{
	Gwen::Controls::CheckBoxWithLabel* checkBox = new Gwen::Controls::CheckBoxWithLabel(p_parent);
	checkBox->Label()->SetText(p_property.getName());
	checkBox->Checkbox()->SetChecked(p_property.getDefault().getBool());
	checkBox->Checkbox()->onCheckChanged.Add(this, &EntityPropertyControl::onCheckBoxChanged);
	checkBox->Dock(Gwen::Pos::Fill);
	return checkBox;
}


Gwen::Controls::Base* EntityPropertyControl::createPropStringControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& p_property)
{
	Gwen::Controls::TextBox* textBox = new Gwen::Controls::TextBox(p_parent);
	textBox->SetText(p_property.getDefault().getString());
	textBox->SetHeight(20);
	textBox->Dock(Gwen::Pos::Fill);
	textBox->onReturnPressed    .Add(this, &EntityPropertyControl::onTextChangeComplete);
	textBox->onLostKeyboardFocus.Add(this, &EntityPropertyControl::onTextChangeComplete);
	textBox->onTextChanged      .Add(this, &EntityPropertyControl::onTextBoxTyped);
	return textBox;
}


Gwen::Controls::Base* EntityPropertyControl::createPropEntityControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& /*p_property*/)
{
	Gwen::Controls::Button* button = new Gwen::Controls::Button(p_parent);
	button->SetText(translateString("ENTITY_PROPERTY_REFERENCE_PICK"));
	button->SetIsToggle(true);
	button->onToggleOn.Add(this, &EntityPropertyControl::onPickEntity);
	button->Dock(Gwen::Pos::Fill);
	return button;
}


Gwen::Controls::Base* EntityPropertyControl::createPropEntityArrayControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& /*p_property*/)
{
	using namespace Gwen::Controls;
	
	Base* panel = new Base(p_parent);
	panel->Dock(Gwen::Pos::Fill);
	
	ListBox* listBox = new ListBox(panel);
	listBox->SetHeight(150);
	listBox->Dock(Gwen::Pos::Fill);
	
	tt::gwen::EqualSizes* buttonPanel = new tt::gwen::EqualSizes(panel);
	buttonPanel->SetMargin(listBox->GetMargin());
	buttonPanel->SetHorizontal(true);
	buttonPanel->SetInnerSpacing(7);
	buttonPanel->SetHeight(20);
	buttonPanel->Dock(Gwen::Pos::Bottom);
	
	Button* buttonAdd = new Button(buttonPanel);
	buttonAdd->SetText("Add...");  // FIXME: Add to loc sheet
	buttonAdd->SetIsToggle(true);
	buttonAdd->UserData.Set("list", listBox);
	buttonAdd->onToggleOn.Add(this, &EntityPropertyControl::onPickEntity);
	
	Button* buttonRemove = new Button(buttonPanel);
	buttonRemove->SetText("Remove");  // FIXME: Add to loc sheet
	buttonRemove->SetDisabled(true);
	buttonRemove->UserData.Set("list", listBox);
	buttonRemove->onPress.Add(this, &EntityPropertyControl::onRemoveSelectedEntityFromArray);
	
	listBox->UserData.Set("buttonAdd",    buttonAdd);
	listBox->UserData.Set("buttonRemove", buttonRemove);
	listBox->onRowSelected.Add(this, &EntityPropertyControl::onSelectedEntityInArrayChanged);
	
	return listBox;
}


Gwen::Controls::Base* EntityPropertyControl::createPropColorRGBControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& /*p_property*/)
{
	tt::gwen::ColorButton* button = new tt::gwen::ColorButton(p_parent);
	button->SetColor(Gwen::Color(255, 255, 255, 0));
	button->SetIsToggle(false);
	button->onPress.Add(this, &EntityPropertyControl::onPickColorRGB);
	button->Dock(Gwen::Pos::Fill);
	return button;
}


Gwen::Controls::Base* EntityPropertyControl::createPropColorRGBAControl(
		Gwen::Controls::Base*                p_parent,
		const level::entity::EntityProperty& /*p_property*/)
{
	tt::gwen::ColorButton* button = new tt::gwen::ColorButton(p_parent);
	button->SetColor(Gwen::Color(255, 255, 255, 255));
	button->SetIsToggle(false);
	button->onPress.Add(this, &EntityPropertyControl::onPickColorRGBA);
	button->Dock(Gwen::Pos::Fill);
	return button;
}


void EntityPropertyControl::changePropertyValue(const std::string& p_newValue)
{
	TT_NULL_ASSERT(m_parentList);
	if (m_parentList != 0)
	{
		m_parentList->changePropertyValue(m_property, p_newValue);
	}
}


void EntityPropertyControl::onResetToDefault()
{
	TT_NULL_ASSERT(m_parentList);
	if (m_parentList != 0)
	{
		m_parentList->resetPropertyValueToDefault(m_property);
	}
}


void EntityPropertyControl::onSelectChoiceValue(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::ComboBox* combo = gwen_cast<Gwen::Controls::ComboBox>(p_sender);
	TT_ASSERTMSG(combo != 0, "onSelectChoiceValue was called by something other than a ComboBox.");
	if (combo != 0 && m_ignoreChangeEvents == false)
	{
		changePropertyValue(combo->GetText().Get());
	}
}


void EntityPropertyControl::onSliderChangeStart(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::HorizontalSlider* slider = gwen_cast<Gwen::Controls::HorizontalSlider>(p_sender);
	if (slider == 0)
	{
		TT_PANIC("Sender is not a HorizontalSlider.");
		return;
	}
	
	if (m_ignoreChangeEvents == false)
	{
		m_sliderUndoStackSize = m_parentList->getEditor()->getUndoStackSize();
	}
}


void EntityPropertyControl::onSliderIntegerValueChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::HorizontalSlider* slider = gwen_cast<Gwen::Controls::HorizontalSlider>(p_sender);
	if (slider == 0)
	{
		TT_PANIC("Sender is not a HorizontalSlider.");
		return;
	}
	
	Gwen::Controls::TextBox* valueLabel = slider->UserData.Get<Gwen::Controls::TextBox*>("valueLabel");
	TT_ASSERTMSG(valueLabel != 0, "Slider does not have a value label associated with it.");
	const std::string value(tt::str::toStr(slider->GetIntegerValue()));
	if (valueLabel != 0)
	{
		valueLabel->SetText(value);
	}
	
	if (m_ignoreChangeEvents == false)
	{
		changePropertyValue(value);
	}
}


void EntityPropertyControl::onSliderIntegerValueChangeFinished(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::HorizontalSlider* slider = gwen_cast<Gwen::Controls::HorizontalSlider>(p_sender);
	TT_ASSERTMSG(slider != 0, "Sender is not a HorizontalSlider.");
	if (slider != 0 && m_ignoreChangeEvents == false)
	{
		// Save value before undoing
		const s32 value = slider->GetIntegerValue();
		const s32 diff  = (m_parentList->getEditor()->getUndoStackSize() - m_sliderUndoStackSize);
		m_parentList->getEditor()->undo(diff);
		changePropertyValue(tt::str::toStr(value));
	}
}


void EntityPropertyControl::onSliderIntegerTextChangeComplete(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::TextBoxNumeric* textBox = gwen_cast<Gwen::Controls::TextBoxNumeric>(p_sender);
	TT_NULL_ASSERT(textBox);
	if (textBox == 0)
	{
		return;
	}
	
	Gwen::Controls::HorizontalSlider* slider = textBox->UserData.Get<Gwen::Controls::HorizontalSlider*>("slider");
	TT_ASSERTMSG(slider != 0, "Text box does not have a slider associated with it.");
	if (slider == 0)
	{
		return;
	}
	
	if (m_userTypedInTextBox == false)
	{
		return;
	}
	
	if (textBox->GetText().Get().empty())
	{
		// Text box was left empty: restore the original slider value
		textBox->SetText(tt::str::toStr(slider->GetIntegerValue()), false);
	}
	else
	{
		// Got a new value for the entity property
		changePropertyValue(textBox->GetText().Get());
	}
}


void EntityPropertyControl::onSliderFloatValueChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::HorizontalSlider* slider = gwen_cast<Gwen::Controls::HorizontalSlider>(p_sender);
	if (slider == 0)
	{
		TT_PANIC("Sender is not a HorizontalSlider.");
		return;
	}
	
	Gwen::Controls::TextBox* valueLabel = slider->UserData.Get<Gwen::Controls::TextBox*>("valueLabel");
	TT_ASSERTMSG(valueLabel != 0, "Slider does not have a value label associated with it.");
	const std::string value(tt::str::toStr(slider->GetFloatValue()));
	if (valueLabel != 0)
	{
		valueLabel->SetText(value);
	}
	
	if (m_ignoreChangeEvents == false)
	{
		changePropertyValue(value);
	}
}


void EntityPropertyControl::onSliderFloatValueChangeFinished(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::HorizontalSlider* slider = gwen_cast<Gwen::Controls::HorizontalSlider>(p_sender);
	TT_ASSERTMSG(slider != 0, "Sender is not a HorizontalSlider.");
	if (slider != 0 && m_ignoreChangeEvents == false)
	{
		// Save value before undoing
		const real value = slider->GetFloatValue();
		const s32 diff = (m_parentList->getEditor()->getUndoStackSize() - m_sliderUndoStackSize);
		m_parentList->getEditor()->undo(diff);
		changePropertyValue(tt::str::toStr(value));
	}
}


void EntityPropertyControl::onSliderFloatTextChangeComplete(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::TextBoxNumeric* textBox = gwen_cast<Gwen::Controls::TextBoxNumeric>(p_sender);
	TT_NULL_ASSERT(textBox);
	if (textBox == 0)
	{
		return;
	}
	
	Gwen::Controls::HorizontalSlider* slider = textBox->UserData.Get<Gwen::Controls::HorizontalSlider*>("slider");
	TT_ASSERTMSG(slider != 0, "Text box does not have a slider associated with it.");
	if (slider == 0)
	{
		return;
	}
	
	if (m_userTypedInTextBox == false)
	{
		return;
	}
	
	if (textBox->GetText().Get().empty())
	{
		// Text box was left empty: restore the original slider value
		textBox->SetText(tt::str::toStr(slider->GetFloatValue()), false);
	}
	else
	{
		// Got a new value for the entity property
		changePropertyValue(textBox->GetText().Get());
	}
}


void EntityPropertyControl::onCheckBoxChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::CheckBox* checkBox = gwen_cast<Gwen::Controls::CheckBox>(p_sender);
	TT_ASSERTMSG(checkBox != 0, "Sender is not a CheckBox.");
	if (checkBox != 0 && m_ignoreChangeEvents == false)
	{
		changePropertyValue(tt::str::toStr(checkBox->IsChecked()));
	}
}


void EntityPropertyControl::onTextChangeComplete(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::TextBox* textBox = gwen_cast<Gwen::Controls::TextBox>(p_sender);
	TT_ASSERTMSG(textBox != 0, "Sender is not a TextBox.");
	if (textBox != 0 && m_ignoreChangeEvents == false)
	{
		if (m_userTypedInTextBox == false)
		{
			return;
		}
		
		std::string enteredText(textBox->GetText().Get());
		
		// Forcibly strip newlines from the text (this can end up in single line text boxes when copy&pasting)
		tt::str::replace(enteredText, "\r", "");
		tt::str::replace(enteredText, "\n", "");
		
		changePropertyValue(enteredText);
	}
}


void EntityPropertyControl::onPickEntity()
{
	TT_NULL_ASSERT(m_parentList);
	if (m_parentList != 0)
	{
		m_parentList->pickEntityForProperty(m_property);
	}
}


void EntityPropertyControl::onRemoveSelectedEntityFromArray(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::ListBox* list = p_sender->UserData.Get<Gwen::Controls::ListBox*>("list");
	TT_ASSERTMSG(list != 0, "Sender does not have a ListBox as user data.");
	if (list != 0 && list->GetSelectedRow() != 0)
	{
		m_parentList->removeValueFromArray(m_property, list->GetSelectedRow()->GetName());
	}
}


void EntityPropertyControl::onSelectedEntityInArrayChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::ListBox* list = gwen_cast<Gwen::Controls::ListBox>(p_sender);
	if (list == 0)
	{
		TT_PANIC("Sender is not a ListBox.");
		return;
	}
	
	Gwen::Controls::Button* buttonRemove = list->UserData.Get<Gwen::Controls::Button*>("buttonRemove");
	TT_ASSERTMSG(buttonRemove != 0, "Entity array list box does not have a reference to the 'Remove' button.");
	if (buttonRemove != 0)
	{
		buttonRemove->SetDisabled(list->GetSelectedRow() == 0);
	}
}


void EntityPropertyControl::onPickColorRGB(Gwen::Controls::Base* p_sender)
{
	tt::gwen::ColorButton* colorButton = gwen_cast<tt::gwen::ColorButton>(p_sender);
	if (colorButton == 0)
	{
		TT_PANIC("Sender is not a ColorButton.");
		return;
	}
	
	TT_NULL_ASSERT(m_parentList);
	if (m_parentList != 0)
	{
		m_parentList->pickColorRGBForProperty(m_property, colorButton->GetColor());
	}
}


void EntityPropertyControl::onPickColorRGBA(Gwen::Controls::Base* p_sender)
{
	tt::gwen::ColorButton* colorButton = gwen_cast<tt::gwen::ColorButton>(p_sender);
	if (colorButton == 0)
	{
		TT_PANIC("Sender is not a ColorButton.");
		return;
	}
	
	TT_NULL_ASSERT(m_parentList);
	if (m_parentList != 0)
	{
		m_parentList->pickColorRGBAForProperty(m_property, colorButton->GetColor());
	}
}


void EntityPropertyControl::onTextBoxTyped()
{
	m_userTypedInTextBox = true;
}

// Namespace end
}
}
}
}
