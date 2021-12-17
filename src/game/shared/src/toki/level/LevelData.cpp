#include <algorithm>

#include <json/json.h>

#include <tt/code/AutoGrowBuffer.h>
#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/FourCC.h>
#include <tt/compression/compression.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/math/hash/CRC32.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/format.h>
#include <tt/str/str.h>

#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/LevelDataObserver.h>
#include <toki/level/Note.h>
#include <toki/level/types.h>

// Used for the agent radii
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace level {

// NOTE: For the reason for these specific signature bytes, see
// http://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header
// (these bytes are inspired by the PNG file header bytes,
//  for the same reasons as listed there: mainly to detect file transmission problems)
const size_t g_levelFormatSignatureLength = 9;
const u8     g_levelFormatSignatureBinary[g_levelFormatSignatureLength] =
{
	0x89,        // Has the high bit set to detect transmission systems that do not support 8 bit data
	'T', 'O', 'K', 'I', // The actual signature bytes for a binary level
	0x0D, 0x0A,  // A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
	0x1A,        // A byte that stops display of the file under DOS when the command type has been used—the end-of-file character
	0x0A         // A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
};

const u8     g_levelFormatSignatureText[g_levelFormatSignatureLength] =
{
	'T', 'O', 'K', 'I', 'L', 'E', 'V', 'E', 'L'
};
const u32    g_levelFormatCurrentVersion = 1;

static const u32 g_levelFormatChunkMarker = tt::code::FourCC<'C', 'H', 'N', 'K'>::value;

// The available chunk IDs:
static const u32 g_chunkIDGlobalInfo  = tt::code::FourCC<'i', 'n', 'f', 'o'>::value;
static const u32 g_chunkIDTiles       = tt::code::FourCC<'t', 'i', 'l', 'e'>::value;
static const u32 g_chunkIDEntities    = tt::code::FourCC<'e', 'n', 't', 't'>::value;
static const u32 g_chunkIDNotes       = tt::code::FourCC<'n', 'o', 't', 'e'>::value;
static const u32 g_chunkIDPathfinding = tt::code::FourCC<'p', 'a', 't', 'h'>::value;
static const u32 g_chunkIDSteamInfo   = tt::code::FourCC<'s', 't', 'e', 'm'>::value;

static const u32 g_chunkVersionGlobalInfo  = 6;
static const u32 g_chunkVersionTiles       = 1;
static const u32 g_chunkVersionEntities    = 1;
static const u32 g_chunkVersionNotes       = 1;
static const u32 g_chunkVersionPathfinding = 1;
static const u32 g_chunkVersionSteamInfo   = 1;


//--------------------------------------------------------------------------------------------------
// Helper functions

const std::string g_dataBeginMarker       = "*** DATA_START ***\n";
const std::string g_dataEndMarker         = "*** DATA_END ***\n";
const std::string g_tilesBeginMarker      = "*** TILES_BEGIN ***\n";
const std::string g_tilesEndMarker        = "*** TILES_END ***\n";
const std::string g_themeTilesBeginMarker = "*** THEME_BEGIN ***\n";
const std::string g_themeTilesEndMarker   = "*** THEME_END ***\n";

Json::Value createEntityInstanceJson(const level::entity::EntityInstancePtr& p_instance)
{
	TT_NULL_ASSERT(p_instance);
	if (p_instance == 0)
	{
		return Json::Value();
	}
	
	Json::Value entry;
	entry["type" ] = p_instance->getType();
	entry["id"   ] = p_instance->getID();
	entry["x"    ] = p_instance->getPosition().x;
	entry["y"    ] = p_instance->getPosition().y;
	entry["props"] = Json::Value(Json::objectValue);
	
	const level::entity::EntityInstance::Properties& props(p_instance->getProperties());
	for (level::entity::EntityInstance::Properties::const_iterator it = props.begin();
	     it != props.end(); ++it)
	{
		entry["props"][(*it).first] = (*it).second;
	}
	
	return entry;
}


entity::EntityInstancePtr createEntityInstanceFromJson(const Json::Value& p_json)
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
	
	entity::EntityInstancePtr instance(level::entity::EntityInstance::create(
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


std::string getSectionBetweenMarkers(const std::string& p_beginMarker, const std::string& p_endMarker,
                                     const std::string& p_levelData)
{
	std::string::size_type start = p_levelData.find(p_beginMarker);
	if (start == std::string::npos) return std::string();
	start += p_beginMarker.size();
	
	const std::string::size_type end = p_levelData.find(p_endMarker);
	if (end == std::string::npos) return std::string();
	
	return p_levelData.substr(start, end - start);
}

//--------------------------------------------------------------------------------------------------
// Public member functions

LevelData::~LevelData()
{
}


LevelDataPtr LevelData::create(s32 p_width, s32 p_height)
{
	TT_ASSERTMSG(p_width  > 0, "Invalid level width specified: %d. Must be at least 1.",  p_width);
	TT_ASSERTMSG(p_height > 0, "Invalid level height specified: %d. Must be at least 1.", p_height);
	if (p_width <= 0 || p_height <= 0)
	{
		return LevelDataPtr();
	}
	
	LevelDataPtr level(new LevelData);
	
	level->m_width  = p_width;
	level->m_height = p_height;
	level->m_startPos.setValues(p_width / 2, p_height / 2);
	level->m_attributeLayer = AttributeLayer::create(p_width, p_height);
	TT_NULL_ASSERT(level->m_attributeLayer);
	if (level->m_attributeLayer != 0)
	{
		level->m_attributeLayer->clear();
	}
	
	return level;
}


LevelDataPtr LevelData::loadLevel(const std::string& p_filename, tt::fs::identifier p_fsID)
{
	if (tt::fs::fileExists(p_filename, p_fsID) == false)
	{
		TT_PANIC("Level file '%s' does not exist.", p_filename.c_str());
		return LevelDataPtr();
	}
	
	tt::fs::FilePtr file = tt::fs::open(p_filename, tt::fs::OpenMode_Read, p_fsID);
	if (file == 0)
	{
		TT_PANIC("Opening file '%s' for reading failed.", p_filename.c_str());
		return LevelDataPtr();
	}
	
	// Set filename
	const std::string filename = tt::fs::utils::getFileTitle(p_filename);
	
	return loadLevel(file, filename);
}


LevelDataPtr LevelData::loadLevel(const tt::fs::FilePtr& p_file, const std::string& p_filename,
                                  bool p_fileContainsOtherData)
{
	LevelDataPtr level(new LevelData);
	if (level->loadFromFile(p_file, p_filename, p_fileContainsOtherData) == false)
	{
		return LevelDataPtr();
	}
	return level;
}


bool LevelData::saveAsText(const std::string& p_filename, tt::fs::identifier p_fsID) const
{
	tt::fs::FilePtr file = tt::fs::open(p_filename, tt::fs::OpenMode_Write, p_fsID);
	if (file == 0)
	{
		// NOTE: File system code will already have triggered a panic.
		//TT_PANIC("Could not open file '%s' for writing.", p_filename.c_str());
		return false;
	}
	
	// Write the file signature and version
	if (file->write(g_levelFormatSignatureText, static_cast<tt::fs::size_type>(g_levelFormatSignatureLength)) !=
	    static_cast<tt::fs::size_type>(g_levelFormatSignatureLength))
	{
		TT_PANIC("Writing file format signature to file '%s' failed.", p_filename.c_str());
		return false;
	}
	
	const std::string levelData("\n### ONLY EDIT BELOW AND DON'T ALTER THE *** MARKERS! ###\n" + getAsString());
	if (file->write(levelData.c_str(), static_cast<tt::fs::size_type>(levelData.size())) != static_cast<tt::fs::size_type>(levelData.size()))
	{
		TT_PANIC("Saving level data (textual) failed.");
		return false;
	}
	
	return true;
}


bool LevelData::save(const std::string& p_filename, tt::fs::identifier p_fsID) const
{
	tt::fs::FilePtr file = tt::fs::open(p_filename, tt::fs::OpenMode_Write, p_fsID);
	if (file == 0)
	{
		// NOTE: File system code will already have triggered a panic.
		//TT_PANIC("Could not open file '%s' for writing.", p_filename.c_str());
		return false;
	}
	
	return save(file);
}


bool LevelData::save(const tt::fs::FilePtr& p_file) const
{
	const char* filename = "";
	(void)filename; // CAT hack
#if !defined(TT_BUILD_FINAL)
	filename = p_file->getPath();
#endif

	// Write the file signature and version
	if (p_file->write(g_levelFormatSignatureBinary, static_cast<tt::fs::size_type>(g_levelFormatSignatureLength)) !=
	    static_cast<tt::fs::size_type>(g_levelFormatSignatureLength))
	{
		TT_PANIC("Writing file format signature to file '%s' failed.", filename);
		return false;
	}
	
	if (tt::fs::writeInteger(p_file, g_levelFormatCurrentVersion) == false)
	{
		TT_PANIC("Writing file format version to file '%s' failed.", filename);
		return false;
	}
	
	// Write all the chunks making up the level
	// NOTE: Add extra calls here when adding new chunks
	if (saveChunk(g_chunkIDGlobalInfo, p_file) == false)
	{
		TT_PANIC("Saving 'global info' chunk failed.");
		return false;
	}
	
	if (saveChunk(g_chunkIDTiles, p_file) == false)
	{
		TT_PANIC("Saving 'tiles' chunk failed.");
		return false;
	}
	
	if (saveChunk(g_chunkIDEntities, p_file) == false)
	{
		TT_PANIC("Saving 'entities' chunk failed.");
		return false;
	}
	
	if (saveChunk(g_chunkIDNotes, p_file) == false)
	{
		TT_PANIC("Saving 'notes' chunk failed.");
		return false;
	}
	
	if (saveChunk(g_chunkIDPathfinding, p_file) == false)
	{
		TT_PANIC("Saving 'pathfinding' chunk failed.");
		return false;
	}
	
	if (saveChunk(g_chunkIDSteamInfo, p_file) == false)
	{
		TT_PANIC("Saving Steam info chunk failed.");
		return false;
	}
	
	return true;
}


void LevelData::resizeTo(const tt::math::PointRect& p_newRect)
{
	if (p_newRect.hasArea() == false)
	{
		TT_PANIC("Invalid new level dimensions: %d x %d. Must be at least 1 x 1.",
		         p_newRect.getWidth(), p_newRect.getHeight());
		return;
	}
	
	// Resize the attribute layer
	m_attributeLayer->resizeTo(p_newRect);
	
	m_width  = p_newRect.getWidth();
	m_height = p_newRect.getHeight();
	
	// Move all entities and remove any entities that fall outside the new rectangle
	const tt::math::VectorRect levelWorldRect(tileToWorld(getLevelRect()));
	const tt::math::Vector2    entityOffset(tileToWorld(-p_newRect.getPosition()));
	for (entity::EntityInstances::iterator it = m_entities.begin(); it != m_entities.end(); )
	{
		(*it)->setPosition((*it)->getPosition() + entityOffset);
		
		if (levelWorldRect.contains((*it)->getPosition()) == false)
		{
			// New position is outside the level boundaries
			removeEntityFromSelection(*it);
			it = m_entities.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	// Notes also need to be moved (but do not remove them if they fall outside of the level bounds)
	for (Notes::iterator it = m_notes.begin(); it != m_notes.end(); ++it)
	{
		(*it)->setWorldRect((*it)->getWorldRect().getTranslated(entityOffset));
	}
}


void LevelData::swap(const LevelDataPtr& p_other)
{
	TT_NULL_ASSERT(p_other);
	if (p_other == 0 || p_other.get() == this)
	{
		return;
	}
	
	using std::swap;
	
	swap(m_filename, p_other->m_filename);
	swap(m_width,    p_other->m_width);
	swap(m_height,   p_other->m_height);
	m_attributeLayer->swap(p_other->m_attributeLayer);
	swap(m_startPos, p_other->m_startPos);
	swap(m_entities, p_other->m_entities);
	swap(m_notes,    p_other->m_notes);
	swap(m_selectedEntities, p_other->m_selectedEntities);
	swap(m_observers,        p_other->m_observers);
	
	notifyEntitySelectionChanged();
}


LevelDataPtr LevelData::clone() const
{
	return LevelDataPtr(new LevelData(*this));
}


s32 LevelData::getEntityCount() const
{
	return static_cast<s32>(m_entities.size());
}


entity::EntityInstancePtr LevelData::getEntityByIndex(s32 p_index) const
{
	if (p_index < 0 || p_index >= getEntityCount())
	{
		TT_PANIC("Invalid entity index: %d. Must be in range 0 - %d.", p_index, getEntityCount());
		return entity::EntityInstancePtr();
	}
	
	return m_entities.at(static_cast<entity::EntityInstances::size_type>(p_index));
}


entity::EntityInstancePtr LevelData::getEntityByID(s32 p_id) const
{
	for (entity::EntityInstances::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		if ((*it)->getID() == p_id)
		{
			return *it;
		}
	}
	
	return entity::EntityInstancePtr();
}


entity::EntityInstances LevelData::getEntitiesInRect(const tt::math::VectorRect& p_worldRect) const
{
	entity::EntityInstances result;
	
	for (level::entity::EntityInstances::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		if (p_worldRect.contains((*it)->getPosition()))
		{
			result.push_back(*it);
		}
	}
	
	return result;
}


s32 LevelData::createEntityID()
{
	s32 id = -1;
	// Find random ID between 0-100000. A random ID helps preventing merge clashes.
	do
	{
		id = tt::math::Random::getStatic().getNext() % 100000;
	} while (getEntityByID(id) != 0);
	
	return id;
}


bool LevelData::addEntity(const entity::EntityInstancePtr& p_entity)
{
	if (p_entity == 0 || p_entity->isValid() == false)
	{
		TT_PANIC("Cannot add invalid entities (those without type or ID).");
		return false;
	}
	
	entity::EntityInstancePtr existingEntity = getEntityByID(p_entity->getID());
	if (existingEntity != 0)
	{
		TT_PANIC("Level already contains an entity with ID %d (of type '%s'). Will not add another.",
		         p_entity->getID(), existingEntity->getType().c_str());
		return false;
	}
	
	m_entities.push_back(p_entity);
	return true;
}


bool LevelData::removeEntity(s32 p_id)
{
	for (entity::EntityInstances::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		if ((*it)->getID() == p_id)
		{
			removeEntityFromSelection(*it);
			m_entities.erase(it);
			return true;
		}
	}
	
	TT_PANIC("Level does not contain an entity with ID %d. Cannot remove it.", p_id);
	return false;
}


bool LevelData::removeEntity(const entity::EntityInstancePtr& p_entity)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity == 0)
	{
		return false;
	}
	
	entity::EntityInstances::iterator it = std::find(m_entities.begin(), m_entities.end(), p_entity);
	if (it == m_entities.end())
	{
		TT_PANIC("Level does not contain entity 0x%08X (with ID %d). Cannot remove it.",
		         p_entity.get(), p_entity->getID());
		return false;
	}
	
	removeEntityFromSelection(*it);
	m_entities.erase(it);
	return true;
}


bool LevelData::hasEntity(s32 p_id) const
{
	// FIXME: Terribly inefficient...
	for (entity::EntityInstances::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		if ((*it)->getID() == p_id)
		{
			return true;
		}
	}
	
	return false;
}


bool LevelData::hasEntity(const entity::EntityInstancePtr& p_entity) const
{
	return std::find(m_entities.begin(), m_entities.end(), p_entity) != m_entities.end();
}


s32 LevelData::getNoteCount() const
{
	return static_cast<s32>(m_notes.size());
}


NotePtr LevelData::getNote(s32 p_index) const
{
	if (p_index < 0 || p_index >= getNoteCount())
	{
		TT_PANIC("Invalid note index: %d. Must be in range 0 - %d.", p_index, getNoteCount());
		return NotePtr();
	}
	
	return m_notes[static_cast<Notes::size_type>(p_index)];
}


NotePtr LevelData::findNoteOverlappingPosition(const tt::math::Vector2& p_pos) const
{
	for (Notes::const_iterator it = m_notes.begin(); it != m_notes.end(); ++it)
	{
		if ((*it)->getWorldRect().contains(p_pos))
		{
			return *it;
		}
	}
	
	return NotePtr();
}


Notes LevelData::findAllNotesOverlappingPosition(const tt::math::Vector2& p_pos) const
{
	Notes notes;
	
	for (Notes::const_iterator it = m_notes.begin(); it != m_notes.end(); ++it)
	{
		if ((*it)->getWorldRect().contains(p_pos))
		{
			notes.push_back(*it);
		}
	}
	
	return notes;
}


void LevelData::addNote(const NotePtr& p_note)
{
	TT_NULL_ASSERT(p_note);
	if (p_note != 0)
	{
		m_notes.push_back(p_note);
	}
}


void LevelData::removeNote(s32 p_index)
{
	if (p_index < 0 || p_index >= getNoteCount())
	{
		TT_PANIC("Invalid note index: %d. Must be in range 0 - %d.", p_index, getNoteCount());
		return;
	}
	
	Notes::iterator it = m_notes.begin();
	std::advance(it, p_index);
	m_notes.erase(it);
}


void LevelData::removeNote(const NotePtr& p_note)
{
	TT_NULL_ASSERT(p_note);
	if (p_note == 0)
	{
		return;
	}
	
	Notes::iterator it = std::find(m_notes.begin(), m_notes.end(), p_note);
	if (it == m_notes.end())
	{
		TT_PANIC("Level does not contain note 0x%08X. Cannot remove it.", p_note.get());
		return;
	}
	
	m_notes.erase(it);
}


void LevelData::addEntityToSelection(const entity::EntityInstancePtr& p_entity)
{
	if (p_entity == 0)
	{
		TT_PANIC("Cannot select null entities.");
		return;
	}
	
	if (hasEntity(p_entity) == false)
	{
		TT_PANIC("Entity 0x%08X (type '%s', ID %d) does not belong to this level. Cannot select it.",
		         p_entity.get(), p_entity->getType().c_str(), p_entity->getID());
		return;
	}
	
	if (m_selectedEntities.find(p_entity) == m_selectedEntities.end())
	{
		m_selectedEntities.insert(p_entity);
		notifyEntitySelectionChanged();
	}
}


void LevelData::removeEntityFromSelection(const entity::EntityInstancePtr& p_entity)
{
	TT_NULL_ASSERT(p_entity);
	entity::EntityInstanceSet::iterator it = m_selectedEntities.find(p_entity);
	if (it != m_selectedEntities.end())
	{
		m_selectedEntities.erase(it);
		notifyEntitySelectionChanged();
	}
}


void LevelData::selectAllEntities()
{
	setSelectedEntities(entity::EntityInstanceSet(m_entities.begin(), m_entities.end()));
}


void LevelData::deselectAllEntities()
{
	if (m_selectedEntities.empty() == false)
	{
		m_selectedEntities.clear();
		notifyEntitySelectionChanged();
	}
}


bool LevelData::isEntitySelected(const entity::EntityInstancePtr& p_entity) const
{
	return m_selectedEntities.find(p_entity) != m_selectedEntities.end();
}


void LevelData::setSelectedEntities(const entity::EntityInstanceSet& p_selection)
{
	entity::EntityInstanceSet validSelection;
	for (entity::EntityInstanceSet::const_iterator it = p_selection.begin(); it != p_selection.end(); ++it)
	{
		const entity::EntityInstancePtr& entity(*it);
		
		if (entity == 0)
		{
			TT_PANIC("Cannot select null entities.");
			continue;
		}
		
		if (hasEntity(entity) == false)
		{
			TT_PANIC("Entity 0x%08X (type '%s', ID %d) does not belong to this level. Cannot select it.",
			         entity.get(), entity->getType().c_str(), entity->getID());
			continue;
		}
		
		validSelection.insert(entity);
	}
	
	if (validSelection != m_selectedEntities)
	{
		m_selectedEntities = validSelection;
		notifyEntitySelectionChanged();
	}
}


void LevelData::registerObserver(const LevelDataObserverWeakPtr& p_observer)
{
	LevelDataObserverPtr observer(p_observer.lock());
	TT_NULL_ASSERT(observer);
	if (observer == 0)
	{
		return;
	}
	
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); ++it)
	{
		if ((*it).lock() == observer)
		{
			TT_PANIC("Observer 0x%08X is already registered.", observer.get());
			return;
		}
	}
	
	m_observers.push_back(p_observer);
}


void LevelData::unregisterObserver(const LevelDataObserverWeakPtr& p_observer)
{
	LevelDataObserverPtr observer(p_observer.lock());
	TT_NULL_ASSERT(observer);
	if (observer == 0)
	{
		return;
	}
	
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); ++it)
	{
		if ((*it).lock() == observer)
		{
			m_observers.erase(it);
			return;
		}
	}
	
	TT_PANIC("Observer 0x%08X was not registered.", observer.get());
}


void LevelData::clearObservers()
{
	m_observers.clear();
	if(m_attributeLayer != 0)
	{
		m_attributeLayer->clearObservers();
	}
	for (entity::EntityInstanceSet::iterator it = m_selectedEntities.begin();
	     it != m_selectedEntities.end(); ++it)
	{
		(*it)->clearObservers();
	}
}


tt::math::PointRect LevelData::getLevelRect() const
{
	return tt::math::PointRect(tt::math::Point2(0, 0), m_width, m_height);
}


std::string LevelData::getAsString() const
{
	Json::Value data;
	
	data["version"] = g_levelFormatCurrentVersion;
	
	// Info
	data["info"] = Json::Value(Json::objectValue);
	data["info"]["version"]         = g_levelFormatCurrentVersion;
	data["info"]["width"]           = getLevelWidth();
	data["info"]["height"]          = getLevelHeight();
	data["info"]["theme"]           = getThemeTypeNameAsString(getLevelTheme());
	data["info"]["default_mission"] = getDefaultMission();
	data["info"]["background"]      = getLevelBackground();
	
	// FIXME: Downcasting u64 here; disabled for now
	//result["info"]["workshopID"] = static_cast<u32>(getPublishedFileId());
	//result["info"]["ownerID"]    = static_cast<u32>(getOwnerId());
	
	// Theme colors
	data["theme_colors"] = Json::Value(Json::objectValue);
	for (s32 skinIndex = 0; skinIndex < level::skin::SkinConfigType_Count; ++skinIndex)
	{
		const level::skin::SkinConfigType skinConfig = static_cast<level::skin::SkinConfigType>(skinIndex);
		const std::string skinName(std::string(level::skin::getSkinConfigTypeName(skinConfig)));
		
		data["theme_colors"][skinName] = Json::Value(Json::objectValue);
		for (s32 themeIndex = 0; themeIndex < level::ThemeType_Count; ++themeIndex)
		{
			const level::ThemeType theme = static_cast<level::ThemeType>(themeIndex);
			const std::string themeName(level::getThemeTypeNameAsString(theme));
			const tt::engine::renderer::ColorRGBA& color(getThemeColor(skinConfig, theme));

			data["theme_colors"][skinName][themeName] = Json::Value(Json::arrayValue);
			data["theme_colors"][skinName][themeName].append(static_cast<u32>(color.r));
			data["theme_colors"][skinName][themeName].append(static_cast<u32>(color.g));
			data["theme_colors"][skinName][themeName].append(static_cast<u32>(color.b));
			data["theme_colors"][skinName][themeName].append(static_cast<u32>(color.a));
		}
	}
	data["entities"] = Json::Value(Json::arrayValue);
	
	// Create JSON for all the entities in the level
	// NOTE: The entities are sorted by ID, to keep diffs as in sync as possible
	typedef std::map<s32, level::entity::EntityInstancePtr> EntitiesByID;
	EntitiesByID entitiesByID;
	
	{
		const level::entity::EntityInstances& allEntities(getAllEntities());
		for (level::entity::EntityInstances::const_iterator it = allEntities.begin();
		     it != allEntities.end(); ++it)
		{
			entitiesByID[(*it)->getID()] = *it;
		}
	}
	
	if (entitiesByID.empty() == false)
	{
		for (EntitiesByID::const_iterator it = entitiesByID.begin(); it != entitiesByID.end(); ++it)
		{
			data["entities"].append(createEntityInstanceJson((*it).second));
		}
	}
	
	// Pathfinding radii
	data["pathfinding_radii"] = Json::Value(Json::arrayValue);
	{
		using namespace game::pathfinding;
		for (PathMgr::AgentRadii::const_iterator it = m_agentRadii.begin(); it != m_agentRadii.end(); ++it)
		{
			data["pathfinding_radii"].append(*it);
		}
	}
	
	// Notes
	const level::Notes& notes(getAllNotes());
	data["notes"] = Json::Value(Json::arrayValue);
	for (level::Notes::const_iterator it = notes.begin(); it != notes.end(); ++it)
	{
		const level::NotePtr&       note(*it);
		const tt::math::VectorRect& rect(note->getWorldRect());
		Json::Value entry;
		entry["text"]   = tt::str::utf16ToUtf8(note->getText());
		entry["x"]      = rect.getPosition().x;
		entry["y"]      = rect.getPosition().y;
		entry["width"]  = rect.getWidth();
		entry["height"] = rect.getHeight();
		data["notes"].append(entry);
	}
	
	static const std::string newline("\n");
	
	std::string result;
	
	// Data section
	result += g_dataBeginMarker + Json::StyledWriter().write(data) + g_dataEndMarker;
	
	const s32 levelWidth  = getLevelWidth();
	const s32 levelHeight = getLevelHeight();
	
	// Create a plain-text copyable representation of the level tiles
	level::AttributeLayerPtr attribs(getAttributeLayer());
	
	tt::math::Point2 pos;
	
	// CollisionType of each tile
	result += g_tilesBeginMarker;
	for (pos.y = levelHeight - 1; pos.y >= 0; --pos.y)
	{
		for (pos.x = 0; pos.x < levelWidth; ++pos.x)
		{
			const level::CollisionType type = attribs->getCollisionType(pos);
			result += level::getCollisionTypeAsChar(type);
		}
		
		result += newline;
	}
	result += g_tilesEndMarker;

	// ThemeType of each tile
	result += g_themeTilesBeginMarker;
	for (pos.y = levelHeight - 1; pos.y >= 0; --pos.y)
	{
		for (pos.x = 0; pos.x < levelWidth; ++pos.x)
		{
			const level::ThemeType type = attribs->getThemeType(pos);
			result += level::getThemeTypeAsChar(type);
		}
		
		result += newline;
	}
	result += g_themeTilesEndMarker;
	
	return result;
}


void LevelData::setLevelBackground(const std::string& p_backgroundName)
{
	m_levelBackground = p_backgroundName;
}


void LevelData::setLevelTheme(ThemeType p_theme)
{
	if (isValidLevelTheme(p_theme) == false)
	{
		TT_PANIC("Invalid theme for the level as a whole: %d", p_theme);
		return;
	}
	
	m_levelTheme = p_theme;
}


void LevelData::setThemeColor(skin::SkinConfigType                   p_skinConfig,
                              ThemeType                              p_theme,
                              const tt::engine::renderer::ColorRGBA& p_color)
{
	TT_ASSERTMSG(skin::isValidSkinConfigType(p_skinConfig),
	             "Invalid skin config: %d. Cannot set theme color.", p_skinConfig);
	TT_ASSERTMSG(isValidThemeType(p_theme), "Invalid theme: %d. Cannot set theme color.", p_theme);
	if (skin::isValidSkinConfigType(p_skinConfig) && isValidThemeType(p_theme))
	{
		m_themeColor[p_skinConfig][p_theme] = p_color;
	}
}


const tt::engine::renderer::ColorRGBA& LevelData::getThemeColor(
		skin::SkinConfigType p_skinConfig,
		ThemeType            p_theme) const
{
	TT_ASSERTMSG(skin::isValidSkinConfigType(p_skinConfig),
	             "Invalid skin config: %d. Cannot get theme color.", p_skinConfig);
	TT_ASSERTMSG(isValidThemeType(p_theme), "Invalid theme: %d. Cannot get theme color.", p_theme);
	static const tt::engine::renderer::ColorRGBA defaultColor(tt::engine::renderer::ColorRGB::white);
	return (skin::isValidSkinConfigType(p_skinConfig) && isValidThemeType(p_theme)) ?
			m_themeColor[p_skinConfig][p_theme] : defaultColor;
}


tt::code::AutoGrowBufferPtr LevelData::createPathfindingData()
{
	m_pathfindingData = tt::code::AutoGrowBuffer::create(4096, 4096);
	return m_pathfindingData;
}


bool LevelData::isValidLevelTheme(ThemeType p_theme)
{
	// The level theme can not be "use level default" (that would be recursive)
	return isValidThemeType(p_theme) && p_theme != ThemeType_UseLevelDefault;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

LevelData::LevelData()
:
m_filename(),
m_width(0),
m_height(0),
m_attributeLayer(),
m_levelBackground(),
m_levelTheme(ThemeType_Sand),
m_levelDefaultMission(),
m_startPos(0, 0),
m_entities(),
m_notes(),
m_agentRadii(),
m_pathfindingData(),
m_steamPublishedFileId(0),
m_steamOwnerId(0),
m_selectedEntities(),
m_observers()
{
	setLevelThemeToDefault();
}


LevelData::LevelData(const LevelData& p_rhs)
:
m_filename(p_rhs.m_filename),
m_width(p_rhs.m_width),
m_height(p_rhs.m_height),
m_attributeLayer(p_rhs.m_attributeLayer != 0 ? p_rhs.m_attributeLayer->clone() : AttributeLayerPtr()),
m_levelBackground(p_rhs.m_levelBackground),
m_levelTheme(p_rhs.m_levelTheme),
m_levelDefaultMission(p_rhs.m_levelDefaultMission),
m_startPos(p_rhs.m_startPos),
m_entities(),
m_notes(),
m_pathfindingData((p_rhs.m_pathfindingData != 0) ? p_rhs.m_pathfindingData->clone() : tt::code::AutoGrowBufferPtr()),
m_steamPublishedFileId(p_rhs.m_steamPublishedFileId),
m_steamOwnerId        (p_rhs.m_steamOwnerId),
m_selectedEntities(),
m_observers(p_rhs.m_observers)
{
	// Clone all entities
	for (entity::EntityInstances::const_iterator it = p_rhs.m_entities.begin();
	     it != p_rhs.m_entities.end(); ++it)
	{
		const entity::EntityInstancePtr& otherEntity(*it);
		entity::EntityInstancePtr clonedEntity(otherEntity->clone());
		m_entities.push_back(clonedEntity);
		
		if (p_rhs.isEntitySelected(otherEntity))
		{
			m_selectedEntities.insert(clonedEntity);
		}
	}
	
	// Clone all notes
	m_notes.reserve(p_rhs.m_notes.size());
	for (Notes::const_iterator it = p_rhs.m_notes.begin(); it != p_rhs.m_notes.end(); ++it)
	{
		m_notes.push_back((*it)->clone());
	}
	
	// Clone all theme colors
	for (s32 skinIndex = 0; skinIndex < skin::SkinConfigType_Count; ++skinIndex)
	{
		for (s32 themeIndex = 0; themeIndex < ThemeType_Count; ++themeIndex)
		{
			m_themeColor[skinIndex][themeIndex] = p_rhs.m_themeColor[skinIndex][themeIndex];
		}
	}
}


void LevelData::notifyEntitySelectionChanged()
{
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); )
	{
		LevelDataObserverPtr observer((*it).lock());
		if (observer == 0)
		{
			it = m_observers.erase(it);
		}
		else
		{
			observer->onLevelDataEntitySelectionChanged();
			++it;
		}
	}
}


void LevelData::setLevelThemeToDefault()
{
	setLevelTheme(ThemeType_Sand);
	
	for (s32 skinIndex = 0; skinIndex < skin::SkinConfigType_Count; ++skinIndex)
	{
		for (s32 themeIndex = 0; themeIndex < ThemeType_Count; ++themeIndex)
		{
			m_themeColor[skinIndex][themeIndex] = tt::engine::renderer::ColorRGB::white;
		}
	}
}


void LevelData::loadThemeColors(size_t& p_chunkSize, const u8*& p_chunkData,
                                skin::SkinConfigType p_skinConfig)
{
	namespace bu = tt::code::bufferutils;
	
	const s32 colorCount = bu::get<s32>(p_chunkData, p_chunkSize);
	for (s32 i = 0; i < colorCount; ++i)
	{
		const std::string themeName = bu::get<std::string>(p_chunkData, p_chunkSize);
		const tt::engine::renderer::ColorRGBA color =
				bu::get<tt::engine::renderer::ColorRGBA>(p_chunkData, p_chunkSize);
		const ThemeType theme = getThemeTypeFromName(themeName);
		TT_ASSERTMSG(isValidThemeType(theme),
		             "Level data contains a color for unsupported theme '%s'. "
		             "This color will be ignored.", themeName.c_str());
		if (isValidThemeType(theme) && skin::isValidSkinConfigType(p_skinConfig))
		{
			m_themeColor[p_skinConfig][theme] = color;
		}
	}
}


bool LevelData::loadFromFile(const tt::fs::FilePtr& p_file, const std::string& p_filename,
                             bool p_fileContainsOtherData)
{
	tt::code::BufferPtr buffer(p_file->getContent());
	const tt::fs::pos_type offset(p_file->getPosition());
	
	if (buffer == 0)
	{
		TT_PANIC("Level file '%s' is empty. Cannot load leveldata.", p_filename.c_str());
		return false;
	}
	
	const u8* filePtr        = reinterpret_cast<const u8*>(buffer->getData()) + offset;
	size_t    remainingBytes = static_cast<size_t>(buffer->getSize() - offset);
	const size_t remainingBytesBeforeLevel = remainingBytes;
	
	namespace bu = tt::code::bufferutils;
	
	// Read and verify the file signature
	u8 signature[g_levelFormatSignatureLength] = { 0 };
	bu::get(signature, g_levelFormatSignatureLength, filePtr, remainingBytes);
	bool isBinary = true;
	for (size_t i = 0; i < g_levelFormatSignatureLength; ++i)
	{
		if (signature[i] != g_levelFormatSignatureBinary[i])
		{
			isBinary = false;
			break;
		}
	}
	
	// Check for textual version
	if (isBinary == false)
	{
		for (size_t i = 0; i < g_levelFormatSignatureLength; ++i)
		{
			if (signature[i] != g_levelFormatSignatureText[i])
			{
				// Does not appear to be a valid binary nor text level file
				TT_PANIC("Invalid level file: file signature byte %d (0x%02X) does not match expected byte 0x%02X.",
				         s32(i), u32(signature[i]), u32(g_levelFormatSignatureText[i]));
				return false;
			}
		}
	}
	
	// Here we should load bin or text
	if (isBinary == false)
	{
		// Switch to textual loading
		m_filename = p_filename;
		std::string text(reinterpret_cast<char const*>(filePtr), remainingBytes);
		if (parseFromString(text))
		{
			return true;
		}
		
		// Failed to load level; reset filename
		m_filename.clear();
		return false;
	}
	
	// Read and check the file version
	// (only one version is supported at the moment, but this can be expanded later)
	const u32 fileVersion = bu::get<u32>(filePtr, remainingBytes);
	if (fileVersion != g_levelFormatCurrentVersion)
	{
#if !defined(TT_BUILD_FINAL)
		TT_PANIC("Unsupported level file format version: %u. Expected version %u. "
		         "Cannot load the file.\nFilename: %s",
		         fileVersion, g_levelFormatCurrentVersion, p_file->getPath());
#endif
		return false;
	}
	
	// Default behavior (loading from binary)
	while (remainingBytes > 0)
	{
		// Read and validate the chunk marker
		const u32 chunkMarker = bu::get<u32>(filePtr, remainingBytes);
		if (chunkMarker != g_levelFormatChunkMarker)
		{
			if(p_fileContainsOtherData)
			{
				// Reached end of level data, terminate loop
				break;
			}
#if !defined(TT_BUILD_FINAL)
			TT_PANIC("Level file appears to be corrupt. Expected to find chunk marker 0x%08X, "
			         "but found 0x%08X instead.\nFilename: %s",
			         g_levelFormatChunkMarker, chunkMarker, p_file->getPath());
#endif
			return false;
		}
		
		// Chunk size is the size of the [ID, version, data] block (without the CRC)
		// Chunk CRC is calculated for the same [ID, version, data] block.
		const size_t  chunkSize    = static_cast<size_t>(bu::get<u32>(filePtr, remainingBytes));
		const u32     chunkCRC     = bu::get<u32>(filePtr, remainingBytes);
		
		if (chunkSize > remainingBytes)
		{
#if !defined(TT_BUILD_FINAL)
			TT_PANIC("Size of the next chunk in the level file (%d bytes) is larger than "
			         "the number of remaining bytes in the file (%d bytes). "
			         "Cannot process this chunk.\nFilename: %s",
			         chunkSize, remainingBytes, p_file->getPath());
#endif
			break;
		}
		
		const u32 calculatedCRC = tt::math::hash::CRC32().calcCRC(filePtr, chunkSize);
		if (calculatedCRC == chunkCRC)
		{
			// Chunk CRC is valid: parse the chunk
			parseChunk(chunkSize, filePtr);
		}
#if !defined(TT_BUILD_FINAL)
		else
		{
			TT_PANIC("Encountered corrupt chunk (of %d bytes) in level file. Skipping this chunk.\n"
			         "Expected chunk CRC 0x%08X, but calculated CRC is 0x%08X.\nFilename: %s",
			         s32(chunkSize), chunkCRC, calculatedCRC, p_file->getPath());
		}
#endif
		
		filePtr        += chunkSize;
		remainingBytes -= chunkSize;
	}
	
	if(p_fileContainsOtherData)
	{
		// Restore file position
		const tt::fs::pos_type readBytes = static_cast<tt::fs::pos_type>(
			remainingBytesBeforeLevel - remainingBytes - sizeof(g_levelFormatChunkMarker));
		
		tt::fs::pos_type newPosition = offset + readBytes;
		
		p_file->seek(newPosition, tt::fs::SeekPos_Set);
	}
	
	// Set filename
	m_filename = p_filename;
	
	return true;
}


bool LevelData::parseFromString(const std::string& p_levelData)
{
	// First find data marker
	const std::string dataStr = getSectionBetweenMarkers(g_dataBeginMarker, g_dataEndMarker, p_levelData);
	std::string tilesStr      = getSectionBetweenMarkers(g_tilesBeginMarker, g_tilesEndMarker, p_levelData);
	std::string themeTilesStr = getSectionBetweenMarkers(g_themeTilesBeginMarker, g_themeTilesEndMarker, p_levelData);
	
	Json::Value data;
	if (Json::Reader().parse(dataStr, data, false) == false ||
	    data.isObject() == false)
	{
		TT_PANIC("Failed to parse data section");
		return false;
	}
	
	// Info
	const Json::Value& versionRoot = data["version"];
	if (versionRoot.isInt() == false)
	{
		TT_PANIC("Info section misses or contains an invalid version number");
		return false;
	}
	const s32 fileVersion(versionRoot.asInt());
	if (fileVersion != g_levelFormatCurrentVersion)
	{
#if !defined(TT_BUILD_FINAL)
		TT_PANIC("Unsupported level file format version: %u. Expected version %u. "
		         "Cannot load the file.\nFilename: %s",
		         fileVersion, g_levelFormatCurrentVersion, m_filename.c_str());
#endif
		TT_PANIC("Mismatching version number");
		return false;
	}
	
	const Json::Value& infoRoot = data["info"];
	if (infoRoot.isObject() == false)
	{
		TT_PANIC("Info section not an object");
		return false;
	}
	
	if (infoRoot.isMember("width"     ) == false || infoRoot["width"     ].isConvertibleTo(Json::intValue)  == false ||
	    infoRoot.isMember("height"    ) == false || infoRoot["width"     ].isConvertibleTo(Json::intValue)  == false ||
	    infoRoot.isMember("background") == false || infoRoot["background"].isString()  == false                      ||
	    infoRoot.isMember("theme"     ) == false || infoRoot["theme"     ].isString()  == false)
	{
		TT_PANIC("Info section contains invalid members");
		return false;
	}
	
	m_width  = infoRoot["width"].asInt();
	m_height = infoRoot["height"].asInt();
	
	// Obsolete
	m_startPos.x = m_width / 2;
	m_startPos.y = m_height / 2;
	
	// Theme
	setLevelThemeToDefault();
	const std::string themeName = infoRoot["theme"].asString();
	const ThemeType   theme     = getThemeTypeFromName(themeName);
	if (isValidLevelTheme(theme))
	{
		setLevelTheme(theme);
	}
	else
	{
		TT_PANIC("Got invalid level theme ('%s') from level data. "
		         "Falling back to default level theme '%s'.\nFilename: %s",
		         themeName.c_str(), getThemeTypeName(m_levelTheme), m_filename.c_str());
	}
	
	// Default mission
	if (infoRoot.isMember("default_mission") && infoRoot["default_mission"].isString())
	{
		setDefaultMission(infoRoot["default_mission"].asString());
	}
	
	const Json::Value& themeColorRoot = data["theme_colors"];
	if (themeColorRoot.isObject() && themeColorRoot.isNull() == false)
	{
		const Json::Value::Members skinNames(themeColorRoot.getMemberNames());
		for (Json::Value::Members::const_iterator it = skinNames.begin(); it != skinNames.end(); ++it)
		{
			const std::string          skinName(*it);
			const skin::SkinConfigType skin = skin::getSkinConfigTypeFromName(skinName);
			
			if (skin::isValidSkinConfigType(skin) == false)
			{
				TT_WARN("Level data contains an unsupported skin '%s'. "
				        "This skin will be ignored.\nFilename: %s", skinName.c_str(), m_filename.c_str());
				continue;
			}
			
			const Json::Value& skinRoot(themeColorRoot[skinName]);
			const Json::Value::Members themeNames(skinRoot.getMemberNames());
			for (Json::Value::Members::const_iterator themeIt = themeNames.begin();
			     themeIt != themeNames.end(); ++themeIt)
			{
				const std::string name(*themeIt);
				const ThemeType   type = getThemeTypeFromName(name);
				
				TT_ASSERTMSG(isValidThemeType(type),
				             "Level data contains a color for unsupported theme '%s'. "
				             "This color will be ignored.\nFilename: %s", name.c_str(), m_filename.c_str());
				
				if (skinRoot[*themeIt].isArray() == false || skinRoot[*themeIt].size() != 4)
				{
					TT_PANIC("Color is not array or has incorrect dimensions.\nFilename: %s", m_filename.c_str());
					return false;
				}
				
				if (isValidThemeType(type))
				{
					const tt::engine::renderer::ColorRGBA color(
						static_cast<u8>(skinRoot[*themeIt][static_cast<Json::Value::UInt>(0)].asInt()),
						static_cast<u8>(skinRoot[*themeIt][static_cast<Json::Value::UInt>(1)].asInt()),
						static_cast<u8>(skinRoot[*themeIt][static_cast<Json::Value::UInt>(2)].asInt()),
						static_cast<u8>(skinRoot[*themeIt][static_cast<Json::Value::UInt>(3)].asInt()));
					m_themeColor[skin][type] = color;
				}
			}
		}
	}
	
	// Level background (unused?)
	m_levelBackground = infoRoot["background"].asString();
	
	// Entities
	const Json::Value& entityRoot = data["entities"];
	if (entityRoot.isArray() == false)
	{
		TT_PANIC("Entity section not an array.\nFilename: %s", m_filename.c_str());
		return false;
	}
	
	for (Json::Value::iterator it = entityRoot.begin(); it != entityRoot.end(); ++it)
	{
		const Json::Value& entry(*it);
		
		entity::EntityInstancePtr instance = createEntityInstanceFromJson(entry);
		if (instance != 0)
		{
			addEntity(instance);
		}
	}
	
	// Pathfinding radii
	const Json::Value& pathfindingRadiiRoot = data["pathfinding_radii"];
	{
		m_agentRadii.clear();
		for (Json::Value::iterator it = pathfindingRadiiRoot.begin(); it != pathfindingRadiiRoot.end(); ++it)
		{
			const Json::Value& entry(*it);
			if (entry.isConvertibleTo(Json::realValue) == false)
			{
				TT_PANIC("Invalid radii.\nFilename: %s", m_filename.c_str());
			}
			m_agentRadii.insert(static_cast<real>(entry.asDouble()));
		}
	}
	
	// Notes
	const Json::Value& notesRoot = data["notes"];
	{
		m_notes.clear();
		for (Json::Value::iterator it = notesRoot.begin(); it != notesRoot.end(); ++it)
		{
			const Json::Value& entry(*it);
			std::wstring text = tt::str::widen(entry["text"].asString());
			real x            = static_cast<real>(entry["x"].asDouble());
			real y            = static_cast<real>(entry["y"].asDouble());
			real width        = static_cast<real>(entry["width"].asDouble());
			real height       = static_cast<real>(entry["height"].asDouble());
			addNote(Note::create(tt::math::VectorRect(tt::math::Vector2(x, y), width, height), text));
		}
	}
	
	// Tiles
	tt::str::replace(tilesStr, "\n", "");
	tt::str::replace(themeTilesStr, "\n", "");
	if ((m_width * m_height) != static_cast<s32>(tilesStr.size()))
	{
		TT_PANIC("Number of tiles in tiles section (%d) mismatches the number of tiles based on level size (%d).\nFilename: %s",
			tilesStr.size(), (m_width * m_height), m_filename.c_str());
		return false;
	}
	
	if ((m_width * m_height) != static_cast<s32>(themeTilesStr.size()))
	{
		TT_PANIC("Number of theme tiles in tiles section (%d) mismatches the number of tiles based on level size (%d).\nFilename: %s",
			themeTilesStr.size(), (m_width * m_height), m_filename.c_str());
		return false;
	}
	
	AttributeLayerPtr layer(AttributeLayer::create(m_width, m_height));
	if (layer == 0)
	{
		TT_PANIC("Could not create a tile layer of %d x %d tiles.\nFilename: %s", m_width, m_height, m_filename.c_str());
		return false;
	}
	
	u8* attribs(layer->getRawData());
	s32 i = 0;
	for (s32 y = m_height-1; y >= 0; --y)
	{
		for (s32 x = 0; x < m_width; ++x)
		{
			const s32 index = (y * m_width) + x;
			const CollisionType collType = getCollisionTypeFromChar(tilesStr[i]);
			if (isValidCollisionType(collType))
			{
				level::setCollisionType(attribs[index], collType);
			}
			else
			{
				TT_WARN("Unsupported tile: '%c'.\nFilename: %s", collType, m_filename.c_str());
			}
			
			const ThemeType themeType = getThemeTypeFromChar(themeTilesStr[i]);
			if (isValidThemeType(themeType))
			{
				level::setThemeType(attribs[index], themeType);
			}
			else
			{
				TT_WARN("Unsupported theme tile: '%c'.\nFilename: %s", themeType, m_filename.c_str());
			}
			++i;
		}
	}
	
	// NOTE: Always overwrite the attribute layer pointer.
	//       If we need support for multiple tile layers, this needs to be changed (obviously).
	m_attributeLayer = layer;
	
	return true;
}


bool LevelData::parseChunk(size_t p_chunkSize, const u8* p_chunkData)
{
	namespace bu = tt::code::bufferutils;
	
	const u32 chunkID      = bu::get<u32>(p_chunkData, p_chunkSize);
	const u32 chunkVersion = bu::get<u32>(p_chunkData, p_chunkSize);
	
	switch (chunkID)
	{
	case g_chunkIDGlobalInfo:  return parseChunkGlobalInfo (chunkVersion, p_chunkSize, p_chunkData);
	case g_chunkIDTiles:       return parseChunkTiles      (chunkVersion, p_chunkSize, p_chunkData);
	case g_chunkIDEntities:    return parseChunkEntities   (chunkVersion, p_chunkSize, p_chunkData);
	case g_chunkIDNotes:       return parseChunkNotes      (chunkVersion, p_chunkSize, p_chunkData);
	case g_chunkIDPathfinding: return parseChunkPathfinding(chunkVersion, p_chunkSize, p_chunkData);
	case g_chunkIDSteamInfo:   return parseChunkSteamInfo  (chunkVersion, p_chunkSize, p_chunkData);
		
	default:
		TT_PANIC("Unsupported chunk ID: %d. Ignoring any data from it.", chunkID);
		return false;
	}
}


bool LevelData::parseChunkGlobalInfo(u32 p_version, size_t p_chunkSize, const u8* p_chunkData)
{
	static const u32 firstVersionWithLevelTheme               = 2;
	static const u32 firstVersionWithThemeColors              = 3;
	static const u32 firstVersionWithThemeColorsPerSkinConfig = 4;
	static const u32 firstVersionWithLevelBackground          = 5;
	static const u32 firstVersionWithDefaultMission           = 6;
	
	TT_ASSERTMSG(p_version <= g_chunkVersionGlobalInfo,
	             "Unsupported global info chunk version: %u. It is too new. "
	             "This code supports global info chunks up to and including version %u.",
	             p_version, g_chunkVersionGlobalInfo);
	
	namespace bu = tt::code::bufferutils;
	
	// NOTE: Using be_get instead of get, because the legacy getWideString used big-endian internally
	// NOTE: This string represents the name, but is unused
	std::wstring dummy = bu::be_get<std::wstring>(p_chunkData, p_chunkSize);
	
	m_width      = bu::get<s32>(p_chunkData, p_chunkSize);
	m_height     = bu::get<s32>(p_chunkData, p_chunkSize);
	m_startPos.x = bu::get<s32>(p_chunkData, p_chunkSize);
	m_startPos.y = bu::get<s32>(p_chunkData, p_chunkSize);
	
	setLevelThemeToDefault();
	if (p_version >= firstVersionWithLevelTheme)
	{
		const std::string themeName = bu::get<std::string>(p_chunkData, p_chunkSize);
		const ThemeType   theme     = getThemeTypeFromName(themeName);
		if (isValidLevelTheme(theme))
		{
			setLevelTheme(theme);
		}
		else
		{
			TT_PANIC("Got invalid level theme ('%s') from level data. "
			         "Falling back to default level theme '%s'.",
			         themeName.c_str(), getThemeTypeName(m_levelTheme));
		}
	}
	
	if (p_version >= firstVersionWithDefaultMission)
	{
		const std::string defaultMission = bu::get<std::string>(p_chunkData, p_chunkSize);
		setDefaultMission(defaultMission);
		
		const std::string& missionID(AppGlobal::getGame()->getMissionID());
		if (missionID.empty() || missionID == "*")
		{
			AppGlobal::getGame()->setMissionID(defaultMission);
		}
	}
	
	if (p_version >= firstVersionWithThemeColorsPerSkinConfig)
	{
		// Theme colors are stored per skin config
		const s32 configCount = bu::get<s32>(p_chunkData, p_chunkSize);
		
		for (s32 skinIndex = 0; skinIndex < configCount; ++skinIndex)
		{
			const std::string          configName = bu::get<std::string>(p_chunkData, p_chunkSize);
			const skin::SkinConfigType config     = skin::getSkinConfigTypeFromName(configName);
			TT_ASSERTMSG(skin::isValidSkinConfigType(config),
			             "Level data contains colors for unsupported skin config '%s'. "
			             "These colors will be ignored.", configName.c_str());
			
			loadThemeColors(p_chunkSize, p_chunkData, config);
		}
	}
	else if (p_version >= firstVersionWithThemeColors)
	{
		// In this version, only one set of theme colors was available: those for "solid"
		loadThemeColors(p_chunkSize, p_chunkData, skin::SkinConfigType_Solid);
	}
	
	m_levelBackground.clear();
	if (p_version >= firstVersionWithLevelBackground)
	{
		m_levelBackground = bu::get<std::string>(p_chunkData, p_chunkSize);
	}
	
	return true;
}


bool LevelData::parseChunkTiles(u32 p_version, size_t p_chunkSize, const u8* p_chunkData)
{
	TT_ASSERTMSG(p_version <= g_chunkVersionTiles,
	             "Unsupported tiles chunk version: %u. It is too new. "
	             "This code supports tiles chunks up to and including version %u.",
	             p_version, g_chunkVersionTiles);
	
	namespace bu = tt::code::bufferutils;
	
	const s32 layerCount = bu::get<s32>(p_chunkData, p_chunkSize);
	if (layerCount < 0)
	{
		TT_PANIC("Invalid tile layer count: %d. Must be at least 0.", layerCount);
		return false;
	}
	
	// For now, the game only supports one tile layer
	TT_ASSERTMSG(layerCount <= 1,
	             "More than one tile layer is specified (%d layers). The game currently supports "
	             "only one layer. The last layer in the file will be used.", layerCount);
	
	for (s32 i = 0; i < layerCount; ++i)
	{
		const s32 layerWidth  = bu::get<s32>(p_chunkData, p_chunkSize);
		const s32 layerHeight = bu::get<s32>(p_chunkData, p_chunkSize);
		if (layerWidth <= 0 || layerHeight <= 0)
		{
			TT_PANIC("Tile layer %d has invalid dimensions: %d x %d. Must be at least 1 x 1.",
			         layerWidth, layerHeight);
			return false;
		}
		
		const size_t layerByteSize = static_cast<size_t>(layerWidth * layerHeight);
		if (layerByteSize > p_chunkSize)
		{
			TT_PANIC("Not enough bytes remain in the tiles chunk for a tile layer of %d x %d tiles. "
			         "Need %d bytes, but %d bytes remain.",
			         layerWidth, layerHeight, s32(layerByteSize), s32(p_chunkSize));
			return false;
		}
		
		AttributeLayerPtr layer(AttributeLayer::create(layerWidth, layerHeight));
		if (layer == 0)
		{
			TT_PANIC("Could not create a tile layer of %d x %d tiles.", layerWidth, layerHeight);
			return false;
		}
		
		bu::get(layer->getRawData(), layerByteSize, p_chunkData, p_chunkSize);
		
		// NOTE: Always overwrite the attribute layer pointer.
		//       If we need support for multiple tile layers, this needs to be changed (obviously).
		m_attributeLayer = layer;
	}
	
	return true;
}


bool LevelData::parseChunkEntities(u32 p_version, size_t p_chunkSize, const u8* p_chunkData)
{
	TT_ASSERTMSG(p_version <= g_chunkVersionEntities,
	             "Unsupported entities chunk version: %u. It is too new. "
	             "This code supports entities chunks up to and including version %u.",
	             p_version, g_chunkVersionEntities);
	
	namespace bu = tt::code::bufferutils;
	
	const s32 entityCount = bu::get<s32>(p_chunkData, p_chunkSize);
	if (entityCount < 0)
	{
		TT_PANIC("Invalid entity count: %d. Must be at least 0.", entityCount);
		return false;
	}
	
	for (s32 entityIdx = 0; entityIdx < entityCount; ++entityIdx)
	{
		// Early out if no more bytes remain
		if (p_chunkSize == 0)
		{
			TT_PANIC("Trying to read entity %d of %d from entity chunk, "
			         "but no more bytes remain in the chunk.",
			         entityIdx, entityCount);
			return false;
		}
		
		// NOTE: Using be_get instead of get, because the legacy getNarrowString used big-endian internally
		const std::string entityType(bu::be_get<std::string>(p_chunkData, p_chunkSize));
		if (entityType.empty())
		{
			TT_PANIC("Encountered invalid entity type in entities chunk (entity number %d in the chunk). "
			         "Entity type cannot be an empty string.", entityIdx);
			return false;
		}
		
		const s32 entityID = bu::get<s32>(p_chunkData, p_chunkSize);
		if (entityID < 0)
		{
			TT_PANIC("Encountered invalid entity ID in entities chunk (entity number %d of type '%s'): %d",
			         entityIdx, entityType.c_str(), entityID);
			return false;
		}
		
		const tt::math::Vector2 pos(bu::get<tt::math::Vector2>(p_chunkData, p_chunkSize));
		
		const s32 propCount = bu::get<s32>(p_chunkData, p_chunkSize);
		if (propCount < 0)
		{
			TT_PANIC("Encountered invalid entity property count in entities chunk "
			         "(entity number %d of type '%s', ID %d): %d",
			         entityIdx, entityType.c_str(), entityID, propCount);
			return false;
		}
		
		// Verify that this entity ID is unique
		if (getEntityByID(entityID) != 0)
		{
			TT_PANIC("Entity chunk contains more than one entity with ID %d. This is not allowed.",
			         entityID);
			return false;
		}
		
		entity::EntityInstancePtr entity(entity::EntityInstance::create(entityType, entityID, pos));
		
		for (s32 propIdx = 0; propIdx < propCount; ++propIdx)
		{
			// NOTE: Using be_get instead of get, because the legacy getNarrowString used big-endian internally
			const std::string propName(bu::be_get<std::string>(p_chunkData, p_chunkSize));
			if (propName.empty())
			{
				TT_PANIC("Entity with ID %d (type '%s'; number %d in the chunk) has an invalid "
				         "name for property %d. Property names are not allowed to be empty.",
				         entityID, entityType.c_str(), entityIdx, propIdx);
				return false;
			}
			
			// NOTE: Using be_get instead of get, because the legacy getNarrowString used big-endian internally
			const std::string propVal(bu::be_get<std::string>(p_chunkData, p_chunkSize));
			entity->setPropertyValue(propName, propVal);
		}
		
		m_entities.push_back(entity);
	}
	
	return true;
}


bool LevelData::parseChunkNotes(u32 p_version, size_t p_chunkSize, const u8* p_chunkData)
{
	TT_ASSERTMSG(p_version <= g_chunkVersionNotes,
	             "Unsupported notes chunk version: %u. It is too new. "
	             "This code supports notes chunks up to and including version %u.",
	             p_version, g_chunkVersionNotes);
	
	namespace bu = tt::code::bufferutils;
	
	const s32 noteCount = bu::get<s32>(p_chunkData, p_chunkSize);
	if (noteCount < 0)
	{
		TT_PANIC("Invalid note count: %d. Must be at least 0.", noteCount);
		return false;
	}
	
	for (s32 noteIdx = 0; noteIdx < noteCount; ++noteIdx)
	{
		// Early out if no more bytes remain
		if (p_chunkSize == 0)
		{
			TT_PANIC("Trying to read note %d of %d from notes chunk, "
			         "but no more bytes remain in the chunk.",
			         noteIdx, noteCount);
			return false;
		}
		
		const tt::math::Vector2 pos(bu::get<tt::math::Vector2>(p_chunkData, p_chunkSize));
		const real              width  = bu::get<real>(p_chunkData, p_chunkSize);
		const real              height = bu::get<real>(p_chunkData, p_chunkSize);
		if (width <= 0.0f || height <= 0.0f)
		{
			TT_PANIC("Note %d has invalid size: %f x %f (must have positive size).",
			         noteIdx, width, height);
			return false;
		}
		
		const std::wstring text(bu::be_get<std::wstring>(p_chunkData, p_chunkSize));
		addNote(Note::create(tt::math::VectorRect(pos, width, height), text));
	}
	
	return true;
}


bool LevelData::parseChunkPathfinding(u32 p_version, size_t p_chunkSize, const u8* p_chunkData)
{
	TT_ASSERTMSG(p_version <= g_chunkVersionPathfinding,
	             "Unsupported pathfinding chunk version: %u. It is too new. "
	             "This code supports pathfinding chunks up to and including version %u.",
	             p_version, g_chunkVersionPathfinding);
	
	namespace bu = tt::code::bufferutils;
	
	if (p_chunkSize > 0)
	{
		m_pathfindingData = tt::code::AutoGrowBuffer::createPrePopulated(p_chunkData, static_cast<tt::code::Buffer::size_type>(p_chunkSize), 1024);
	}
	else
	{
		m_pathfindingData.reset();
	}
	
	return true;
}


bool LevelData::parseChunkSteamInfo(u32 p_version, size_t p_chunkSize, const u8* p_chunkData)
{
	TT_ASSERTMSG(p_version <= g_chunkVersionSteamInfo,
	             "Unsupported Steam info chunk version: %u. It is too new. "
	             "This code supports Steam info chunks up to and including version %u.",
	             p_version, g_chunkVersionSteamInfo);
	
	namespace bu = tt::code::bufferutils;
	
	m_steamPublishedFileId = bu::get<u64>(p_chunkData, p_chunkSize);
	m_steamOwnerId         = bu::get<u64>(p_chunkData, p_chunkSize);
	
	return true;
}


bool LevelData::saveChunk(u32 p_chunkID, const tt::fs::FilePtr& p_file) const
{
	TT_NULL_ASSERT(p_file);
	if (p_file == 0) return false;
	
	// Create the payload data for the chunk
	tt::code::AutoGrowBufferPtr chunkData;
	switch (p_chunkID)
	{
	case g_chunkIDGlobalInfo:  chunkData = saveChunkGlobalInfo();  break;
	case g_chunkIDTiles:       chunkData = saveChunkTiles();       break;
	case g_chunkIDEntities:    chunkData = saveChunkEntities();    break;
	case g_chunkIDNotes:       chunkData = saveChunkNotes();       break;
	case g_chunkIDPathfinding: chunkData = saveChunkPathfinding(); break;
	case g_chunkIDSteamInfo:   chunkData = saveChunkSteamInfo();   break;
		
	default:
		TT_PANIC("Unsupported chunk ID: 0x%08X", p_chunkID);
		return false;
	}
	
	if (chunkData == 0)
	{
		TT_PANIC("Creating chunk data for chunk 0x%08X failed.", p_chunkID);
		return false;
	}
	
	const u32 chunkSize = chunkData->getUsedSize();
	if (chunkSize == 0)
	{
		TT_PANIC("No chunk data was created for chunk 0x%08X.", p_chunkID);
		return false;
	}
	
	// Calculate the CRC for the chunk data
	tt::math::hash::CRC32 crc;
	for (s32 blockIdx = 0; blockIdx < chunkData->getBlockCount(); ++blockIdx)
	{
		const void*                 blockData = chunkData->getBlock(blockIdx);
		tt::code::Buffer::size_type blockSize = chunkData->getBlockSize(blockIdx);
		crc.update(blockData, static_cast<size_t>(blockSize));
	}
	
	const u32 chunkCRC = crc.getHash();
	
	// Write the chunk to file
	// - Chunk marker
	if (tt::fs::writeInteger(p_file, g_levelFormatChunkMarker) == false)
	{
		TT_PANIC("Writing chunk marker for chunk 0x%08X failed.", p_chunkID);
		return false;
	}
	
	// - Chunk size
	if (tt::fs::writeInteger(p_file, chunkSize) == false)
	{
		TT_PANIC("Writing chunk size (%u) for chunk 0x%08X failed.", chunkSize, p_chunkID);
		return false;
	}
	
	// - Chunk CRC
	if (tt::fs::writeInteger(p_file, chunkCRC) == false)
	{
		TT_PANIC("Writing chunk CRC (0x%08X) for chunk 0x%08X failed.", chunkCRC, p_chunkID);
		return false;
	}
	
	// - Chunk data (also contains chunk ID and version)
	for (s32 blockIdx = 0; blockIdx < chunkData->getBlockCount(); ++blockIdx)
	{
		const void*       blockData = chunkData->getBlock(blockIdx);
		tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(chunkData->getBlockSize(blockIdx));
		
		if (p_file->write(blockData, blockSize) != blockSize)
		{
			TT_PANIC("Writing chunk data for chunk 0x%08X failed.", p_chunkID);
			return false;
		}
	}
	
	return true;
}


tt::code::AutoGrowBufferPtr LevelData::saveChunkGlobalInfo() const
{
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(256, 128));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Write chunk ID and version
	bu::put(g_chunkIDGlobalInfo,      &context);  // Chunk ID
	bu::put(g_chunkVersionGlobalInfo, &context);  // Chunk version
	
	// Write chunk data
	// NOTE: Using be_put instead of put, because the legacy putWideString used big-endian internally
	// NOTE: Write dummy string; as name is unused
	bu::be_put(std::wstring(), &context);
	bu::put   (m_width,    &context);
	bu::put   (m_height,   &context);
	bu::put   (m_startPos, &context);
	bu::put   (std::string(getThemeTypeName(m_levelTheme)), &context);
	bu::put   (std::string(getDefaultMission()), &context);
	
	// For chunk version 3 (and up): write the theme colors
	// (version 4 and up has theme colors per skin config type)
	bu::put(s32(skin::SkinConfigType_Count), &context);
	for (s32 skinIndex = 0; skinIndex < skin::SkinConfigType_Count; ++skinIndex)
	{
		const skin::SkinConfigType config = static_cast<skin::SkinConfigType>(skinIndex);
		bu::put(std::string(skin::getSkinConfigTypeName(config)), &context);
		
		bu::put(s32(ThemeType_Count), &context);
		for (s32 themeIndex = 0; themeIndex < ThemeType_Count; ++themeIndex)
		{
			const ThemeType theme = static_cast<ThemeType>(themeIndex);
			
			// Save colors by theme name, to try to keep dependency on the values of this enum to a minimum
			bu::put(std::string(getThemeTypeName(theme)), &context);
			bu::put(m_themeColor[skinIndex][themeIndex],  &context);
		}
	}
	
	bu::put(m_levelBackground, &context);
	
	context.flush();
	
	return buffer;
}


tt::code::AutoGrowBufferPtr LevelData::saveChunkTiles() const
{
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(2048, 2048));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Write chunk ID and version
	bu::put(g_chunkIDTiles,      &context);  // Chunk ID
	bu::put(g_chunkVersionTiles, &context);  // Chunk version
	
	// Write chunk data
	// - Layer count (game supports only one at the moment)
	bu::put(s32(1), &context);
	
	// - For the only layer, the width, height and tile data
	bu::put(m_attributeLayer->getWidth(),  &context);
	bu::put(m_attributeLayer->getHeight(), &context);
	bu::put(m_attributeLayer->getRawData(), m_attributeLayer->getLength(), &context);
	
	context.flush();
	
	return buffer;
}


tt::code::AutoGrowBufferPtr LevelData::saveChunkEntities() const
{
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(2048, 2048));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Write chunk ID and version
	bu::put(g_chunkIDEntities,      &context);  // Chunk ID
	bu::put(g_chunkVersionEntities, &context);  // Chunk version
	
	// Write chunk data
	// - Entity count
	bu::put(getEntityCount(), &context);
	
	for (entity::EntityInstances::const_iterator entityIt = m_entities.begin();
	     entityIt != m_entities.end(); ++entityIt)
	{
		const entity::EntityInstancePtr& entity(*entityIt);
		
		// NOTE: Using be_put instead of put, because the legacy putNarrowString used big-endian internally
		bu::be_put(entity->getType(),     &context);
		bu::put   (entity->getID(),       &context);
		bu::put   (entity->getPosition(), &context);
		
		const entity::EntityInstance::Properties& entityProps(entity->getProperties());
		
		bu::put(static_cast<s32>(entityProps.size()), &context);  // Property count
		
		for (entity::EntityInstance::Properties::const_iterator propIt = entityProps.begin();
		     propIt != entityProps.end(); ++propIt)
		{
			// NOTE: Using be_put instead of put, because the legacy putNarrowString used big-endian internally
			bu::be_put((*propIt).first,  &context);  // Property name
			bu::be_put((*propIt).second, &context);  // Property value
		}
	}
	
	context.flush();
	
	return buffer;
}


tt::code::AutoGrowBufferPtr LevelData::saveChunkNotes() const
{
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(1024, 1024));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Write chunk ID and version
	bu::put(g_chunkIDNotes,      &context);  // Chunk ID
	bu::put(g_chunkVersionNotes, &context);  // Chunk version
	
	// Write chunk data
	bu::put(getNoteCount(), &context);  // Note count
	
	for (Notes::const_iterator it = m_notes.begin(); it != m_notes.end(); ++it)
	{
		const NotePtr& note(*it);
		
		bu::put   (note->getWorldRect(), &context);
		// NOTE: Using be_put instead of put, because the legacy putWideString used big-endian internally
		bu::be_put(note->getText(),      &context);
	}
	
	context.flush();
	
	return buffer;
}


tt::code::AutoGrowBufferPtr LevelData::saveChunkPathfinding() const
{
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(4096, 4096));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Write chunk ID and version
	bu::put(g_chunkIDPathfinding,      &context);  // Chunk ID
	bu::put(g_chunkVersionPathfinding, &context);  // Chunk version
	
	if (m_pathfindingData != 0)
	{
		// Save all data blocks
		const s32 blockCount = m_pathfindingData->getBlockCount();
		for (s32 i = 0; i < blockCount; ++i)
		{
			const tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(m_pathfindingData->getBlockSize(i));
			bu::put(reinterpret_cast<const u8*>(m_pathfindingData->getBlock(i)), blockSize, &context);
		}
	}
	
	context.flush();
	
	return buffer;
}


tt::code::AutoGrowBufferPtr LevelData::saveChunkSteamInfo() const
{
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(128, 128));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Write chunk ID and version
	bu::put(g_chunkIDSteamInfo,      &context);  // Chunk ID
	bu::put(g_chunkVersionSteamInfo, &context);  // Chunk version
	
	bu::put(m_steamPublishedFileId, &context);
	bu::put(m_steamOwnerId,         &context);
	
	context.flush();
	
	return buffer;
}


// Namespace end
}
}
