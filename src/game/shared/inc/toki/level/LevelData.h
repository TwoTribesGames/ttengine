#if !defined(INC_TOKI_LEVEL_LEVELDATA_H)
#define INC_TOKI_LEVEL_LEVELDATA_H


#include <set>
#include <string>
#include <vector>

#include <tt/code/AutoGrowBuffer.h>
#include <tt/code/Buffer.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/fs/types.h>
#include <tt/math/Point2.h>
#include <tt/math/Rect.h>

#include <toki/level/entity/EntityInstance.h>
#include <toki/level/entity/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>

#include <toki/game/pathfinding/PathMgr.h>


namespace toki {
namespace level {

/*! \brief Manages information about a level. */
class LevelData
{
public:
	~LevelData();
	
	/*! \brief Creates an empty level. */
	static LevelDataPtr create(s32 p_width, s32 p_height);
	
	/*! \brief Loads an entire level from file (binary). */
	static LevelDataPtr loadLevel(const std::string& p_filename, tt::fs::identifier p_fsID = 0);
	
	/*! \brief Loads an entire level from fileptr. p_filename is used for setting m_filename. */
	static LevelDataPtr loadLevel(const tt::fs::FilePtr& p_file, const std::string& p_filename,
	                              bool p_fileContainsOtherData = false);
	
	bool saveAsText(const std::string& p_filename, tt::fs::identifier p_fsID = 0) const;
	bool save(const std::string& p_filename, tt::fs::identifier p_fsID = 0) const;
	bool save(const tt::fs::FilePtr& p_file) const;
	
	/*! \param p_newRect New rectangle for the level, relative to the existing level data. */
	void resizeTo(const tt::math::PointRect& p_newRect);
	
	/*! \brief Swap the contents of this object with another one. */
	void swap(const LevelDataPtr& p_other);
	
	LevelDataPtr clone() const;
	
	s32 getEntityCount() const;
	entity::EntityInstancePtr getEntityByIndex(s32 p_index) const;
	entity::EntityInstancePtr getEntityByID(s32 p_id)       const;
	inline const entity::EntityInstances& getAllEntities() const { return m_entities; }
	entity::EntityInstances getEntitiesInRect(const tt::math::VectorRect& p_worldRect) const;
	s32 createEntityID();
	bool addEntity(const entity::EntityInstancePtr& p_entity);
	bool removeEntity(s32 p_id);
	bool removeEntity(const entity::EntityInstancePtr& p_entity);
	bool hasEntity(s32 p_id) const;
	bool hasEntity(const entity::EntityInstancePtr& p_entity) const;
	
	s32     getNoteCount()       const;
	NotePtr getNote(s32 p_index) const;
	NotePtr findNoteOverlappingPosition(const tt::math::Vector2& p_pos) const;
	Notes   findAllNotesOverlappingPosition(const tt::math::Vector2& p_pos) const;
	void    addNote(const NotePtr& p_note);
	void    removeNote(s32 p_index);
	void    removeNote(const NotePtr& p_note);
	inline const Notes& getAllNotes() const { return m_notes; }
	
	// For editor (not saved in file format):
	void addEntityToSelection(const entity::EntityInstancePtr& p_entity);
	void removeEntityFromSelection(const entity::EntityInstancePtr& p_entity);
	void selectAllEntities();
	void deselectAllEntities();
	bool isEntitySelected(const entity::EntityInstancePtr& p_entity) const;
	void setSelectedEntities(const entity::EntityInstanceSet& p_selection);
	inline const entity::EntityInstanceSet& getSelectedEntities() const { return m_selectedEntities; }
	
	void registerObserver  (const LevelDataObserverWeakPtr& p_observer);
	void unregisterObserver(const LevelDataObserverWeakPtr& p_observer);
	void clearObservers();
	
	/*! \return Rectangle covered by the level data, in tiles. */
	tt::math::PointRect getLevelRect() const;
	
	std::string getAsString() const;
	
	inline const std::string&       getLevelFilename()  const { return m_filename;       }
	inline s32                      getLevelWidth()     const { return m_width;          }  //!< In tiles.
	inline s32                      getLevelHeight()    const { return m_height;         }  //!< In tiles.
	inline const AttributeLayerPtr& getAttributeLayer() const { return m_attributeLayer; }
	
	//! Player start position, in tiles
	inline const tt::math::Point2& getPlayerStartPosition() const { return m_startPos; }
	
	void                      setLevelBackground(const std::string& p_backgroundName);
	inline const std::string& getLevelBackground() const { return m_levelBackground; }
	
	void             setLevelTheme(ThemeType p_theme);
	inline ThemeType getLevelTheme() const { return m_levelTheme; }
	
	inline void setDefaultMission(const std::string& p_defaultMission) { m_levelDefaultMission = p_defaultMission; }
	inline const std::string& getDefaultMission() const { return m_levelDefaultMission; }
	
	void setThemeColor(skin::SkinConfigType                   p_skinConfig,
	                   ThemeType                              p_theme,
	                   const tt::engine::renderer::ColorRGBA& p_color);
	const tt::engine::renderer::ColorRGBA& getThemeColor(
			skin::SkinConfigType p_skinConfig,
			ThemeType            p_theme) const;
	
	// Pathfinding data
	inline void setAgentRadii(const game::pathfinding::PathMgr::AgentRadii& p_agentRadii) { m_agentRadii = p_agentRadii; }
	inline const game::pathfinding::PathMgr::AgentRadii& getAgentRadii() const { return m_agentRadii; }
	inline bool hasPathfindingData() const { return m_pathfindingData != 0; }
	tt::code::AutoGrowBufferPtr createPathfindingData();
	inline void invalidatePathfindingData() { m_pathfindingData.reset(); }
	inline tt::code::AutoGrowBufferPtr getPathfindingData() const { return m_pathfindingData; }
	
	// Steam details:
	
	// The Steam Workshop ID of this level, if it was published to Steam Workshop:
	inline void setPublishedFileId(u64 p_id) { m_steamPublishedFileId = p_id; }
	inline u64  getPublishedFileId() const   { return m_steamPublishedFileId; }
	
	// Steam user ID of the user that created this level
	inline void setOwnerId(u64 p_id) { m_steamOwnerId = p_id; }
	inline u64  getOwnerId() const   { return m_steamOwnerId; }
	
	static bool isValidLevelTheme(ThemeType p_theme);
	
private:
	typedef std::vector<LevelDataObserverWeakPtr> Observers;
	
	
	LevelData();
	LevelData(const LevelData& p_rhs);
	
	void notifyEntitySelectionChanged();
	
	void setLevelThemeToDefault();
	
	/*! \brief Loads theme colors from binary data. Results get stored in m_themeColor
	           for p_skinConfig, if it is valid (otherwise results are ignored). */
	void loadThemeColors(size_t& p_chunkSize, const u8*& p_chunkData,
	                     skin::SkinConfigType p_skinConfig);
	
	bool loadFromFile(const tt::fs::FilePtr& p_file, const std::string& p_filename,
	                  bool p_fileContainsOtherData = false);
	
	bool parseFromString(const std::string& p_levelData);
	
	bool parseChunk(size_t p_chunkSize, const u8* p_chunkData);
	bool parseChunkGlobalInfo (u32 p_version, size_t p_chunkSize, const u8* p_chunkData);
	bool parseChunkTiles      (u32 p_version, size_t p_chunkSize, const u8* p_chunkData);
	bool parseChunkEntities   (u32 p_version, size_t p_chunkSize, const u8* p_chunkData);
	bool parseChunkNotes      (u32 p_version, size_t p_chunkSize, const u8* p_chunkData);
	bool parseChunkPathfinding(u32 p_version, size_t p_chunkSize, const u8* p_chunkData);
	bool parseChunkSteamInfo  (u32 p_version, size_t p_chunkSize, const u8* p_chunkData);
	
	bool                        saveChunk(u32 p_chunkID, const tt::fs::FilePtr& p_file) const;
	tt::code::AutoGrowBufferPtr saveChunkGlobalInfo()  const;
	tt::code::AutoGrowBufferPtr saveChunkTiles()       const;
	tt::code::AutoGrowBufferPtr saveChunkEntities()    const;
	tt::code::AutoGrowBufferPtr saveChunkNotes()       const;
	tt::code::AutoGrowBufferPtr saveChunkPathfinding() const;
	tt::code::AutoGrowBufferPtr saveChunkSteamInfo()   const;
	
	// No assignment
	LevelData& operator=(const LevelData&);
	
	
	std::string        m_filename;        //!< The filename (excl. extension) of the level
	s32                m_width;           //!< The level width, in tiles.
	s32                m_height;          //!< The level height, in tiles.
	AttributeLayerPtr  m_attributeLayer;  //!< The attribute layer.
	
	std::string m_levelBackground;     //!< Predefined background shoebox for the level. Meant for user levels.
	ThemeType   m_levelTheme;          //!< The default theme for this level (can be overridden for individual tiles).
	std::string m_levelDefaultMission; //!< The default mission for this level
	
	// Per-theme vertex colors for the skinning planes
	tt::engine::renderer::ColorRGBA m_themeColor[skin::SkinConfigType_Count][ThemeType_Count];
	
	tt::math::Point2                       m_startPos;  //!< Player start position, in tiles.
	entity::EntityInstances                m_entities;
	Notes                                  m_notes;
	game::pathfinding::PathMgr::AgentRadii m_agentRadii;
	tt::code::AutoGrowBufferPtr            m_pathfindingData;
	u64                                    m_steamPublishedFileId; // Steam Workshop ID of this level, if it was published to Steam Workshop
	u64                                    m_steamOwnerId;         // Steam user ID of the user that created this level
	
	// For editor (not saved in file format):
	entity::EntityInstanceSet m_selectedEntities;
	Observers                 m_observers;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_LEVELDATA_H)
