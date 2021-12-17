#if !defined(INC_STEAM_LEADERBOARDS_H)
#define INC_STEAM_LEADERBOARDS_H

#if !defined(TT_PLATFORM_OSX_IPHONE)  // Steam support works on Mac OS X, but not on iOS

#include <string>
#include <vector>

#include <steam/steam_api.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

#include <tt/score/Score.h>
#include <tt/thread/Mutex.h>

namespace tt {
namespace steam {


class Leaderboards
{
public:
	///////////////////////////////////////////////////////////////////////////
	// Instance management
	
	static bool createInstance(const std::string& p_saveFileName, tt::fs::identifier p_saveFileFSID,
		bool                    p_createOnNotFound  = false,
		ELeaderboardSortMethod  p_createSortMethod  = k_ELeaderboardSortMethodDescending,
		ELeaderboardDisplayType p_createDisplayType = k_ELeaderboardDisplayTypeNumeric);
	
	inline static Leaderboards* getInstance()
	{
		TT_NULL_ASSERT(ms_instance);
		return ms_instance;
	}
	inline static bool hasInstance() { return ms_instance != 0; }
	static void destroyInstance();
	
	// Add a download/upload request and returns a request ID
	s32 addUploadRequest(const tt::score::UploadRequest& p_request);
	s32 addDownloadRequest(const tt::score::DownloadRequest& p_request);
	void commitRequests();
	
	void update();
	
private:
	enum CommunicationState
	{
		CommunicationState_Idle,
		
		// Find/creation
		CommunicationState_FindLeaderboard,
		CommunicationState_SetupLeaderboard,
		CommunicationState_CreateLeaderboard,
		
		// Upload
		CommunicationState_UploadScore,
		CommunicationState_UploadScoreSucceeded,
		
		// Download
		CommunicationState_DownloadScores,
		CommunicationState_DownloadScoresSucceeded
	};
	
	enum RequestState
	{
		RequestState_Idle,
		RequestState_ProcessingUpload,
		RequestState_ProcessingDownload
	};
	
	// Enable autocreation of leaderboards (only in non-final builds)
	Leaderboards(const std::string& p_saveFileName, tt::fs::identifier p_saveFileFSID,
		bool p_createOnNotFound, ELeaderboardSortMethod p_createSortMethod, 
		ELeaderboardDisplayType p_createDisplayType);
	
	~Leaderboards();
	
	void loadQueue();
	void saveQueue() const;
	
	// Retrieves the leaderboard based on current RequestState
	const std::string& getRequestedLeaderboard();
	
	// Called when SteamUserStats()->FindLeaderboard() returns asynchronously
	void onFindLeaderboard(LeaderboardFindResult_t* p_result, bool p_IOFailure);
	CCallResult<Leaderboards, LeaderboardFindResult_t> m_callResultFindLeaderboard;
	
	// Called when SteamUserStats()->UploadLeaderboardScore() returns asynchronously
	void onUploadScore(LeaderboardScoreUploaded_t* p_result, bool p_IOFailure);
	CCallResult<Leaderboards, LeaderboardScoreUploaded_t> m_callResultUploadScore;
	
	// Called when SteamUserStats()->DownloadLeaderboardEntries() returns asynchronously
	void onDownloadScores(LeaderboardScoresDownloaded_t* p_result, bool p_IOFailure);
	CCallResult<Leaderboards, LeaderboardScoresDownloaded_t> m_callResultDownloadScores;
	
	tt::score::UploadRequests   m_uploadRequests;
	tt::score::DownloadRequests m_downloadRequests;
	tt::thread::Mutex           m_requestsLock;
	
	RequestState     m_requestState;
	s32              m_currentRequestID;
	
	CommunicationState m_communicationState;
	
	// Steam handle to current leaderboard
	SteamLeaderboard_t m_currentLeaderboard; 
	
	const std::string        m_saveFileName;
	const tt::fs::identifier m_saveFileFSID;
	
#if !defined(TT_BUILD_FINAL)
	// Only used in non-final builds
	bool                    m_createOnNotFound;
	ELeaderboardSortMethod  m_createSortMethod;
	ELeaderboardDisplayType m_createDisplayType;
#endif
	
	// Disable copy/assignment
	Leaderboards(const Leaderboards& p_rhs);
	Leaderboards& operator=(const Leaderboards&);
	
	// Single instance
	static Leaderboards* ms_instance;
};

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)

#endif // !defined(INC_STEAM_LEADERBOARDS_H)
