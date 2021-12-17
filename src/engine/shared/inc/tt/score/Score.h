#if !defined(INC_TT_SCORE_SCORE_H)
#define INC_TT_SCORE_SCORE_H

#include <string>
#include <vector>

#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace score {

///////////////////////////////////////////////////////////////////////////
// Upload Requests

struct UploadRequest
{
	std::string leaderboard;
	s32         score;
	UploadRequest(const std::string& p_leaderboard, s32 p_score)
	:
	leaderboard(p_leaderboard),
	score(p_score)
	{}
};

///////////////////////////////////////////////////////////////////////////
// Download Requests

enum DownloadRequestSource
{
	DownloadRequestSource_Game, // Request made from game internally
	DownloadRequestSource_User  // Request made by user action
};

enum DownloadRequestRangeType
{
	DownloadRequestRangeType_Global,
	DownloadRequestRangeType_GlobalAroundUser,
	DownloadRequestRangeType_Friends,
	DownloadRequestRangeType_FriendsAroundUser,
	
	DownloadRequestRangeType_Count,
	DownloadRequestRangeType_Invalid
};

struct DownloadRequestResultEntry
{
	std::string name;
	s32         rank;
	s32         score;
	bool        isDeveloper;
	DownloadRequestResultEntry(const std::string& p_name, s32 p_rank, s32 p_score, bool p_isDeveloper)
	:
	name(p_name),
	rank(p_rank),
	score(p_score),
	isDeveloper(p_isDeveloper)
	{}
};
typedef std::vector<DownloadRequestResultEntry> DownloadRequestResultEntries;

enum DownloadRequestStatus
{
	DownloadRequestStatus_Success,
	DownloadRequestStatus_Failed
};

struct DownloadRequestResults
{
	DownloadRequestStatus        status;
	DownloadRequestRangeType     rangeType;
	std::string                  leaderboard;
	s32                          leaderboardTotalEntries;
	DownloadRequestResultEntries entries;
	s32                          currentUserIndex; // -1 if not in entries, otherwise the index at which our user can be found
	u32                          userData;
};

typedef void (*DownloadRequestCallback)(s32 p_requestID, const DownloadRequestResults& p_results);

struct DownloadRequest
{
	std::string              leaderboard;
	DownloadRequestRangeType rangeType;
	s32                      rangeMin;
	s32                      rangeMax;
	DownloadRequestSource    source;
	DownloadRequestCallback  callback;
	u32                      userData;
	
	DownloadRequest(const std::string& p_leaderboard, DownloadRequestRangeType p_rangeType,
	               s32 p_rangeMin, s32 p_rangeMax, DownloadRequestSource p_source, DownloadRequestCallback p_callback, u32 p_userData = 0)
	:
	leaderboard(p_leaderboard),
	rangeType(p_rangeType),
	rangeMin(p_rangeMin),
	rangeMax(p_rangeMax),
	source(p_source),
	callback(p_callback),
	userData(p_userData)
	{
		TT_ASSERT(p_rangeMin <= p_rangeMax);
		
		// Do some validation
		switch (p_rangeType)
		{
		case DownloadRequestRangeType_Global:            TT_ASSERT(p_rangeMin >= 1); break;
		case DownloadRequestRangeType_GlobalAroundUser:  TT_ASSERT(p_rangeMin <= 0 && p_rangeMax >= 0); break;
		case DownloadRequestRangeType_Friends:           TT_ASSERT(p_rangeMin >= 1); break;
		case DownloadRequestRangeType_FriendsAroundUser: TT_ASSERT(p_rangeMin <= 0 && p_rangeMax >= 0); break;
		default:
			TT_PANIC("Unhandled range type '%d'", p_rangeType);
			break;
		}
	}
};


// If there are not enough entries in the leaderboard before or after the user's entry,
// this helper will adjust the range to try to return the number of entries requested.
// For example, if the user is #1 on the leaderboard, start is set to -2, and end is set to 2,
// Helper will set range to the first 5 entries in the leaderboard.
inline void fitRequestRange(s32& p_minRange, s32& p_maxRange, s32 p_minClamp, s32 p_maxClamp)
{
	TT_ASSERT(p_minRange <= p_maxRange);
	TT_ASSERT(p_minClamp <= p_maxClamp);
	
	if (p_minRange < 0)
	{
		p_maxRange -= p_minRange;
		p_minRange = 0;
	}
	else if (p_maxRange > p_maxClamp)
	{
		p_minRange -= p_maxRange - p_maxClamp;
		p_maxRange = p_maxClamp;
	}
	
	tt::math::clamp(p_minRange, p_minClamp, p_maxClamp);
	tt::math::clamp(p_maxRange, p_minClamp, p_maxClamp);
}


typedef std::vector<std::pair<s32, UploadRequest> >   UploadRequests;
typedef std::vector<std::pair<s32, DownloadRequest> > DownloadRequests;


// Namespace end
}
}

#endif // INC_TT_SCORE_SCORE_H
