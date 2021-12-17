#include <json/json.h>

#include <tt/app/Application.h>
#include <tt/code/bufferutils.h>
#include <tt/fs/fs.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>


#include <toki/cfg.h>
#include <toki/game/StartInfo.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

StartInfo::StartInfo()
:
m_type(Type_NormalLevel),
m_levelName(),
m_levelPath(),
m_levelFilePath(),
m_shouldStartMission(false),
m_missionID(),
m_recordingFilePath(),
m_workshopFileID(0)
{
}


bool StartInfo::initMission(const std::string& p_missionID)
{
	if (p_missionID.empty())
	{
		TT_PANIC("Empty mission ID specified.");
		return false;
	}
	
	m_missionID = p_missionID;
	m_shouldStartMission = true;
	return true;
}


bool StartInfo::setLevel(const std::string& p_levelName, bool p_validate)
{
	if (p_levelName.empty())
	{
		TT_PANIC("Empty level name specified. Resetting to default level.");
		resetToDefaultLevel();
		return false;
	}
	m_levelName     = p_levelName;
	m_levelPath     = "levels/";
	m_recordingFilePath.clear();
	m_type          = Type_NormalLevel;
	m_workshopFileID = 0;
	updateLevelFilePath();
	
	return p_validate ? validate(true) : true;
}


bool StartInfo::setMissionID(const std::string& p_missionID)
{
	m_missionID = p_missionID;
	return true;
}


bool StartInfo::setUserLevel(const std::string& p_levelFilePath)
{
	// FIXME: It is nice to be able to startup user generated levels, however we need to make sure that people
	// don't startup official Toki Tori 2 levels this way. Perhaps encode them or add a special flag?
	if (p_levelFilePath.empty())
	{
		TT_PANIC("Empty path to level file specified. Resetting to default level.");
		resetToDefaultLevel();
		return false;
	}
	
	m_levelName     = tt::fs::utils::getFileTitle(p_levelFilePath);
	m_levelPath     = tt::fs::utils::getParentDirectory(p_levelFilePath);
	m_levelFilePath = p_levelFilePath;
	m_recordingFilePath.clear();
	m_type          = Type_UserLevel;
	m_workshopFileID = 0;
	
	return validate(true);
}


bool StartInfo::setWorkshopLevel(const std::string& p_levelFilePath, u64 p_workshopFileId)
{
	if (setUserLevel(p_levelFilePath))
	{
		m_workshopFileID = p_workshopFileId;
		return true;
	}
	else
	{
		return false;
	}
}


bool StartInfo::setUserRecording(const std::string& p_recordingPath)
{
	if (p_recordingPath.empty())
	{
		TT_PANIC("Empty path to recording file specified. Resetting to default level.");
		resetToDefaultLevel();
		return false;
	}
	
	m_levelName.clear();
	m_levelPath = "levels/";
	m_levelFilePath.clear();
	m_recordingFilePath = p_recordingPath;
	m_type = Type_UserRecording;
	m_workshopFileID = 0;
	
	return validate(true);
}


void StartInfo::setToUserLevelMode()
{
	m_levelPath = "userlevels/";
	m_type      = Type_UserLevel;
	m_recordingFilePath.clear();
	m_workshopFileID = 0;
	updateLevelFilePath();
}


bool StartInfo::setUserLevelName(const std::string& p_name)
{
	// NOTE: Name is allowed to be empty (indicates "no level")
	m_levelName = p_name;
	m_type      = Type_UserLevel;
	m_recordingFilePath.clear();
	updateLevelFilePath();
	
	return p_name.empty() ? true : validate(false);
}


void StartInfo::resetToDefaultLevel()
{
	// Don't call setLevel, possible infinite recursion
	m_levelName     = cfg()->getStringDirect("toki.startup.default_level");
	m_levelPath     = "levels/";
	m_type          = Type_NormalLevel;
	m_workshopFileID = 0;
	updateLevelFilePath();
	
	validate(false);
}


void StartInfo::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u8, Type>(m_type, p_context);
	bu::put(m_shouldStartMission, p_context);
	bu::put(m_missionID, p_context);
	
	// Store filepath (userlevel) or levelname (non-userlevel)
	switch (m_type)
	{
	case Type_NormalLevel:
		bu::put(m_levelName, p_context);
		break;
		
	case Type_UserLevel:
		bu::put(m_levelPath,      p_context);
		bu::put(m_levelName,      p_context);
		bu::put(m_workshopFileID, p_context);
		break;
		
	case Type_UserRecording:
		bu::put(m_recordingFilePath, p_context);
		break;
		
	default:
		TT_PANIC("Unhandled startinfo type %d", m_type);
		break;
	}
}


bool StartInfo::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	if (p_context == 0)
	{
		return false;
	}
	
	namespace bu = tt::code::bufferutils;
	
	const Type type = bu::getEnum<u8, Type>(p_context);
	if (isValidType(type) == false)
	{
		return false;
	}
	m_type = type;
	
	m_shouldStartMission = bu::get<bool>(p_context);
	m_missionID          = bu::get<std::string>(p_context);
	
	// Load filepath (userlevel) or levelname (non-userlevel)
	bool loadOk = false;
	const std::string data = bu::get<std::string>(p_context);
	switch (m_type)
	{
	case Type_NormalLevel:
		loadOk = setLevel(data);
		break;
		
	case Type_UserLevel:
		m_levelPath      = data;
		loadOk           = setUserLevelName(bu::get<std::string>(p_context));
		m_workshopFileID = bu::get<u64>(p_context);
		break;
		
	case Type_UserRecording:
		loadOk = setUserRecording(data);
		break;
		
	default:
		TT_PANIC("Unhandled startinfo type %d", m_type);
		break;
	}
	
	return loadOk;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool StartInfo::validate(bool p_resetIfInvalid)
{
	const std::string startupFile = (m_type == Type_UserRecording) ? m_recordingFilePath : m_levelFilePath;
	
	if (tt::fs::fileExists(startupFile) == false)
	{
		TT_PANIC("Startup file '%s' (from command line) does not exist.\n"
		         "Falling back to default level '%s' (from config).",
		         startupFile.c_str(), cfg()->getStringDirect("toki.startup.default_level"));
		
		if (p_resetIfInvalid)
		{
			resetToDefaultLevel();
		}
		return false;
	}
	
	return true;
}


void StartInfo::updateLevelFilePath()
{
	if (isUserLevel() && m_levelName.empty())
	{
		// Can't provide a full level file path, because we don't have a level name
		m_levelFilePath = m_levelPath;
	}
	else
	{
		m_levelFilePath = m_levelPath + m_levelName + ".ttlvl";
	}
}

// Namespace end
}
}
