#if !defined(INC_TOKI_STEAM_WORKSHOP_H)
#define INC_TOKI_STEAM_WORKSHOP_H


#if defined(TT_STEAM_BUILD)  // Steam Workshop functionality is only available in Steam builds


#include <map>
#include <vector>

#include <steam/steam_api.h>

#include <tt/code/BitMask.h>
#include <tt/platform/tt_error.h>

#include <toki/steam/fwd.h>


namespace toki {
namespace steam {

class Workshop
{
public:
	struct FileDetails
	{
		enum ValidDetails
		{
			ValidDetails_File,               // RemoteStorageGetPublishedFileDetailsResult_t is ready for use
			ValidDetails_VotesUser,
			ValidDetails_VotesGlobal,
			ValidDetails_FileDownloaded,     // Workshop file was downloaded (local copy available)
			ValidDetails_PreviewDownloaded,  // Preview image was downloaded (local copy available)
			
			ValidDetails_Count
		};
		typedef tt::code::BitMask<ValidDetails, ValidDetails_Count> ValidDetailsMask;
		
		ValidDetailsMask validMask;  // indicates which information is valid
		
		// Subset of RemoteStorageGetPublishedFileDetailsResult_t:
		// FIXME: Perhaps translate to the information we need, in our types?
		RemoteStorageGetPublishedFileDetailsResult_t details;
		uint32 keepRefreshingUntilNewerThan;  // force request of details until last update newer than this timestamp
		
		// Subset of RemoteStorageGetPublishedItemVoteDetailsResult_t:
		int32         globalVotesFor;
		int32         globalVotesAgainst;
		int32         globalReports;
		float         globalScore;
		
		// Subset of RemoteStorageUserVoteDetails_t:
		EWorkshopVote currentUserVote; // what the current user voted for this item
		EWorkshopVote pendingNewUserVote; // trying to set the user vote to this
		
		
		inline FileDetails()
		:
		validMask         (),
		details           (),
		keepRefreshingUntilNewerThan(0),
		globalVotesFor    (0),
		globalVotesAgainst(0),
		globalReports     (0),
		globalScore       (0.0f),
		currentUserVote   (k_EWorkshopVoteUnvoted),
		pendingNewUserVote(k_EWorkshopVoteUnvoted)
		{ }
	};
	
	
	static void createInstance();
	static void destroyInstance();
	
	static inline bool      hasInstance() { return ms_instance != 0;                         }
	static inline Workshop* getInstance() { TT_NULL_ASSERT(ms_instance); return ms_instance; }
	
	inline const PublishedFileIds& getSubscribedFiles() const { return m_subscribedFiles; }
	bool isSubscribedToFile(PublishedFileId_t p_id) const;
	
	// Returns the files that the current user published
	inline const PublishedFileIds& getPublishedFiles() const { return m_publishedFiles; }
	bool isPublishedFile(PublishedFileId_t p_id) const;
	
	/*! \brief Returns the path to the local copy of a downloaded file.
	           Only valid if this file was downloaded. Returns an empty string otherwise. */
	std::string getLocalPath(PublishedFileId_t p_id) const;
	
	/*! \brief Returns the path to the local copy of the preview image of a downloaded file.
	           Only valid if this file was downloaded. Returns an empty string otherwise. */
	std::string getLocalPreviewImagePath(PublishedFileId_t p_id) const;
	
	/*! \brief Returns the (cached) details of a Workshop file.
	           Returns a null pointer if no details are available. */
	const FileDetails* getCachedFileDetails(PublishedFileId_t p_id) const;
	
	/*! \brief Requests details of the specified Workshop file.
	    \param p_id           The Workshop file for which to get the details.
	    \param p_forceRefresh Whether to retrieve the details from Steam even if cached details are present. */
	void requestFileDetails(PublishedFileId_t p_id, bool p_forceRefresh = false);
	
	/*! \brief Allows this class to update its bookkeeping when a new item was published. */
	void notifyFileWasPublished(PublishedFileId_t p_id);
	
	/*! \brief Sets the user's vote for a Workshop item.
	    \param p_id The item to vote for.
	    \param p_thumbsUp Thumbs up / for (true) or down / against (false). */
	void setUserVote(PublishedFileId_t p_id, bool p_thumbsUp);
	
	void openBrowseWorkshopPage();
	
	void registerObserver  (const WorkshopObserverPtr& p_observer);
	void unregisterObserver(const WorkshopObserverPtr& p_observer);
	
private:
	typedef std::map   <PublishedFileId_t, FileDetails> PublishedFileDetails;
	typedef std::vector<UGCHandle_t>                    UGCHandles;
	typedef std::vector<WorkshopObserverWeakPtr>        Observers;
	
	
	Workshop();
	~Workshop();
	
	void removeSubscribedFileInfo(PublishedFileId_t  p_id);
	void removePublishedFileInfo (PublishedFileId_t  p_id);
	void removeCachedFileDetails (PublishedFileId_t  p_id);
	void requestFileDetails      (PublishedFileId_t* p_ids, int32 p_count, bool p_forceRefresh = false);
	void requestGlobalVoteDetails(PublishedFileId_t* p_ids, int32 p_count);
	void requestUserVoteDetails  (PublishedFileId_t* p_ids, int32 p_count);
	void requestFileDownload     (PublishedFileId_t  p_id);
	void requestFileDownload(UGCHandle_t p_handle, const std::string& p_filename, uint32 p_lastUpdateTimestamp);
	FileDetails* findDetailsForFileHandle(UGCHandle_t p_handle);
	void markFileAsDownloaded(UGCHandle_t p_handle, bool p_notifyObservers);
	
	// Functions to immediately trigger a specific API call (not using queues):
	void triggerEnumerateSubscribedFiles();
	void triggerEnumeratePublishedFiles();
	void triggerFileDetailsDownload(PublishedFileId_t p_id);
	void triggerGetGlobalVote      (PublishedFileId_t p_id);
	void triggerGetUserVote        (PublishedFileId_t p_id);
	void triggerFileDownload       (UGCHandle_t       p_handle);
	
	static std::string makeDownloadPath(const std::string& p_filename, uint64 p_fileID);
	
	// Disable copying/assignment
	Workshop(const Workshop&);
	const Workshop& operator=(const Workshop&);
	
	// Steam call result callbacks
	void onResultEnumerateSubscribedFiles       (RemoteStorageEnumerateUserSubscribedFilesResult_t* p_result, bool p_ioFailure);
	void onResultEnumeratePublishedFiles        (RemoteStorageEnumerateUserPublishedFilesResult_t*  p_result, bool p_ioFailure);
	void onResultGetPublishedFileDetails        (RemoteStorageGetPublishedFileDetailsResult_t*      p_result, bool p_ioFailure);
	void onResultGetPublishedItemVoteDetails    (RemoteStorageGetPublishedItemVoteDetailsResult_t*  p_result, bool p_ioFailure);
	void onResultGetUserPublishedItemVoteDetails(RemoteStorageUserVoteDetails_t*                    p_result, bool p_ioFailure);
	void onResultUGCDownload                    (RemoteStorageDownloadUGCResult_t*                  p_result, bool p_ioFailure);
	void onResultUpdateUserPublishedItemVote    (RemoteStorageUpdateUserPublishedItemVoteResult_t*  p_result, bool p_ioFailure);
	
	
	STEAM_CALLBACK_MANUAL(Workshop, onFileSubscribed,   RemoteStoragePublishedFileSubscribed_t,   m_callbackFileSubscribed);
	STEAM_CALLBACK_MANUAL(Workshop, onFileUnsubscribed, RemoteStoragePublishedFileUnsubscribed_t, m_callbackFileUnsubscribed);
	STEAM_CALLBACK_MANUAL(Workshop, onFileDeleted,      RemoteStoragePublishedFileDeleted_t,      m_callbackFileDeleted);
	
	CCallResult<Workshop, RemoteStorageEnumerateUserSubscribedFilesResult_t> m_callResultEnumerateUserSubscribedFiles;
	CCallResult<Workshop, RemoteStorageEnumerateUserPublishedFilesResult_t>  m_callResultEnumerateUserPublishedFiles;
	CCallResult<Workshop, RemoteStorageGetPublishedFileDetailsResult_t>      m_callResultGetPublishedFileDetails;
	CCallResult<Workshop, RemoteStorageGetPublishedItemVoteDetailsResult_t>  m_callResultGetPublishedItemVoteDetails;
	CCallResult<Workshop, RemoteStorageUserVoteDetails_t>                    m_callResultGetUserPublishedItemVoteDetails;
	CCallResult<Workshop, RemoteStorageDownloadUGCResult_t>                  m_callResultUGCDownload;
	CCallResult<Workshop, RemoteStorageUpdateUserPublishedItemVoteResult_t>  m_callResultUpdateUserPublishedItemVote;
	
	PublishedFileIds     m_subscribedFiles;
	PublishedFileIds     m_publishedFiles;
	PublishedFileDetails m_publishedFileDetails; // cached details about any Workshop file
	PublishedFileIds     m_getFileDetailsQueue;
	PublishedFileIds     m_getGlobalVoteQueue;
	PublishedFileIds     m_getUserVoteQueue;
	UGCHandles           m_downloadQueue;
	
	uint32 m_enumerateSubscribedFilesStartIndex;
	uint32 m_enumeratePublishedFilesStartIndex;
	
	Observers m_observers;
	
	static Workshop* ms_instance;
};

// Namespace end
}
}


#endif  // defined(TT_STEAM_BUILD)

#endif  // !defined(INC_TOKI_STEAM_WORKSHOP_H)
