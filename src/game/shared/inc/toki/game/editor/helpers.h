#if !defined(INC_TOKI_GAME_EDITOR_HELPERS_H)
#define INC_TOKI_GAME_EDITOR_HELPERS_H


#include <string>

#include <json/forwards.h>

#include <tt/loc/LocStr.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>
#include <tt/str/str_types.h>
#include <tt/str/StringFormatter.h>

#include <toki/level/entity/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace editor {

extern const std::string g_clipboardEntityMarker;
extern const std::string g_clipboardTilesBeginMarker;
extern const std::string g_clipboardTilesEndMarker;
extern const std::string g_clipboardThemeTilesBeginMarker;
extern const std::string g_clipboardThemeTilesEndMarker;


// Entity to/from JSON

Json::Value createEntityInstanceJson(const level::entity::EntityInstancePtr& p_instance,
                                     const tt::math::Vector2&                p_positionOffset);
level::entity::EntityInstancePtr createEntityInstanceFromJson(const Json::Value&         p_json,
                                                              const level::LevelDataPtr& p_targetLevel,
                                                              s32*                       p_idFromData_OUT = 0);
level::entity::EntityInstances createEntitiesFromJson(const std::string&         p_json,
                                                      const level::LevelDataPtr& p_targetLevel,
                                                      tt::math::Vector2*         p_minPos_OUT,
                                                      tt::math::Vector2*         p_maxPos_OUT);


// Localization

std::wstring translateString(const std::string& p_locID);

template<typename T>
inline std::wstring translateString(const std::string& p_locID, const T& p_var1)
{
	// TODO: If loc ID does not exist or has no translation, do something smart to still provide a result...
	tt::str::StringFormatter formatter(translateString(p_locID));
	formatter << p_var1;
	return formatter.getResult();
}


template<typename T>
inline std::wstring translateString(const std::string& p_locID, const T& p_var1, const T& p_var2)
{
	// TODO: If loc ID does not exist or has no translation, do something smart to still provide a result...
	tt::str::StringFormatter formatter(translateString(p_locID));
	formatter << p_var1 << p_var2;
	return formatter.getResult();
}


// Misc

std::wstring getSkinConfigDisplayName(level::skin::SkinConfigType p_skinConfig);
std::wstring getThemeDisplayName(level::ThemeType p_theme);

std::string getLevelsSourceDir();

// Returns a string representation of the level data. Useful for level diffing.
std::string getLevelDataAsString(const level::LevelDataPtr& p_levelData);

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_HELPERS_H)
