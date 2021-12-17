#include <json/json.h>

#include <tt/code/ErrorStatus.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/fs/File.h>
#include <tt/input/KeyBindings.h>
#include <tt/input/KeyList.h>
#if defined(TT_PLATFORM_WIN)
#include <tt/input/KeyboardController.h>
#endif
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/common.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>

#include <algorithm>

namespace tt {
namespace input {

ControllerBindings KeyBindings::ms_permanentBindings;
ControllerBindings KeyBindings::ms_customBindings;

//--------------------------------------------------------------------------------------------------
// Helpers functions

KeyCode getKeyCode(const std::string& p_keyCode)
{
	// Character code
	if (p_keyCode.length() == 1)   return str::toUpper(p_keyCode).at(0);
	
	if (p_keyCode == "VK_TAB")      return Key_Tab;
	if (p_keyCode == "VK_BACK")     return Key_Backspace;
	if (p_keyCode == "VK_CLEAR")    return Key_Clear;
	if (p_keyCode == "VK_RETURN")   return Key_Enter;
	if (p_keyCode == "VK_SHIFT")    return Key_Shift;
	if (p_keyCode == "VK_CONTROL")  return Key_Control;
	if (p_keyCode == "VK_MENU")     return Key_Alt;
	if (p_keyCode == "VK_PAUSE")    return Key_Break;
	if (p_keyCode == "VK_CAPITAL")  return Key_CapsLock;
	//if (p_keyCode == "VK_KANA")     return VK_KANA;
	//if (p_keyCode == "VK_HANGEUL")  return VK_HANGEUL;
	//if (p_keyCode == "VK_HANGUL")   return VK_HANGUL;
	//if (p_keyCode == "VK_JUNJA")    return VK_JUNJA;
	//if (p_keyCode == "VK_FINAL")    return VK_FINAL;
	//if (p_keyCode == "VK_HANJA")    return VK_HANJA;
	//if (p_keyCode == "VK_KANJI")    return VK_KANJI;
	if (p_keyCode == "VK_ESCAPE")   return Key_Escape;
	//if (p_keyCode == "VK_CONVERT")  return VK_CONVERT;
	//if (p_keyCode == "VK_NONCONVERT") return VK_NONCONVERT;
	//if (p_keyCode == "VK_ACCEPT")   return VK_ACCEPT;
	//if (p_keyCode == "VK_MODECHANGE") return VK_MODECHANGE;
	if (p_keyCode == "VK_SPACE")    return Key_Space;
	if (p_keyCode == "VK_PRIOR")   return Key_PageUp;
	if (p_keyCode == "VK_NEXT")    return Key_PageDown;
	if (p_keyCode == "VK_END")     return Key_End;
	if (p_keyCode == "VK_HOME")    return Key_Home;
	if (p_keyCode == "VK_LEFT")    return Key_Left;
	if (p_keyCode == "VK_UP")      return Key_Up;
	if (p_keyCode == "VK_RIGHT")   return Key_Right;
	if (p_keyCode == "VK_DOWN")    return Key_Down;
	//if (p_keyCode == "VK_SELECT")  return VK_SELECT;
	//if (p_keyCode == "VK_PRINT")   return VK_PRINT;
	//if (p_keyCode == "VK_EXECUTE") return VK_EXECUTE;
	//if (p_keyCode == "VK_SNAPSHOT") return VK_SNAPSHOT;
	if (p_keyCode == "VK_INSERT")  return Key_Insert;
	if (p_keyCode == "VK_DELETE")  return Key_Delete;
	//if (p_keyCode == "VK_HELP")    return VK_HELP;
	if (p_keyCode == "VK_OEM_1")   return Key_Semicolon;         // ';:' for US
	if (p_keyCode == "VK_OEM_PLUS")   return Key_Plus;   // '+' any country
	if (p_keyCode == "VK_OEM_COMMA")  return Key_Comma;  // ',' any country
	if (p_keyCode == "VK_OEM_MINUS")  return Key_Minus;  // '-' any country
	if (p_keyCode == "VK_OEM_PERIOD") return Key_Period; // '.' any country
	if (p_keyCode == "VK_OEM_2")   return Key_Slash;    // '/?' for US
	if (p_keyCode == "VK_OEM_3")   return Key_Grave;    // '`~' for US
	if (p_keyCode == "VK_OEM_4")   return Key_LeftSquareBracket;    //  '[{' for US
	if (p_keyCode == "VK_OEM_5")   return Key_Backslash;    //  '\|' for US
	if (p_keyCode == "VK_OEM_6")   return Key_RightSquareBracket;    //  ']}' for US
	if (p_keyCode == "VK_OEM_7")   return Key_Apostrophe;    //  ''"' for US
	return 0;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

bool KeyBindings::createFromXML(const fs::FilePtr& p_file)
{
	// Legacy support only; if possible use createFromJSON instead
	if (p_file == 0)
	{
		return false;
	}
	
	// Load keybindings file
	xml::XmlDocument doc(p_file);
	xml::XmlNode* root = doc.getRootNode();
	
	// Validate the root
	if(root->getName() != "keybindings")
	{
		TT_PANIC("Invalid keybinding file: %s", p_file->getPath());
		return false;
	}
	
	// Required actions aren't supported, so just assume there are none
	tt::input::RequiredActions requiredActions;
	
	// Parse all actions
	for(xml::XmlNode* actionNode = root->getFirstChild("action"); actionNode != 0;
		actionNode = actionNode->getSibling())
	{
		TT_ASSERT(actionNode->getName() == "action");
		
		// Get name of this action
		std::string action = actionNode->getAttribute("name");
		
		// Get all keys for this action
		for(xml::XmlNode* keyNode = actionNode->getFirstChild("key"); keyNode != 0;
			keyNode = keyNode->getSibling())
		{
			std::string controllerID = keyNode->getAttribute("controller");
			std::string keyCode      = keyNode->getAttribute("code");
			
			const KeyCode code = getKeyCode(keyCode);
			addActionToCustomBindings(controllerID, action, code, requiredActions);
		}
	}
	
	return true;
}


bool KeyBindings::createFromJSON(const fs::FilePtr& p_file, const RequiredActions& p_requiredActions)
{
	if (p_file == 0)
	{
		return false;
	}
	
	RequiredActions requiredActions(p_requiredActions);
	
	tt::code::BufferPtr fileContent = p_file->getContent();
	if (fileContent == 0)
	{
		return false;
	}
	
	const char* jsonDataBegin = reinterpret_cast<const char*>(fileContent->getData());
	const char* jsonDataEnd   = jsonDataBegin + fileContent->getSize();
	
	Json::Value  rootNode;
	Json::Reader reader;
	if (reader.parse(jsonDataBegin, jsonDataEnd, rootNode, false) == false)
	{
		return false;
	}
	
	const Json::Value::Members& controllers(rootNode.getMemberNames());
	for (Json::Value::Members::const_iterator controllerIt = controllers.begin();
	     controllerIt != controllers.end(); ++controllerIt)
	{
		if (rootNode[*controllerIt].isObject() == false)
		{
			TT_PANIC("Member '%s' should be an object.", controllerIt->c_str());
			return false;
		}
		
		const std::string controllerID(*controllerIt);
		
		const Json::Value& controller(rootNode[*controllerIt]);
		const Json::Value::Members actions(controller.getMemberNames());
		for (Json::Value::Members::const_iterator actionIt = actions.begin();
		     actionIt != actions.end(); ++actionIt)
		{
			if (controller[*actionIt].isConvertibleTo(Json::uintValue) == false)
			{
				TT_PANIC("Member '%s' is not an unsigned int", actionIt->c_str());
				return false;
			}
			
			const std::string action(*actionIt);
			const KeyCode code(static_cast<u8>(controller[*actionIt].asUInt()));
			
			addActionToCustomBindings(controllerID, action, code, requiredActions);
		}
	}
	
	TT_ASSERTMSG(requiredActions.empty(), "Required action(s) missing from bindings file. Defaulting to default bindings.");
	
	return requiredActions.empty();
}


bool KeyBindings::saveAsJSON(const fs::FilePtr& p_file)
{
	if (p_file == 0)
	{
		return false;
	}
	
	Json::Value rootNode;
	for (ControllerBindings::const_iterator it = ms_customBindings.begin(); it != ms_customBindings.end(); ++it)
	{
		const ActionMap& actions(it->second);
		if (actions.empty())
		{
			continue;
		}
		Json::Value& controller = rootNode[it->first];
		for (ActionMap::const_iterator actionIt = it->second.begin(); actionIt != it->second.end(); ++actionIt)
		{
			// Only store first key
			if (actionIt->second.size() == 1)
			{
				controller[actionIt->first] = static_cast<Json::UInt>(*actionIt->second.begin());
			}
			else
			{
				TT_PANIC("Json only allows for one keybinding per action. Action '%s' has %d.",
				         actionIt->first.c_str(), actionIt->second.size());;
			}
		}
	}
	
	// Write the settings data as nicely formatted JSON
	const std::string jsonText = Json::StyledWriter().write(rootNode);
	const tt::fs::size_type bytesToWrite = static_cast<tt::fs::size_type>(jsonText.length());
	
	return (p_file->write(jsonText.c_str(), bytesToWrite) == bytesToWrite);
}


ActionMap KeyBindings::getPermanentControllerBindings(const std::string& p_controllerID)
{
	ActionMap bindings;
	
	ControllerBindings::const_iterator it = ms_permanentBindings.find(p_controllerID);
	if (it != ms_permanentBindings.end())
	{
		bindings.insert((*it).second.begin(), (*it).second.end());
	}
	return bindings;
}


ActionMap KeyBindings::getCustomControllerBindings(const std::string& p_controllerID)
{
	ActionMap bindings;
	
	ControllerBindings::const_iterator it = ms_customBindings.find(p_controllerID);
	if (it != ms_customBindings.end())
	{
		bindings.insert((*it).second.begin(), (*it).second.end());
	}
	return bindings;
}


ActionMap KeyBindings::getCombinedControllerBindings(const std::string& p_controllerID)
{
	ActionMap bindings;
	
	// Permanent bindings first
	ControllerBindings::const_iterator it = ms_permanentBindings.find(p_controllerID);
	if (it != ms_permanentBindings.end())
	{
		bindings.insert((*it).second.begin(), (*it).second.end());
	}
	
	it = ms_customBindings.find(p_controllerID);
	if (it != ms_customBindings.end())
	{
		for (ActionMap::const_iterator actionIt = (*it).second.begin(); actionIt != (*it).second.end(); ++actionIt)
		{
			ActionMap::iterator bindingsIt = bindings.find(actionIt->first);
			if (bindingsIt != bindings.end())
			{
				// Copy individual keys (if they don't already exist)
				KeyList& srcList = bindingsIt->second;
				const KeyList& dstList = actionIt->second;
				for (KeyList::const_iterator keyIt = dstList.begin(); keyIt != dstList.end(); ++keyIt)
				{
					if (std::find(srcList.begin(), srcList.end(), *keyIt) == srcList.end())
					{
						srcList.push_back(*keyIt);
					}
					else
					{
						TT_WARN("Already have key %d for action '%s'", *keyIt, actionIt->first.c_str());
					}
				}
			}
			else
			{
				// Non existent; simply copy
				bindings[actionIt->first] = actionIt->second;
			}
		}
	}
	return bindings;
}


void KeyBindings::setPermanentControllerBindings(const std::string& p_controllerID, const ActionMap& p_bindings)
{
	ControllerBindings::iterator it = ms_permanentBindings.find(p_controllerID);
	if (it != ms_permanentBindings.end())
	{
		(*it).second = p_bindings;
	}
	else
	{
		ms_permanentBindings.insert(std::make_pair(p_controllerID, p_bindings));
	}
}


void KeyBindings::setCustomControllerBindings(const std::string& p_controllerID, const ActionMap& p_bindings)
{
	ControllerBindings::iterator it = ms_customBindings.find(p_controllerID);
	if (it != ms_customBindings.end())
	{
		(*it).second = p_bindings;
	}
	else
	{
		ms_customBindings.insert(std::make_pair(p_controllerID, p_bindings));
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void KeyBindings::addActionToCustomBindings(const std::string& p_controllerID, const std::string& p_action, KeyCode p_code,
                                            RequiredActions& p_requiredActions)
{
	if (ms_customBindings.find(p_controllerID) != ms_customBindings.end())
	{
		// Controller already present -> search for current action
		if (ms_customBindings[p_controllerID].find(p_action) != ms_customBindings[p_controllerID].end())
		{
			KeyList& list = ms_customBindings[p_controllerID][p_action];
			if (std::find(list.begin(), list.end(), p_code) == list.end())
			{
				// Action already present -> add key to list
				ms_customBindings[p_controllerID][p_action].push_back(p_code);
			}
			else
			{
				TT_PANIC("Already have code %d", p_code);
			}
		}
		else
		{
			// New action, create keylist and insert
			KeyList keys;
			keys.push_back(p_code);
			
			ms_customBindings[p_controllerID].insert(ActionMap::value_type(p_action, keys));
		}
	}
	else
	{
		// No such controller yet, insert controller and action
		KeyList keys;
		keys.push_back(p_code);
		
		ActionMap actionMap;
		actionMap.insert(ActionMap::value_type(p_action, keys));
		
		ms_customBindings.insert(ControllerBindings::value_type(p_controllerID, actionMap));
	}
	
	// If found in required actions; remove it
	RequiredActions::iterator it = std::find(p_requiredActions.begin(), p_requiredActions.end(), p_action);
	if (it != p_requiredActions.end())
	{
		p_requiredActions.erase(it);
	}
}

// Namespace end
}
}
