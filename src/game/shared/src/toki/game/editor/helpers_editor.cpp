#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
#include <Windows.h>
#include <tt/engine/renderer/DXUT/DXUT.h>
#endif
#include <json/json.h>
#include <map>

#include <tt/app/Application.h>
#include <tt/code/helpers.h>
#include <tt/fs/utils/utils.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/steam/helpers.h>
#include <tt/str/str.h>
#include <tt/system/Language.h>

#include <toki/game/editor/helpers.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/entity/helpers.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/Note.h>
#include <toki/loc/Loc.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace editor {

const std::string g_clipboardEntityMarker         ("***ENTITIES***: ");
const std::string g_clipboardTilesBeginMarker     ("***TILES_BEGIN***");
const std::string g_clipboardTilesEndMarker       ("***TILES_END***");
const std::string g_clipboardThemeTilesBeginMarker("***THEME_BEGIN***");
const std::string g_clipboardThemeTilesEndMarker  ("***THEME_END***");


//--------------------------------------------------------------------------------------------------
// Entity to/from JSON

Json::Value createEntityInstanceJson(const level::entity::EntityInstancePtr& p_instance,
                                     const tt::math::Vector2&                p_positionOffset)
{
	TT_NULL_ASSERT(p_instance);
	if (p_instance == 0)
	{
		return Json::Value();
	}
	
	Json::Value entry;
	entry["type" ] = p_instance->getType();
	entry["id"   ] = p_instance->getID();
	entry["x"    ] = p_instance->getPosition().x + p_positionOffset.x;
	entry["y"    ] = p_instance->getPosition().y + p_positionOffset.y;
	entry["props"] = Json::Value(Json::objectValue);
	
	const level::entity::EntityInstance::Properties& props(p_instance->getProperties());
	for (level::entity::EntityInstance::Properties::const_iterator it = props.begin();
	     it != props.end(); ++it)
	{
		entry["props"][(*it).first] = (*it).second;
	}
	
	return entry;
}


level::entity::EntityInstancePtr createEntityInstanceFromJson(const Json::Value&         p_json,
                                                              const level::LevelDataPtr& p_targetLevel,
                                                              s32*                       p_idFromData_OUT)
{
	if (p_json.isMember("type" ) == false || p_json["type" ].isString()                       == false ||
	    p_json.isMember("id"   ) == false || p_json["id"   ].isConvertibleTo(Json::intValue)  == false ||
	    p_json.isMember("x"    ) == false || p_json["x"    ].isConvertibleTo(Json::realValue) == false ||
	    p_json.isMember("y"    ) == false || p_json["y"    ].isConvertibleTo(Json::realValue) == false ||
	    p_json.isMember("props") == false || p_json["props"].isObject()                       == false)
	{
		return level::entity::EntityInstancePtr();
	}
	
	const std::string type(p_json["type"].asString());
	s32               id = p_json["id"].asInt();
	const real        x  = static_cast<real>(p_json["x"].asDouble());
	const real        y  = static_cast<real>(p_json["y"].asDouble());
	
	if (type.empty())
	{
		return level::entity::EntityInstancePtr();
	}
	
	if (p_idFromData_OUT != 0) *p_idFromData_OUT = id;
	
	if (p_targetLevel->hasEntity(id))
	{
		id = p_targetLevel->createEntityID();
	}
	
	level::entity::EntityInstancePtr instance(level::entity::EntityInstance::create(
		type, id, tt::math::Vector2(x, y)));
	
	const Json::Value& props(p_json["props"]);
	const Json::Value::Members propMembers(props.getMemberNames());
	for (Json::Value::Members::const_iterator propIt = propMembers.begin();
	     propIt != propMembers.end(); ++propIt)
	{
		instance->setPropertyValue(*propIt, props[*propIt].asString());
	}
	
	return instance;
}


level::entity::EntityInstances createEntitiesFromJson(const std::string&         p_json,
                                                      const level::LevelDataPtr& p_targetLevel,
                                                      tt::math::Vector2*         p_minPos_OUT,
                                                      tt::math::Vector2*         p_maxPos_OUT)
{
	using level::entity::EntityInstance;
	using level::entity::EntityInstancePtr;
	using level::entity::EntityInstances;
	
	Json::Value entityRoot;
	if (Json::Reader().parse(p_json, entityRoot, false) == false ||
	    entityRoot.isArray() == false)
	{
		return EntityInstances();
	}
	
	tt::math::Vector2 entityMinPos( std::numeric_limits<real>::max(),  std::numeric_limits<real>::max());
	tt::math::Vector2 entityMaxPos(-std::numeric_limits<real>::max(), -std::numeric_limits<real>::max());
	EntityInstances   instances;
	
	typedef std::pair<s32, s32>   OldNewID;
	typedef std::vector<OldNewID> OldNewIDs;
	OldNewIDs oldNewIDs;
	
	// Parse all entities from the JSON data
	for (Json::Value::iterator it = entityRoot.begin(); it != entityRoot.end(); ++it)
	{
		const Json::Value& entry(*it);
		
		s32 idFromData = 0;
		EntityInstancePtr instance = createEntityInstanceFromJson(entry, p_targetLevel, &idFromData);
		if (instance != 0)
		{
			const tt::math::Vector2& pos(instance->getPosition());
			if (pos.x < entityMinPos.x) entityMinPos.x = pos.x;
			if (pos.y < entityMinPos.y) entityMinPos.y = pos.y;
			if (pos.x > entityMaxPos.x) entityMaxPos.x = pos.x;
			if (pos.y > entityMaxPos.y) entityMaxPos.y = pos.y;
			
			oldNewIDs.push_back(OldNewID(idFromData, instance->getID()));
			instances.push_back(instance);
		}
	}
	
	// Fix the entity-to-entity references (so they remain valid)
	level::entity::updateInternalEntityReferences(instances, oldNewIDs, p_targetLevel);
	
	if (p_minPos_OUT != 0) *p_minPos_OUT = entityMinPos;
	if (p_maxPos_OUT != 0) *p_maxPos_OUT = entityMaxPos;
	
	return instances;
}


//--------------------------------------------------------------------------------------------------
// Localization


std::wstring translateString(const std::string& p_locID)
{
	return (AppGlobal::getLoc().hasLocStr(loc::SheetID_Editor)) ?
	        AppGlobal::getLoc().getLocStr(loc::SheetID_Editor).getString(p_locID) : tt::str::widen(p_locID);
}


//--------------------------------------------------------------------------------------------------
// Misc

std::wstring getSkinConfigDisplayName(level::skin::SkinConfigType p_skinConfig)
{
	switch (p_skinConfig)
	{
	case level::skin::SkinConfigType_Solid:  return translateString("SKINCONFIG_SOLID");
		
	default:
		TT_PANIC("Unsupported skin config type: %d. Do not have a display name for it.", p_skinConfig);
		return L"";
	}
}


std::wstring getThemeDisplayName(level::ThemeType p_theme)
{
	switch (p_theme)
	{
	case level::ThemeType_UseLevelDefault: return translateString("THEME_LEVEL_DEFAULT");
	case level::ThemeType_DoNotTheme:      return translateString("THEME_DO_NOT_THEME");
	case level::ThemeType_Sand:            return translateString("THEME_SAND");
	case level::ThemeType_Rocks:           return translateString("THEME_ROCKS");
	case level::ThemeType_Beach:           return translateString("THEME_BEACH");
	case level::ThemeType_DarkRocks:       return translateString("THEME_DARK_ROCKS");
		
	default:
		TT_PANIC("Unsupported theme type: %d. Do not have a display name for it.", p_theme);
		return L"";
	}
}


std::string getLevelsSourceDir()
{
	return tt::fs::utils::compactPath(
			tt::app::getApplication()->getAssetRootDir() + "../../source/shared/levels/",
			"\\/");
}


std::string getLevelDataAsString(const level::LevelDataPtr& p_levelData)
{
	TT_NULL_ASSERT(p_levelData);
	if (p_levelData == 0)
	{
		return std::string();
	}
	
	static const std::string newline("\n");
	static const std::string separator(80, '-');
	
	std::string result;
	
	const s32 levelWidth  = p_levelData->getLevelWidth();
	const s32 levelHeight = p_levelData->getLevelHeight();
	
	// Show some basic info
	result += separator + newline;
	result += "---- Basic Info: ---------------------------------------------------------------" + newline;
	result += newline;
	result += "Level width:        " + tt::str::toStr(levelWidth )                                   + newline;
	result += "Level height:       " + tt::str::toStr(levelHeight)                                   + newline;
	result += "Level theme:        " + level::getThemeTypeNameAsString(p_levelData->getLevelTheme()) + newline;
	result += "Background shoebox: " + p_levelData->getLevelBackground()                             + newline;
	result += "Workshop file ID:   " + tt::str::toStr(p_levelData->getPublishedFileId())             + newline;
	result += "Owner ID:           " + tt::str::toStr(p_levelData->getOwnerId())                     + newline;
	result += newline;
	result += newline;
	
	// Theme colors
	result += separator + newline;
	result += "---- Theme Colors: -------------------------------------------------------------" + newline;
	result += newline;
	for (s32 skinIndex = 0; skinIndex < level::skin::SkinConfigType_Count; ++skinIndex)
	{
		const level::skin::SkinConfigType skinConfig = static_cast<level::skin::SkinConfigType>(skinIndex);
		
		for (s32 themeIndex = 0; themeIndex < level::ThemeType_Count; ++themeIndex)
		{
			const level::ThemeType theme = static_cast<level::ThemeType>(themeIndex);
			const tt::engine::renderer::ColorRGBA& color(p_levelData->getThemeColor(skinConfig, theme));
			
			result += "Skin:  " + std::string(level::skin::getSkinConfigTypeName(skinConfig)) + newline;
			result += "Theme: " + level::getThemeTypeNameAsString(theme)                      + newline;
			result += "Color: (" + tt::str::toStr(color.r) + ", " +
			                       tt::str::toStr(color.g) + ", " +
			                       tt::str::toStr(color.b) + ", " +
			                       tt::str::toStr(color.a) + ")" + newline;
			result += newline;
		}
	}
	result += newline;
	
	// Create a plain-text copyable representation of the selected tiles
	level::AttributeLayerPtr attribs(p_levelData->getAttributeLayer());
	
	tt::math::Point2 pos;
	
	// CollisionType of each tile
	result += separator + newline;
	result += "---- Collision Tiles: ----------------------------------------------------------" + newline;
	result += newline;
	for (pos.y = levelHeight - 1; pos.y >= 0; --pos.y)
	{
		for (pos.x = 0; pos.x < levelWidth; ++pos.x)
		{
			const level::CollisionType type = attribs->getCollisionType(pos);
			result += level::getCollisionTypeAsChar(type);
		}
		
		result += newline;
	}
	result += newline;
	result += newline;
	
	// ThemeType of each tile
	result += separator + newline;
	result += "---- Theme Tiles: --------------------------------------------------------------" + newline;
	result += newline;
	for (pos.y = levelHeight - 1; pos.y >= 0; --pos.y)
	{
		for (pos.x = 0; pos.x < levelWidth; ++pos.x)
		{
			const level::ThemeType type = attribs->getThemeType(pos);
			result += level::getThemeTypeAsChar(type);
		}
		
		result += newline;
	}
	result += newline;
	result += newline;
	
	
	// Create JSON for all the entities in the level
	// NOTE: The entities are sorted by ID, to keep diffs as in sync as possible
	typedef std::map<s32, level::entity::EntityInstancePtr> EntitiesByID;
	EntitiesByID entitiesByID;
	
	{
		const level::entity::EntityInstances& allEntities(p_levelData->getAllEntities());
		for (level::entity::EntityInstances::const_iterator it = allEntities.begin();
		     it != allEntities.end(); ++it)
		{
			entitiesByID[(*it)->getID()] = *it;
		}
	}
	
	if (entitiesByID.empty() == false)
	{
		Json::Value entityRoot;
		for (EntitiesByID::const_iterator it = entitiesByID.begin(); it != entitiesByID.end(); ++it)
		{
			entityRoot.append(createEntityInstanceJson((*it).second, tt::math::Vector2::zero));
		}
		
		result += separator + newline;
		result += "---- Entities: -----------------------------------------------------------------" + newline;
		result += newline;
		result += Json::StyledWriter().write(entityRoot) + newline;
		result += newline;
		result += newline;
	}
	
	// Show all the notes
	result += separator + newline;
	result += "---- Notes: --------------------------------------------------------------------" + newline;
	result += newline;
	const level::Notes& notes(p_levelData->getAllNotes());
	for (level::Notes::const_iterator it = notes.begin(); it != notes.end(); ++it)
	{
		const level::NotePtr&       note(*it);
		const tt::math::VectorRect& rect(note->getWorldRect());
		
		result += "Text:     "  + tt::str::utf16ToUtf8(note->getText()) + newline;
		result += "Position: (" + tt::str::toStr(rect.getPosition().x) + ", "  + tt::str::toStr(rect.getPosition().y) + ")" + newline;
		result += "Size:     "  + tt::str::toStr(rect.getWidth())      + " x " + tt::str::toStr(rect.getHeight())           + newline;
		result += newline;
	}
	result += newline;
	
	return result;
}

// Namespace end
}
}
}
