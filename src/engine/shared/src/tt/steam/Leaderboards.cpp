#if !defined(TT_PLATFORM_OSX_IPHONE)  // Steam support works on Mac OS X, but not on iOS

#include <json/json.h>

#include <tt/doc/json/json_util.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/Leaderboards.h>
#include <tt/thread/CriticalSection.h>

#include <algorithm>

namespace tt {
namespace steam {

Leaderboards* Leaderboards::ms_instance = 0;

// Save file metadata
const s32         g_saveSignatureLength = 7;
const char        g_saveSignature[g_saveSignatureLength] = { 'T', 'T', 'L', 'E', 'A', 'D',  0 };
const s32         g_saveVersion         = 1;

// Developer Steam IDs
const u64 g_developerIDs[] =
{
	76561198012790042LL, // Martijn
	76561198012790024LL, // Collin
	76561198001732415LL, // Meinte
	76561198118031397LL, // Matthijs
	76561198017016344LL, // SonicPicnic
	76561198007753844LL, // Niels
	76561197999317911LL, // Tristan
	76561197975911898LL, // Jay
	76561197971178169LL, // Paul
	76561197970252088LL, // Hessel
	76561197993359390LL  // Eelke
};

//--------------------------------------------------------------------------------------------------
// Public member functions

bool Leaderboards::createInstance(const std::string& p_saveFileName, tt::fs::identifier p_saveFileFSID,
                                  bool p_createOnNotFound, ELeaderboardSortMethod p_createSortMethod, ELeaderboardDisplayType p_createDisplayType)
{
	if (ms_instance == 0)
	{
		// Verify that steam is initialized
		if (SteamUser() == 0)
		{
			TT_WARN("Steam has not been initialized, unable to initialize Leaderboards");
			return false;
		}
		if (SteamUserStats() == 0)
		{
			TT_PANIC("Steam user statistics interface has not been initialized.");
			return false;
		}
		if (SteamUser()->BLoggedOn() == false)
		{
			// -> User is playing in offline mode, leaderboard scores cannot be uploaded
			TT_WARN("Steam user is not logged on, cannot upload scores.");
			return false;
		}
		
#if defined(TT_BUILD_FINAL)
		// Prohibit creation of leaderboards in final builds
		p_createOnNotFound = false;
#endif
		ms_instance = new Leaderboards(p_saveFileName, p_saveFileFSID, 
			p_createOnNotFound, p_createSortMethod, p_createDisplayType);
	}
	else
	{
		TT_PANIC("Instance already exists");
	}
	return hasInstance();
}


void Leaderboards::destroyInstance()
{
	if (ms_instance == 0)
	{
		TT_PANIC("Instance already deleted");
		return;
	}
	
	delete ms_instance;
	ms_instance = 0;
}


s32 Leaderboards::addUploadRequest(const tt::score::UploadRequest& p_request)
{
	tt::thread::CriticalSection critSec(&m_requestsLock);
	
	++m_currentRequestID;
	m_uploadRequests.push_back(std::make_pair(m_currentRequestID, p_request));
	
	return m_currentRequestID;
}


s32 Leaderboards::addDownloadRequest(const tt::score::DownloadRequest& p_request)
{
	tt::thread::CriticalSection critSec(&m_requestsLock);
	
	++m_currentRequestID;
	m_downloadRequests.push_back(std::make_pair(m_currentRequestID, p_request));
	
	return m_currentRequestID;
}


void Leaderboards::commitRequests()
{
	// Do nothing
}


void Leaderboards::loadQueue()
{
	if (m_saveFileName.empty())
	{
		TT_PANIC("Cannot load leaderboard with empty filename");
		return;
	}
	
	if (tt::fs::fileExists(m_saveFileName, m_saveFileFSID) == false)
	{
		// No saved data exists; do nothing
		return;
	}
	
	tt::fs::FilePtr file = tt::fs::open(m_saveFileName, tt::fs::OpenMode_Read, m_saveFileFSID);
	if (file == 0 || file->getLength() == 0)
	{
		// File is either empty or cannot be opened; do nothing
		return;
	}
	
	// Load the save data as binary JSON
	Json::Value saveRoot;
	if (tt::doc::json::loadBinaryJson(&saveRoot, file, g_saveSignature,
	                                  g_saveSignatureLength, g_saveVersion) == false)
	{
		TT_PANIC("Loading leaderboard data binary JSON from file '%s' failed.", m_saveFileName.c_str());
		return;
	}
	
	for (Json::Value::iterator it = saveRoot.begin(); it != saveRoot.end(); ++it)
	{
		Json::Value& entry(*it);
		if (entry.isObject() == false)
		{
			TT_PANIC("Non-object found in leaderboard JSON");
			return;
		}
		
		const Json::Value::Members members(entry.getMemberNames());
		
		if (entry.size() != 1)
		{
			TT_PANIC("Object with more than 1 member found in leaderboard JSON");
			return;
		}
		
		std::string name(members[0]);
		
		if (entry[name].isConvertibleTo(Json::intValue) == false)
		{
			TT_PANIC("Object has non-Integer value in leaderboard JSON");
			return;
		}
		s32 score = entry[name].asInt();
		
		// Finally push this entry to the leaderboard queue
		addUploadRequest(tt::score::UploadRequest(name, score));
	}
}


void Leaderboards::saveQueue() const
{
	if (m_saveFileName.empty())
	{
		TT_PANIC("Cannot save leaderboard with empty filename");
		return;
	}
	
	// Prepare the JSON data to save
	Json::Value saveRoot(Json::arrayValue);
	for (tt::score::UploadRequests::const_iterator it = m_uploadRequests.begin(); 
		it != m_uploadRequests.end(); ++it)
	{
		Json::Value entry(Json::objectValue);
		entry[it->second.leaderboard] = it->second.score;
			
		saveRoot.append(entry);
	}
	
	tt::fs::FilePtr file(tt::fs::open(m_saveFileName, tt::fs::OpenMode_Write, m_saveFileFSID));
	
	tt::doc::json::saveBinaryJson(saveRoot, file, g_saveSignature, g_saveSignatureLength, g_saveVersion);

#if !defined(TT_BUILD_FINAL)
	//* DEBUG: Also write a human readable copy to the assets root
	{
		tt::fs::FilePtr debugFile(tt::fs::open("debug_autogen_leaderboardqueue.json", tt::fs::OpenMode_Write));
		if (debugFile != 0)
		{
			const std::string json(Json::StyledWriter().write(saveRoot));
			debugFile->write(json.c_str(), static_cast<tt::fs::size_type>(json.length()));
		}
	}
	//*/
#endif
}


void Leaderboards::update()
{
	tt::thread::CriticalSection critSec(&m_requestsLock);
	
	switch (m_communicationState)
	{
	case CommunicationState_Idle:
		TT_ASSERT(m_requestState == RequestState_Idle);
		
		// Not communication, determine requeststate. Uploads are ALWAYS done before downloads
		if      (m_uploadRequests.empty()   == false) m_requestState = RequestState_ProcessingUpload;
		else if (m_downloadRequests.empty() == false) m_requestState = RequestState_ProcessingDownload;
		
		if (m_requestState != RequestState_Idle)
		{
			TT_ASSERT(m_currentLeaderboard == 0);
			
			// Setup Steam call to find leaderboard
			m_communicationState = CommunicationState_FindLeaderboard;
			SteamAPICall_t handle = SteamUserStats()->FindLeaderboard(getRequestedLeaderboard().c_str());
			m_callResultFindLeaderboard.Set(handle, this, &Leaderboards::onFindLeaderboard);
		}
		break;
		
#if !defined(TT_BUILD_FINAL)
	case CommunicationState_SetupLeaderboard:
		TT_ASSERT(m_requestState == RequestState_ProcessingUpload ||
		          m_requestState == RequestState_ProcessingDownload);
		
		if (m_uploadRequests.empty() == false)
		{
			TT_ASSERT(m_currentLeaderboard == 0);
			
			// Setup Steam call to create leaderboard
			m_communicationState = CommunicationState_CreateLeaderboard;
			SteamAPICall_t handle = SteamUserStats()->FindOrCreateLeaderboard(
					getRequestedLeaderboard().c_str(), m_createSortMethod, m_createDisplayType);
			m_callResultFindLeaderboard.Set(handle, this, &Leaderboards::onFindLeaderboard);
		}
		break;
#endif
		
	case CommunicationState_UploadScoreSucceeded:
		TT_ASSERT(m_requestState == RequestState_ProcessingUpload);
		
		// Succes, remove entry from queue
		m_uploadRequests.erase(m_uploadRequests.begin());
		
		// Reset handle to leaderboard
		m_currentLeaderboard = 0;
		
		// Move to idle state
		m_communicationState = CommunicationState_Idle;
		m_requestState       = RequestState_Idle;
		break;
	
	case CommunicationState_DownloadScoresSucceeded:
		TT_ASSERT(m_requestState == RequestState_ProcessingDownload);
		
		// Succes, remove entry from queue
		m_downloadRequests.erase(m_downloadRequests.begin());
		
		// Reset handle to leaderboard
		m_currentLeaderboard = 0;
		
		// Move to idle state
		m_communicationState = CommunicationState_Idle;
		m_requestState       = RequestState_Idle;
		break;
		
	case CommunicationState_FindLeaderboard:
	case CommunicationState_CreateLeaderboard:
	case CommunicationState_UploadScore:
	case CommunicationState_DownloadScores:
		// Do nothing; these states are handled by the Steam callbacks
		break;
		
	default:
		TT_PANIC("Unhandled communication state '%d'", m_communicationState);
		break;
	}
}


const std::string& Leaderboards::getRequestedLeaderboard()
{
	tt::thread::CriticalSection critSec(&m_requestsLock);
	
	static std::string emptyString;
	switch (m_requestState)
	{
	case RequestState_ProcessingUpload:
		if (m_uploadRequests.empty())
		{
			TT_PANIC("No upload requests. Cannot retrieve leaderboard.");
			return emptyString;
		}
		return m_uploadRequests.front().second.leaderboard;
		
	case RequestState_ProcessingDownload:
		if (m_downloadRequests.empty())
		{
			TT_PANIC("No download requests. Cannot retrieve leaderboard.");
			return emptyString;
		}
		return m_downloadRequests.front().second.leaderboard;
		
	default:
		break;
	}
	
	TT_PANIC("getRequestedLeaderboard should only be requested in " 
	         "RequestState_ProcessingUpload or RequestState_ProcessingDownload. m_requestState is %d",
	         m_requestState);
	
	return emptyString;
}


//--------------------------------------------------------------------------------------------------
// Callbacks

void Leaderboards::onFindLeaderboard(LeaderboardFindResult_t* p_result, bool p_IOFailure)
{
	tt::thread::CriticalSection critSec(&m_requestsLock);
	
	TT_ASSERT(m_communicationState == CommunicationState_FindLeaderboard || 
		m_communicationState == CommunicationState_CreateLeaderboard);
	
	TT_ASSERT(m_requestState == RequestState_ProcessingUpload || m_requestState == RequestState_ProcessingDownload);
	
	const std::string& leaderboard = getRequestedLeaderboard();
	
	// Handle communication problems
	if (p_result == 0 || p_IOFailure)
	{
		TT_PANIC("Communication problem with finding leaderboard '%s'", leaderboard.c_str());
		
		// Revert back to idle state
		m_communicationState = CommunicationState_Idle;
		m_requestState       = RequestState_Idle;
		return;
	}
	
	// Handle leaderboard not found
	if (p_result->m_bLeaderboardFound == false)
	{
#if !defined(TT_BUILD_FINAL)
		if (m_createOnNotFound && m_communicationState == CommunicationState_FindLeaderboard)
		{
			TT_PANIC("Leaderboard '%s' could not be found. Autocreating it", leaderboard.c_str());
			
			m_communicationState = CommunicationState_SetupLeaderboard;
			return;
		}
		
		TT_PANIC("Leaderboard '%s' could not be found/created", leaderboard.c_str());
#endif
		
		// Remove invalid entry from queue
		if( m_requestState == RequestState_ProcessingUpload)
		{
			// Fire callback to indicate failure
			onUploadScore(0, true);
		}
		else
		{
			// Fire callback to indicate failure
			onDownloadScores(0, true);
		}
		
		return;
	}
	
	m_currentLeaderboard = p_result->m_hSteamLeaderboard;
	
	// Move to uploading state
	if (m_requestState == RequestState_ProcessingUpload)
	{
		TT_ASSERT(m_uploadRequests.empty() == false);
		if (m_uploadRequests.empty() == false)
		{
			// Move to next state
			m_communicationState = CommunicationState_UploadScore;
			
			// Setup Steam call to upload score
			const s32 score = m_uploadRequests.front().second.score;
			SteamAPICall_t handle = SteamUserStats()->UploadLeaderboardScore(m_currentLeaderboard,
				k_ELeaderboardUploadScoreMethodKeepBest, score, 0, 0);
			m_callResultUploadScore.Set(handle, this, &Leaderboards::onUploadScore);
			return;
		}
	}
	// Move to downloading state
	else if (m_requestState == RequestState_ProcessingDownload)
	{
		TT_ASSERT(m_downloadRequests.empty() == false);
		if (m_downloadRequests.empty() == false)
		{
			// Move to next state
			m_communicationState = CommunicationState_DownloadScores;
			
			const tt::score::DownloadRequest& request(m_downloadRequests.front().second);
			
			// Convert range to steam values
			ELeaderboardDataRequest rangeType = k_ELeaderboardDataRequestGlobal;
			switch (request.rangeType)
			{
			case tt::score::DownloadRequestRangeType_Global:            rangeType = k_ELeaderboardDataRequestGlobal; break;
			case tt::score::DownloadRequestRangeType_GlobalAroundUser:  rangeType = k_ELeaderboardDataRequestGlobalAroundUser; break;
			case tt::score::DownloadRequestRangeType_Friends:           rangeType = k_ELeaderboardDataRequestFriends; break;
			case tt::score::DownloadRequestRangeType_FriendsAroundUser: rangeType = k_ELeaderboardDataRequestFriends; break; // handle manually
			default:
				TT_PANIC("Unhandled range type '%d'", request.rangeType);
				break;
			}
			
			// Setup Steam call to upload score
			SteamAPICall_t handle = SteamUserStats()->DownloadLeaderboardEntries(m_currentLeaderboard,
				rangeType, request.rangeMin, request.rangeMax);
			m_callResultDownloadScores.Set(handle, this, &Leaderboards::onDownloadScores);
			return;
		}
	}
	
	TT_PANIC("Failed to continue after finding leaderboard. Defaulting to idle state.");
	// Revert back to idle state
	m_communicationState = CommunicationState_Idle;
	m_requestState       = RequestState_Idle;
}


void Leaderboards::onUploadScore(LeaderboardScoreUploaded_t* p_result, bool p_IOFailure)
{
	tt::thread::CriticalSection critSec(&m_requestsLock);
	
	TT_ASSERT(m_communicationState == CommunicationState_UploadScore);
	
	if (m_uploadRequests.empty())
	{
		TT_PANIC("Empty upload request queue. This shouldn't happen.");
		
		// Revert back to idle state
		m_communicationState = CommunicationState_Idle;
		m_requestState       = RequestState_Idle;
		return;
	}
	
	if (p_result == 0 || p_result->m_bSuccess == false || p_IOFailure)
	{
		const tt::score::UploadRequest& request = m_uploadRequests.front().second;
		TT_PANIC("Score '%d' could not be uploaded to leaderboard '%s'.\n"
				"p_result == 0: %d, p_result->m_bSuccess == false: %d, p_IOFailure: %d\n",
				request.score, request.leaderboard.c_str(),
				p_result == 0, p_result == 0 || p_result->m_bSuccess == false, p_IOFailure);
		
		// FIXME: Make sure that this entry is moved to the back of some "failed" queue, to prevent endless loops of failing entries
		//m_uploadRequests.push_back(m_uploadRequests.front());
		m_uploadRequests.erase(m_uploadRequests.begin());
		
		// Revert back to idle state
		m_communicationState = CommunicationState_Idle;
		m_requestState       = RequestState_Idle;
		
		// Reset handle to leaderboard
		m_currentLeaderboard = 0;
		return;
	}
	
	m_communicationState = CommunicationState_UploadScoreSucceeded;
}


void Leaderboards::onDownloadScores(LeaderboardScoresDownloaded_t* p_result, bool p_IOFailure)
{
	TT_ASSERT(m_communicationState == CommunicationState_DownloadScores);
	
	if (m_downloadRequests.empty())
	{
		TT_PANIC("Empty download request queue. This shouldn't happen.");
		
		// Revert back to idle state
		m_communicationState = CommunicationState_Idle;
		m_requestState       = RequestState_Idle;
		return;
	}
	
	tt::score::DownloadRequestResults results;
	const tt::score::DownloadRequest& request = m_downloadRequests.front().second;
	const s32              requestID = m_downloadRequests.front().first;
	
	results.leaderboard = request.leaderboard;
	results.userData    = request.userData;
	
	if (p_IOFailure)
	{
		TT_PANIC("Could not fetch results for leaderboard '%s'", request.leaderboard.c_str());
		
		// Fire callback before the erase
		results.status = tt::score::DownloadRequestStatus_Failed;
		request.callback(requestID, results);
		
		m_downloadRequests.erase(m_downloadRequests.begin());
		
		// Revert back to idle state
		m_communicationState = CommunicationState_Idle;
		m_requestState       = RequestState_Idle;
		return;
	}
	
	// Fill results structure
	// https://partner.steamgames.com/documentation/leaderboards
	results.currentUserIndex        = -1;
	results.rangeType               = request.rangeType;
	const CSteamID currentSteamID(SteamUser()->GetSteamID());
	
	if (request.rangeType == tt::score::DownloadRequestRangeType_Global ||
	    request.rangeType == tt::score::DownloadRequestRangeType_GlobalAroundUser)
	{
		// Total entries is based on global count
		results.leaderboardTotalEntries = SteamUserStats()->GetLeaderboardEntryCount(m_currentLeaderboard);
		
		for (s32 i = 0; i < p_result->m_cEntryCount; ++i)
		{
			LeaderboardEntry_t entry;
			SteamUserStats()->GetDownloadedLeaderboardEntry(p_result->m_hSteamLeaderboardEntries, i, &entry, 0, 0);
			const std::string username(SteamFriends()->GetFriendPersonaName(entry.m_steamIDUser));
			const u64 id = entry.m_steamIDUser.ConvertToUint64();
			auto b = std::begin(g_developerIDs),e = std::end(g_developerIDs);
			const bool isDeveloper = std::find(b, e, id) != e;
			results.entries.push_back(tt::score::DownloadRequestResultEntry(username, entry.m_nGlobalRank, entry.m_nScore, isDeveloper));
		
			// Check if entry is our user
			if (currentSteamID == entry.m_steamIDUser)
			{
				results.currentUserIndex = i;
			}
		}
	}
	else
	{
		// Total entries is based on friend count
		results.leaderboardTotalEntries = p_result->m_cEntryCount;
		
		// For some reason, Steam returns ALL friends records and ignores the min/max range (to display "near" friends)
		// Hence we have to do this manually.
		tt::score::DownloadRequestResultEntries entries;
		for (s32 i = 0; i < p_result->m_cEntryCount; ++i)
		{
			LeaderboardEntry_t entry;
			SteamUserStats()->GetDownloadedLeaderboardEntry(p_result->m_hSteamLeaderboardEntries, i, &entry, 0, 0);
			const std::string username(SteamFriends()->GetFriendPersonaName(entry.m_steamIDUser));
			const u64 id = entry.m_steamIDUser.ConvertToUint64();
			const bool isDeveloper = std::find(std::begin(g_developerIDs), std::end(g_developerIDs), id) != std::end(g_developerIDs);
			entries.push_back(tt::score::DownloadRequestResultEntry(username, entry.m_nGlobalRank, entry.m_nScore, isDeveloper));
			
			// Check if entry is our user
			if (currentSteamID == entry.m_steamIDUser)
			{
				results.currentUserIndex = i;
			}
		}
		
		// entries now contains ALL friends. Try to chop off the ones outside the range.
		// If there are not enough entries in the leaderboard before or after the user's entry,
		// Steam will adjust the range to try to return the number of entries requested.
		// For example, if the user is #1 on the leaderboard, start is set to -2, and end is set to 2,
		// Steam will return the first 5 entries in the leaderboard.
		s32 rangeMin = 0;
		s32 rangeMax = 0;
		if (request.rangeType == tt::score::DownloadRequestRangeType_Friends)
		{
			// The start and end parameters control the requested range. For example, you can display the top 10
			// on a leaderboard for your game by setting start to 1 and end to 10.
			rangeMin = request.rangeMin - 1;
			rangeMax = request.rangeMax;
		}
		else if (request.rangeType == tt::score::DownloadRequestRangeType_FriendsAroundUser)
		{
			if (results.currentUserIndex == -1)
			{
				rangeMin = 0;
				rangeMax = request.rangeMax - request.rangeMin + 1;
			}
			else
			{
				rangeMin = results.currentUserIndex + request.rangeMin;
				rangeMax = results.currentUserIndex + request.rangeMax + 1;
			}
		}
		
		score::fitRequestRange(rangeMin, rangeMax, 0, static_cast<s32>(entries.size()));
		
		// Some OOB safety checks; just to be sure
		if (rangeMin < 0)
		{
			TT_PANIC("Invalid rangeMin. Problem with score::fitRequestRange");
			rangeMin = 0;
		}
		if (rangeMax > static_cast<s32>(entries.size()))
		{
			TT_PANIC("Invalid rangeMax. Problem with score::fitRequestRange");
			rangeMax = static_cast<s32>(entries.size());
		}
		
		for (s32 i = rangeMin; i < rangeMax; ++i)
		{
			results.entries.push_back(entries[i]);
		}
		
		// Offset new index
		results.currentUserIndex -= rangeMin;
	}
	
	results.status = tt::score::DownloadRequestStatus_Success;

	request.callback(requestID, results);

	m_communicationState = CommunicationState_DownloadScoresSucceeded;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Leaderboards::Leaderboards(const std::string& p_saveFileName, tt::fs::identifier p_saveFileFSID,
	bool p_createOnNotFound, ELeaderboardSortMethod p_createSortMethod, ELeaderboardDisplayType p_createDisplayType)
:
m_callResultFindLeaderboard(),
m_callResultUploadScore(),
m_callResultDownloadScores(),
m_uploadRequests(),
m_downloadRequests(),
m_requestsLock(),
m_requestState(RequestState_Idle),
m_currentRequestID(0),
m_communicationState(CommunicationState_Idle),
m_currentLeaderboard(0),
m_saveFileName(p_saveFileName),
m_saveFileFSID(p_saveFileFSID)
#if !defined(TT_BUILD_FINAL)
,
m_createOnNotFound(p_createOnNotFound),
m_createSortMethod(p_createSortMethod), 
m_createDisplayType(p_createDisplayType)
#endif
{
#if defined(TT_BUILD_FINAL)
	(void)p_createOnNotFound;
	(void)p_createSortMethod;
	(void)p_createDisplayType;
#endif

	if (m_saveFileName.empty() == false)
	{
		loadQueue();
	}
	else
	{
		TT_WARN("No filename specified for saving leaderboard queue. No queue will be saved!");
	}
}


Leaderboards::~Leaderboards()
{
	if (m_saveFileName.empty() == false)
	{
		saveQueue();
	}
}

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)
