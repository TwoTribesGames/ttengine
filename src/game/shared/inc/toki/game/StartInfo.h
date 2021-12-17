#if !defined(INC_TOKI_GAME_STARTINFO_H)
#define INC_TOKI_GAME_STARTINFO_H


#include <string>

#include <tt/code/fwd.h>
#include <tt/platform/tt_types.h>

namespace toki {
namespace game {

class StartInfo
{
public:
	enum Type
	{
		Type_NormalLevel,
		Type_UserLevel,
		Type_UserRecording,
		
		Type_Count
	};
	
	static inline bool isValidType(Type p_type) { return p_type >= 0 && p_type < Type_Count; }
	
	
	StartInfo();
	
	bool initMission     (const std::string& p_missionID);
	bool setLevel        (const std::string& p_levelName, bool p_validate = true);
	bool setMissionID    (const std::string& p_missionID);
	bool setUserLevel    (const std::string& p_levelPath);
	bool setUserRecording(const std::string& p_recordingPath);
	
	bool setWorkshopLevel(const std::string& p_levelPath, u64 p_workshopFileId);
	
	// For setting up user level info in steps
	void setToUserLevelMode();
	bool setUserLevelName(const std::string& p_name);
	
	inline Type               getType()              const { return m_type;                   }
	inline bool               isUserLevel()          const { return m_type == Type_UserLevel; }
	inline const std::string& getLevelName()         const { return m_levelName;              }
	inline const std::string& getLevelPath()         const { return m_levelPath;              }
	inline const std::string& getLevelFilePath()     const { return m_levelFilePath;          }
	inline const std::string& getMissionID()         const { return m_missionID;              }
	inline const std::string& getRecordingFilePath() const { return m_recordingFilePath;      }
	inline u64                getWorkshopFileID()    const { return m_workshopFileID;         }
	
	inline bool               shouldStartMission() const                { return m_shouldStartMission;      }
	inline void               setShouldStartMission(bool p_shouldStart) { m_shouldStartMission = p_shouldStart; }
	
	void resetToDefaultLevel();
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	bool unserialize(tt::code::BufferReadContext*  p_context);
	
	inline bool operator==(const StartInfo& p_rhs) const
	{
		return m_type              == p_rhs.m_type &&
		       m_levelName         == p_rhs.m_levelName &&
		       m_levelName         == p_rhs.m_levelName &&
		       m_levelFilePath     == p_rhs.m_levelFilePath &&
		       m_levelPath         == p_rhs.m_levelPath &&
		       m_missionID         == p_rhs.m_missionID &&
		       m_recordingFilePath == p_rhs.m_recordingFilePath &&
		       m_workshopFileID    == p_rhs.m_workshopFileID;
	}
	
	inline bool operator!=(const StartInfo& p_rhs) const { return operator==(p_rhs) == false; }
	
private:
	bool validate(bool p_resetIfInvalid);
	void updateLevelFilePath();
	
	
	Type        m_type;
	std::string m_levelName;
	std::string m_levelPath;
	std::string m_levelFilePath;
	bool        m_shouldStartMission;
	std::string m_missionID;
	std::string m_recordingFilePath;
	u64         m_workshopFileID;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_STARTINFO_H)
