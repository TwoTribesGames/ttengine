#if defined(TT_STEAM_BUILD)  // Steam Workshop functionality is only available in Steam builds

#include <algorithm>

#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/errorhelpers.h>
#include <tt/steam/helpers.h>
#include <tt/str/str.h>

#include <toki/steam/Workshop.h>
#include <toki/steam/WorkshopObserver.h>


namespace toki {
namespace steam {

using tt::steam::getHumanReadableResult;

static const std::string g_previewImageTemplateName = "preview.png";

Workshop* Workshop::ms_instance = 0;


static void notifyObserversOfFileChange(std::vector<WorkshopObserverWeakPtr>& p_observers,
                                        PublishedFileId_t                     p_id,
                                        WorkshopObserver::FileAction          p_action)
{
	for (std::vector<WorkshopObserverWeakPtr>::iterator it = p_observers.begin(); it != p_observers.end(); )
	{
		WorkshopObserverPtr observer((*it).lock());
		if (observer != 0)
		{
			observer->onWorkshopFileChange(p_id, p_action);
			++it;
		}
		else
		{
			it = p_observers.erase(it);
		}
	}
}


template<typename T>
static bool contains(const std::vector<T>& p_container, const T& p_value)
{
	return std::find(p_container.begin(), p_container.end(), p_value) != p_container.end();
}


//--------------------------------------------------------------------------------------------------
// Public member functions

void Workshop::createInstance()
{
	TT_ASSERTMSG(ms_instance == 0, "Workshop instance already created.");
	if (ms_instance == 0)
	{
		ms_instance = new Workshop;
	}
}


void Workshop::destroyInstance()
{
	delete ms_instance;
	ms_instance = 0;
}


bool Workshop::isSubscribedToFile(PublishedFileId_t p_id) const
{
	return contains(m_subscribedFiles, p_id);
}


bool Workshop::isPublishedFile(PublishedFileId_t p_id) const
{
	return contains(m_publishedFiles, p_id);
}


std::string Workshop::getLocalPath(PublishedFileId_t p_id) const
{
	const FileDetails* details = getCachedFileDetails(p_id);
	if (details == 0 || details->validMask.checkFlag(FileDetails::ValidDetails_File) == false)
	{
		return std::string();
	}
	
	return makeDownloadPath(details->details.m_pchFileName, details->details.m_hFile);
}


std::string Workshop::getLocalPreviewImagePath(PublishedFileId_t p_id) const
{
	const FileDetails* details = getCachedFileDetails(p_id);
	if (details == 0 ||
	    details->validMask.checkFlag(FileDetails::ValidDetails_File) == false ||
	    details->details.m_hPreviewFile == k_UGCHandleInvalid)
	{
		return std::string();
	}
	
	return makeDownloadPath(g_previewImageTemplateName, details->details.m_hPreviewFile);
}


const Workshop::FileDetails* Workshop::getCachedFileDetails(PublishedFileId_t p_id) const
{
	PublishedFileDetails::const_iterator it = m_publishedFileDetails.find(p_id);
	return (it != m_publishedFileDetails.end()) ? &(*it).second : 0;
}


void Workshop::requestFileDetails(PublishedFileId_t p_id, bool p_forceRefresh)
{
	requestFileDetails(&p_id, 1, p_forceRefresh);
}


void Workshop::notifyFileWasPublished(PublishedFileId_t p_id)
{
	TT_ASSERT(p_id != 0);  // FIXME: Is there an official way to signal "invalid published file ID"?
	
	// Add the file to the list of published files
	if (isPublishedFile(p_id) == false)
	{
		m_publishedFiles.push_back(p_id);
	}
	
	// Make sure we get updated file details, not just the same details that we already had
	// (only if we already had cached details: otherwise any details will be newer than what we had)
	PublishedFileDetails::iterator it = m_publishedFileDetails.find(p_id);
	if (it != m_publishedFileDetails.end())
	{
		(*it).second.keepRefreshingUntilNewerThan = (*it).second.details.m_rtimeUpdated;
	}
	
	// Update our cached details of this file
	requestFileDetails(p_id, true);
}


void Workshop::setUserVote(PublishedFileId_t p_id, bool p_thumbsUp)
{
#if TT_DEMO_BUILD == 0
	const EWorkshopVote newVote = p_thumbsUp ? k_EWorkshopVoteFor : k_EWorkshopVoteAgainst;
	
	FileDetails& details(m_publishedFileDetails[p_id]);
	if (details.validMask.checkFlag(FileDetails::ValidDetails_VotesUser) == false ||  // don't know about this vote status yet
	    details.currentUserVote != newVote)                                           // vote changed
	{
		if (m_callResultUpdateUserPublishedItemVote.IsActive())
		{
			TT_PANIC("Cannot set new vote for Workshop item %llu: still busy updating a vote.\n"
			         "If you see this message, programmers need to make item voting use a queue.",
			         p_id);
			return;
		}
		
		// Set new vote
		TT_Printf("Setting user vote for item %llu to %d (old vote: %d)\n",
		          p_id, newVote, details.currentUserVote);
		TT_ASSERT(details.pendingNewUserVote == details.currentUserVote);
		details.pendingNewUserVote = newVote;
		SteamAPICall_t handle = SteamRemoteStorage()->UpdateUserPublishedItemVote(p_id, p_thumbsUp);
		m_callResultUpdateUserPublishedItemVote.Set(handle, this, &Workshop::onResultUpdateUserPublishedItemVote);
	}
#else
	(void)p_id;
	(void)p_thumbsUp;
#endif
}


void Workshop::openBrowseWorkshopPage()
{
	tt::steam::openURL("http://steamcommunity.com/workshop/browse/?appid=" + tt::str::toStr(SteamUtils()->GetAppID()));
}


void Workshop::registerObserver(const WorkshopObserverPtr& p_observer)
{
	TT_NULL_ASSERT(p_observer);
	m_observers.push_back(p_observer);
}


void Workshop::unregisterObserver(const WorkshopObserverPtr& p_observer)
{
	TT_NULL_ASSERT(p_observer);
	
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); )
	{
		WorkshopObserverPtr existingPtr((*it).lock());
		if (existingPtr == 0 || existingPtr == p_observer)
		{
			it = m_observers.erase(it);
		}
		else
		{
			++it;
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Workshop::Workshop()
:
m_callbackFileSubscribed(),
m_callbackFileUnsubscribed(),
m_callbackFileDeleted(),
m_callResultEnumerateUserSubscribedFiles(),
m_callResultEnumerateUserPublishedFiles(),
m_callResultGetPublishedFileDetails(),
m_callResultGetPublishedItemVoteDetails(),
m_callResultGetUserPublishedItemVoteDetails(),
m_callResultUGCDownload(),
m_callResultUpdateUserPublishedItemVote(),
m_subscribedFiles(),
m_publishedFiles(),
m_publishedFileDetails(),
m_getFileDetailsQueue(),
m_getGlobalVoteQueue(),
m_getUserVoteQueue(),
m_downloadQueue(),
m_enumerateSubscribedFilesStartIndex(0),
m_enumeratePublishedFilesStartIndex(0),
m_observers()
{
#if TT_DEMO_BUILD == 0
	m_callbackFileSubscribed  .Register(this, &Workshop::onFileSubscribed);
	m_callbackFileUnsubscribed.Register(this, &Workshop::onFileUnsubscribed);
	m_callbackFileDeleted     .Register(this, &Workshop::onFileDeleted);
	
	// Immediately get the list of published and subscribed files, so that we have this available as soon as possible
	triggerEnumerateSubscribedFiles();
	triggerEnumeratePublishedFiles();
#endif
}


Workshop::~Workshop()
{
}


void Workshop::removeSubscribedFileInfo(PublishedFileId_t p_id)
{
	PublishedFileIds::iterator it = std::find(m_subscribedFiles.begin(), m_subscribedFiles.end(), p_id);
	if (it != m_subscribedFiles.end())
	{
		m_subscribedFiles.erase(it);
	}
}


void Workshop::removePublishedFileInfo(PublishedFileId_t p_id)
{
	PublishedFileIds::iterator it = std::find(m_publishedFiles.begin(), m_publishedFiles.end(), p_id);
	if (it != m_publishedFiles.end())
	{
		m_publishedFiles.erase(it);
	}
}


void Workshop::removeCachedFileDetails(PublishedFileId_t p_id)
{
	m_publishedFileDetails.erase(p_id);
}


void Workshop::requestFileDetails(PublishedFileId_t* p_ids, int32 p_count, bool p_forceRefresh)
{
#if TT_DEMO_BUILD == 0
	const bool queueWasEmpty = m_getFileDetailsQueue.empty();
	
	for (int32 i = 0; i < p_count; ++i)
	{
		// Only add to the queue if we don't have the details yet and it wasn't already in the queue
		const FileDetails* details = getCachedFileDetails(p_ids[i]);
		if ((details == 0                                                          ||
		     details->validMask.checkFlag(FileDetails::ValidDetails_File) == false ||
		     p_forceRefresh) &&
		    contains(m_getFileDetailsQueue, p_ids[i]) == false)
		{
			m_getFileDetailsQueue.push_back(p_ids[i]);
		}
	}
	
	if (queueWasEmpty && m_getFileDetailsQueue.empty() == false)
	{
		// First entry was added to the queue: start processing the queue
		triggerFileDetailsDownload(m_getFileDetailsQueue.front());
	}
	
	// When getting file details, also request vote details along with it
	requestGlobalVoteDetails(p_ids, p_count);
	requestUserVoteDetails  (p_ids, p_count);
#else
	(void)p_ids;
	(void)p_count;
	(void)p_forceRefresh;
#endif
}


void Workshop::requestGlobalVoteDetails(PublishedFileId_t* p_ids, int32 p_count)
{
	const bool queueWasEmpty = m_getGlobalVoteQueue.empty();
	
	for (int32 i = 0; i < p_count; ++i)
	{
		// Only add to the queue if we don't have the details yet and it wasn't already in the queue
		const FileDetails* details = getCachedFileDetails(p_ids[i]);
		if ((details == 0 || details->validMask.checkFlag(FileDetails::ValidDetails_VotesGlobal) == false) &&
		    contains(m_getGlobalVoteQueue, p_ids[i]) == false)
		{
			m_getGlobalVoteQueue.push_back(p_ids[i]);
		}
	}
	
	if (queueWasEmpty && m_getGlobalVoteQueue.empty() == false)
	{
		// First entry was added to the queue: start processing the queue
		triggerGetGlobalVote(m_getGlobalVoteQueue.front());
	}
}


void Workshop::requestUserVoteDetails(PublishedFileId_t* p_ids, int32 p_count)
{
	const bool queueWasEmpty = m_getUserVoteQueue.empty();
	
	for (int32 i = 0; i < p_count; ++i)
	{
		// Only add to the queue if we don't have the details yet and it wasn't already in the queue
		const FileDetails* details = getCachedFileDetails(p_ids[i]);
		if ((details == 0 || details->validMask.checkFlag(FileDetails::ValidDetails_VotesUser) == false) &&
		    contains(m_getUserVoteQueue, p_ids[i]) == false)
		{
			m_getUserVoteQueue.push_back(p_ids[i]);
		}
	}
	
	if (queueWasEmpty && m_getUserVoteQueue.empty() == false)
	{
		// First entry was added to the queue: start processing the queue
		triggerGetUserVote(m_getUserVoteQueue.front());
	}
}


void Workshop::requestFileDownload(PublishedFileId_t p_id)
{
	const FileDetails* details = getCachedFileDetails(p_id);
	if (details == 0)
	{
		TT_PANIC("File details must be available for published file %llu before its data can be downloaded.",
		         p_id);
		return;
	}
	
	TT_ASSERTMSG(details->details.m_hFile != k_UGCHandleInvalid,
	             "Workshop file %llu does not have a valid file.", details->details.m_hFile);
	requestFileDownload(details->details.m_hFile, details->details.m_pchFileName, details->details.m_rtimeUpdated);
	
	if (details->details.m_hPreviewFile != k_UGCHandleInvalid)
	{
		requestFileDownload(details->details.m_hPreviewFile, g_previewImageTemplateName, details->details.m_rtimeUpdated);
	}
}


void Workshop::requestFileDownload(UGCHandle_t        p_handle,
                                   const std::string& p_filename,
                                   uint32             p_lastUpdateTimestamp)
{
	TT_ASSERT(p_handle != k_UGCHandleInvalid);
	
	// If already in queue, ignore
	if (contains(m_downloadQueue, p_handle))
	{
		return;
	}
	
	// If file exists on disk and published file not newer, ignore
	const std::string filePath(makeDownloadPath(p_filename, p_handle));
	if (tt::fs::fileExists(filePath))
	{
		tt::fs::FilePtr file(tt::fs::open(filePath, tt::fs::OpenMode_Read));
		TT_ASSERTMSG(file != 0, "Could not open local copy of Workshop file %llu ('%s') for timestamp check.",
		             p_handle, p_filename.c_str());
		if (file != 0)
		{
			markFileAsDownloaded(p_handle, false);
			
			const s64 modifiedTime = tt::fs::convertToUnixTime(tt::fs::getWriteTime(file));
			
			if (modifiedTime >= static_cast<s64>(p_lastUpdateTimestamp))
			{
				/*
				TT_Printf("Workshop::requestFileDownload: File %llu ('%s') is up to date "
				          "(local time %lld, Workshop time %lu). Ignoring download request.\n",
				          details->m_hFile, details->m_pchFileName, modifiedTime, details->m_rtimeUpdated);
				// */
				return;
			}
		}
	}
	
	const bool queueWasEmpty = m_downloadQueue.empty();
	
	m_downloadQueue.push_back(p_handle);
	
	if (queueWasEmpty)
	{
		// First entry was added to the queue: start processing the queue
		triggerFileDownload(m_downloadQueue.front());
	}
}


Workshop::FileDetails* Workshop::findDetailsForFileHandle(UGCHandle_t p_handle)
{
	// Don't even try to search for an invalid handle (can yield false positives and makes no sense)
	if (p_handle == k_UGCHandleInvalid)
	{
		return 0;
	}
	
	for (PublishedFileDetails::iterator it = m_publishedFileDetails.begin();
	     it != m_publishedFileDetails.end(); ++it)
	{
		if (p_handle == (*it).second.details.m_hFile ||
		    p_handle == (*it).second.details.m_hPreviewFile)
		{
			return &(*it).second;
		}
	}
	
	return 0;
}


void Workshop::markFileAsDownloaded(UGCHandle_t p_handle, bool p_notifyObservers)
{
	FileDetails* details = findDetailsForFileHandle(p_handle);
	if (details == 0)
	{
		// None of the cached details have this file handle: cannot do anything
		return;
	}
	
	if (p_handle == details->details.m_hFile)
	{
		details->validMask.setFlag(FileDetails::ValidDetails_FileDownloaded);
		if (p_notifyObservers)
		{
			notifyObserversOfFileChange(m_observers, details->details.m_nPublishedFileId,
			                            WorkshopObserver::FileAction_FileDownloaded);
		}
	}
	else if (p_handle == details->details.m_hPreviewFile)
	{
		details->validMask.setFlag(FileDetails::ValidDetails_PreviewDownloaded);
		if (p_notifyObservers)
		{
			notifyObserversOfFileChange(m_observers, details->details.m_nPublishedFileId,
			                            WorkshopObserver::FileAction_PreviewDownloaded);
		}
	}
}


void Workshop::triggerEnumerateSubscribedFiles()
{
	SteamAPICall_t handle = SteamRemoteStorage()->EnumerateUserSubscribedFiles(m_enumerateSubscribedFilesStartIndex);
	m_callResultEnumerateUserSubscribedFiles.Set(handle, this, &Workshop::onResultEnumerateSubscribedFiles);
}


void Workshop::triggerEnumeratePublishedFiles()
{
	SteamAPICall_t handle = SteamRemoteStorage()->EnumerateUserPublishedFiles(m_enumeratePublishedFilesStartIndex);
	m_callResultEnumerateUserPublishedFiles.Set(handle, this, &Workshop::onResultEnumeratePublishedFiles);
}


void Workshop::triggerFileDetailsDownload(PublishedFileId_t p_id)
{
	SteamAPICall_t handle = SteamRemoteStorage()->GetPublishedFileDetails(p_id, 0);
	m_callResultGetPublishedFileDetails.Set(handle, this, &Workshop::onResultGetPublishedFileDetails);
}


void Workshop::triggerGetGlobalVote(PublishedFileId_t p_id)
{
	SteamAPICall_t handle = SteamRemoteStorage()->GetPublishedItemVoteDetails(p_id);
	m_callResultGetPublishedItemVoteDetails.Set(handle, this, &Workshop::onResultGetPublishedItemVoteDetails);
}


void Workshop::triggerGetUserVote(PublishedFileId_t p_id)
{
	SteamAPICall_t handle = SteamRemoteStorage()->GetUserPublishedItemVoteDetails(p_id);
	m_callResultGetUserPublishedItemVoteDetails.Set(handle, this, &Workshop::onResultGetUserPublishedItemVoteDetails);
}


void Workshop::triggerFileDownload(UGCHandle_t p_handle)
{
	SteamAPICall_t handle = SteamRemoteStorage()->UGCDownload(p_handle, 0);
	m_callResultUGCDownload.Set(handle, this, &Workshop::onResultUGCDownload);
}


std::string Workshop::makeDownloadPath(const std::string& p_filename, uint64 p_fileID)
{
	// Files are stored locally under [game_dir]/workshop/[file_id].[filename_ext]
	return "workshop/" + tt::str::toStr(p_fileID) + "." + tt::fs::utils::getExtension(p_filename);
}


// Called when SteamRemoteStorage()->EnumerateUserSubscribedFiles() returns asynchronously
void Workshop::onResultEnumerateSubscribedFiles(
		RemoteStorageEnumerateUserSubscribedFilesResult_t* p_result,
		bool                                               p_ioFailure)
{
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		TT_PANIC("Could not retrieve user's subscribed files.\n"
		         "I/O failure: %s\nResult (code %d): %s",
		         p_ioFailure ? "yes" : "no", p_result->m_eResult, getHumanReadableResult(p_result->m_eResult));
		return;
	}
	
	/*
	TT_Printf("Workshop::onResultEnumerateSubscribedFiles: User subscribed to %d file(s):\n",
	          p_result->m_nResultsReturned);
	// */
	
	if (p_result->m_nResultsReturned > 0)
	{
		m_subscribedFiles.reserve(static_cast<PublishedFileIds::size_type>(p_result->m_nResultsReturned));
		m_subscribedFiles.assign(p_result->m_rgPublishedFileId, p_result->m_rgPublishedFileId + p_result->m_nResultsReturned);
	}
	
	/* DEBUG: Print IDs of subscribed files
	for (int32 i = 0; i < p_result->m_nResultsReturned; ++i)
	{
		TT_Printf("Workshop::onResultEnumerateSubscribedFiles: - ID %llu\n", p_result->m_rgPublishedFileId[i]);
	}
	// */
	
	requestFileDetails(p_result->m_rgPublishedFileId, p_result->m_nResultsReturned);
	
	// If there were more results available, get the next batch
	if (p_result->m_nTotalResultCount > p_result->m_nResultsReturned)
	{
		m_enumerateSubscribedFilesStartIndex += p_result->m_nResultsReturned;
		triggerEnumerateSubscribedFiles();
	}
}


// Called when SteamRemoteStorage()->EnumerateUserPublishedFiles() returns asynchronously
void Workshop::onResultEnumeratePublishedFiles(
		RemoteStorageEnumerateUserPublishedFilesResult_t* p_result,
		bool                                              p_ioFailure)
{
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		TT_PANIC("Could not retrieve user's published files.\n"
		         "I/O failure: %s\nResult (code %d): %s",
		         p_ioFailure ? "yes" : "no", p_result->m_eResult, getHumanReadableResult(p_result->m_eResult));
		return;
	}
	
	/*
	TT_Printf("Workshop::onResultEnumeratePublishedFiles: User published %d file(s):\n",
	          p_result->m_nResultsReturned);
	// */
	
	if (p_result->m_nResultsReturned > 0)
	{
		m_publishedFiles.reserve(static_cast<PublishedFileIds::size_type>(p_result->m_nResultsReturned));
		m_publishedFiles.assign(p_result->m_rgPublishedFileId, p_result->m_rgPublishedFileId + p_result->m_nResultsReturned);
	}
	
	/* DEBUG: Print IDs of published files
	for (int32 i = 0; i < p_result->m_nResultsReturned; ++i)
	{
		TT_Printf("Workshop::onResultEnumeratePublishedFiles: - ID %llu\n", p_result->m_rgPublishedFileId[i]);
	}
	// */
	
	requestFileDetails(p_result->m_rgPublishedFileId, p_result->m_nResultsReturned);
	
	// If there were more results available, get the next batch
	if (p_result->m_nTotalResultCount > p_result->m_nResultsReturned)
	{
		m_enumeratePublishedFilesStartIndex += p_result->m_nResultsReturned;
		triggerEnumeratePublishedFiles();
	}
}


// Called when SteamRemoteStorage()->GetPublishedFileDetails() returns asynchronously
void Workshop::onResultGetPublishedFileDetails(
		RemoteStorageGetPublishedFileDetailsResult_t* p_result,
		bool                                          p_ioFailure)
{
	const PublishedFileId_t id = p_result->m_nPublishedFileId;
	bool moveToBackOfQueue = false;
	
	// FIXME: What to do with failure situations?
	// NOTE: We can get a "file not found" result if the user subscribed to a Workshop item, but that item was later deleted
	//       (the user is still subscribed to that item in that case, even though it no longer exists)
	if (p_result->m_eResult != k_EResultFileNotFound)
	{
		TT_ASSERTMSG(p_ioFailure == false && p_result->m_eResult == k_EResultOK,
		             "Could not retrieve details for published file %llu.\n"
		             "I/O failure: %s\nResult (code %d): %s",
		             id, p_ioFailure ? "yes" : "no", p_result->m_eResult, getHumanReadableResult(p_result->m_eResult));
	}
	
	// Interpret "file not found" errors as "this Workshop item has been deleted".
	if (p_result->m_eResult == k_EResultFileNotFound)
	{
		// Remove it from our internal bookkeeping
		const bool wasSubscribed = isSubscribedToFile(id);
		removeSubscribedFileInfo(id);
		removePublishedFileInfo(id);
		
		// Also notify observers that this file was "unsubscribed" if the user was subscribed to this file
		if (wasSubscribed)
		{
			notifyObserversOfFileChange(m_observers, id, WorkshopObserver::FileAction_Unsubscribed);
		}
	}
	
	// If we got a valid result, store the details
	if (p_ioFailure == false && p_result->m_eResult == k_EResultOK)
	{
		FileDetails& cachedDetails(m_publishedFileDetails[id]);
		cachedDetails.details = *p_result;
		cachedDetails.validMask.setFlag(FileDetails::ValidDetails_File);
		
		// Do not consider the details to be available until the desired timestamp is met
		if (p_result->m_rtimeUpdated <= cachedDetails.keepRefreshingUntilNewerThan)
		{
			moveToBackOfQueue = true;
		}
		else
		{
			cachedDetails.keepRefreshingUntilNewerThan = 0;
			
			notifyObserversOfFileChange(m_observers, id, WorkshopObserver::FileAction_DetailsAvailable);
			
			/*
			TT_Printf("Published file details:\n"
			          "- m_nPublishedFileId = %llu\n"
			          "- m_nCreatorAppID    = %lu\n"
			          "- m_nConsumerAppID   = %lu\n"
			          "- m_rgchTitle        = %s\n"
			          "- m_rgchDescription  = %s\n"
			          "- m_hFile            = %llu\n"
			          "- m_hPreviewFile     = %llu\n"
			          "- m_ulSteamIDOwner   = %llu\n"
			          "- m_rtimeCreated     = %lu\n"
			          "- m_rtimeUpdated     = %lu\n"
			          "- m_eVisibility      = %d\n"
			          "- m_bBanned          = %s\n"
			          "- m_rgchTags         = %s\n"
			          "- m_bTagsTruncated   = %s\n"
			          "- m_pchFileName      = %s\n"
			          "- m_nFileSize        = %d\n"
			          "- m_nPreviewFileSize = %d\n"
			          "- m_rgchURL          = %s\n"
			          "- m_eFileType        = %d\n"
			          "\n",
			          p_result->m_nPublishedFileId,
			          p_result->m_nCreatorAppID,
			          p_result->m_nConsumerAppID,
			          p_result->m_rgchTitle,
			          p_result->m_rgchDescription,
			          p_result->m_hFile,
			          p_result->m_hPreviewFile,
			          p_result->m_ulSteamIDOwner,
			          p_result->m_rtimeCreated,
			          p_result->m_rtimeUpdated,
			          p_result->m_eVisibility,
			          p_result->m_bBanned ? "true" : "false",
			          p_result->m_rgchTags,
			          p_result->m_bTagsTruncated ? "true" : "false",
			          p_result->m_pchFileName,
			          p_result->m_nFileSize,
			          p_result->m_nPreviewFileSize,
			          p_result->m_rgchURL,
			          p_result->m_eFileType);
			// */
			
			if (isSubscribedToFile(id))
			{
				requestFileDownload(id);
			}
			
			/*
			TT_Printf("Workshop::onResultGetPublishedFileDetails: Got details for file %llu: title '%s', filename '%s'\n",
			          id, p_result->m_rgchTitle, p_result->m_pchFileName);
			// */
		}
	}
	
	// Remove the request for this file ID from the queue
	PublishedFileIds::iterator it = std::find(m_getFileDetailsQueue.begin(), m_getFileDetailsQueue.end(), id);
	TT_ASSERTMSG(it != m_getFileDetailsQueue.end(),
	             "Got details for Workshop file %llu, but these details weren't requested (not in the queue).",
	             id);
	if (it != m_getFileDetailsQueue.end())
	{
		m_getFileDetailsQueue.erase(it);
		TT_ASSERTMSG(contains(m_getFileDetailsQueue, id) == false,
		             "Workshop file %llu, is in the details queue more than once (internal error).", id);
	}
	
	if (moveToBackOfQueue)
	{
		m_getFileDetailsQueue.push_back(id);
	}
	
	// Continue with the next item in the queue
	if (m_getFileDetailsQueue.empty() == false)
	{
		triggerFileDetailsDownload(m_getFileDetailsQueue.front());
	}
}


void Workshop::onResultGetPublishedItemVoteDetails(
		RemoteStorageGetPublishedItemVoteDetailsResult_t* p_result,
		bool                                              p_ioFailure)
{
	const PublishedFileId_t id = p_result->m_unPublishedFileId;
	
	// FIXME: What to do with failure situations?
	TT_ASSERTMSG(p_ioFailure == false && p_result->m_eResult == k_EResultOK,
	             "Could not get global vote details for Workshop file %llu.\n"
	             "I/O failure: %s\nResult (code %d): %s",
	             id, p_ioFailure ? "yes" : "no", p_result->m_eResult, getHumanReadableResult(p_result->m_eResult));
	
	if (p_ioFailure == false && p_result->m_eResult == k_EResultOK)
	{
		FileDetails& details(m_publishedFileDetails[id]);
		details.globalVotesFor     = p_result->m_nVotesFor;
		details.globalVotesAgainst = p_result->m_nVotesAgainst;
		details.globalReports      = p_result->m_nReports;
		details.globalScore        = p_result->m_fScore;
		details.validMask.setFlag(FileDetails::ValidDetails_VotesGlobal);
		
		notifyObserversOfFileChange(m_observers, id, WorkshopObserver::FileAction_GlobalVotesAvailable);
		
		/*
		TT_Printf("Workshop::onResultGetPublishedItemVoteDetails: Global vote details for file %llu:\n"
		          "- Votes for:     %d\n"
		          "- Votes against: %d\n"
		          "- Reports:       %d\n"
		          "- Score:         %f\n",
		          id,
		          p_result->m_nVotesFor,
		          p_result->m_nVotesAgainst,
		          p_result->m_nReports,
		          p_result->m_fScore);
		// */
	}
	
	// Remove the request for this file ID from the queue
	PublishedFileIds::iterator it = std::find(m_getGlobalVoteQueue.begin(), m_getGlobalVoteQueue.end(), id);
	TT_ASSERTMSG(it != m_getGlobalVoteQueue.end(),
	             "Got global votes for Workshop file %llu, but these votes weren't requested (not in the queue).",
	             id);
	if (it != m_getGlobalVoteQueue.end())
	{
		m_getGlobalVoteQueue.erase(it);
		TT_ASSERTMSG(contains(m_getGlobalVoteQueue, id) == false,
		             "Workshop file %llu is in the global vote queue more than once (internal error).", id);
	}
	
	// Continue with the next item in the queue
	if (m_getGlobalVoteQueue.empty() == false)
	{
		triggerGetGlobalVote(m_getGlobalVoteQueue.front());
	}
}


void Workshop::onResultGetUserPublishedItemVoteDetails(
		RemoteStorageUserVoteDetails_t* p_result,
		bool                            p_ioFailure)
{
	const PublishedFileId_t id = p_result->m_nPublishedFileId;
	
	// FIXME: What to do with failure situations?
	TT_ASSERTMSG(p_ioFailure == false && p_result->m_eResult == k_EResultOK,
	             "Could not get user vote details for Workshop file %llu.\n"
	             "I/O failure: %s\nResult (code %d): %s",
	             id, p_ioFailure ? "yes" : "no", p_result->m_eResult, getHumanReadableResult(p_result->m_eResult));
	
	if (p_ioFailure == false && p_result->m_eResult == k_EResultOK)
	{
		FileDetails& details(m_publishedFileDetails[id]);
		details.currentUserVote    = p_result->m_eVote;
		details.pendingNewUserVote = details.currentUserVote;
		details.validMask.setFlag(FileDetails::ValidDetails_VotesUser);
		
		notifyObserversOfFileChange(m_observers, id, WorkshopObserver::FileAction_UserVotesAvailable);
		
		/*
		const char* voteName = "unknown";
		switch (p_result->m_eVote)
		{
		case k_EWorkshopVoteUnvoted: voteName = "Unvoted";               break;
		case k_EWorkshopVoteFor:     voteName = "For (thumbs up)";       break;
		case k_EWorkshopVoteAgainst: voteName = "Against (thumbs down)"; break;
		default: break;
		}
		TT_Printf("Workshop::onResultGetUserPublishedItemVoteDetails: User's vote for file %llu: %s\n",
		          id, voteName);
		// */
	}
	
	// Remove the request for this file ID from the queue
	PublishedFileIds::iterator it = std::find(m_getUserVoteQueue.begin(), m_getUserVoteQueue.end(), id);
	TT_ASSERTMSG(it != m_getUserVoteQueue.end(),
	             "Got user votes for Workshop file %llu, but these votes weren't requested (not in the queue).",
	             id);
	if (it != m_getUserVoteQueue.end())
	{
		m_getUserVoteQueue.erase(it);
		TT_ASSERTMSG(contains(m_getUserVoteQueue, id) == false,
		             "Workshop file %llu is in the user vote queue more than once (internal error).", id);
	}
	
	// Continue with the next item in the queue
	if (m_getUserVoteQueue.empty() == false)
	{
		triggerGetUserVote(m_getUserVoteQueue.front());
	}
}


void Workshop::onResultUGCDownload(RemoteStorageDownloadUGCResult_t* p_result, bool p_ioFailure)
{
	const UGCHandle_t fileHandle = p_result->m_hFile;
	// FIXME: What to do with failure situations?
	TT_ASSERTMSG(p_ioFailure == false && p_result->m_eResult == k_EResultOK,
	             "Could not download Workshop file with handle %llu (filename %s)\n"
	             "I/O failure: %s\nResult (code %d): %s",
	             fileHandle, p_result->m_pchFileName, p_ioFailure ? "yes" : "no", p_result->m_eResult,
	             getHumanReadableResult(p_result->m_eResult));
	
	if (p_ioFailure == false && p_result->m_eResult == k_EResultOK)
	{
		//TT_Printf("Workshop::onResultUGCDownload: Downloaded file '%s', size %d bytes.\n",
		//          p_result->m_pchFileName, p_result->m_nSizeInBytes);
		
		// Write the downloaded content to disk (keep a local copy,
		// so data doesn't have to be downloaded for each application run)
		const std::string downloadPath(makeDownloadPath(p_result->m_pchFileName, fileHandle));
		
		const std::string downloadDir(tt::fs::utils::getDirectory(downloadPath));
		if (tt::fs::dirExists(downloadDir) == false &&
		    tt::fs::utils::createDirRecursive(downloadDir) == false)
		{
			// FIXME: Perform proper error handling
			TT_PANIC("Could not create directory for Workshop downloaded file.\nPath: %s",
			         downloadDir.c_str());
		}
		
		tt::fs::FilePtr file(tt::fs::open(downloadPath, tt::fs::OpenMode_Write));
		if (file != 0)
		{
			u8* fileContents = new u8[p_result->m_nSizeInBytes];
			
			const int32 bytesRead = SteamRemoteStorage()->UGCRead(
					fileHandle, fileContents, p_result->m_nSizeInBytes, 0, k_EUGCRead_ContinueReadingUntilFinished);
			TT_ASSERT(bytesRead == p_result->m_nSizeInBytes);
			
			file->write(fileContents, static_cast<tt::fs::size_type>(p_result->m_nSizeInBytes));
			
			delete[] fileContents;
			
			// Set the local copy's modification timestamp to the one provided by Steam Workshop
			{
				FileDetails* details = findDetailsForFileHandle(fileHandle);
				if (details != 0)
				{
					const uint32            unixTime   = details->details.m_rtimeUpdated;
					const tt::fs::time_type nativeTime = tt::fs::convertToNativeTime(static_cast<s64>(unixTime));
					if (tt::fs::setWriteTime(file, nativeTime) == false)
					{
						TT_WARN("Could not set modification time of local copy of Workshop file %llu "
						        "to match the last update time given by Steam Workshop.",
						        details->details.m_nPublishedFileId);
					}
				}
			}
			
			file.reset();  // explicitly close the file, so that we no longer keep it in use
			
			// File is successfully downloaded: update flags and notify observers
			markFileAsDownloaded(fileHandle, true);
		}
	}
	
	// Remove the request for this file ID from the queue
	UGCHandles::iterator it = std::find(m_downloadQueue.begin(), m_downloadQueue.end(), fileHandle);
	TT_ASSERTMSG(it != m_downloadQueue.end(),
	             "Workshop file '%s' (ID %llu) was downloaded, but this download wasn't requested (not in the queue).",
	             p_result->m_pchFileName, fileHandle);
	if (it != m_downloadQueue.end())
	{
		m_downloadQueue.erase(it);
		TT_ASSERTMSG(contains(m_downloadQueue, fileHandle) == false,
		             "Workshop file %llu, is in the download queue more than once (internal error).", fileHandle);
	}
	
	// Continue with the next item in the queue
	if (m_downloadQueue.empty() == false)
	{
		triggerFileDownload(m_downloadQueue.front());
	}
}


void Workshop::onResultUpdateUserPublishedItemVote(
		RemoteStorageUpdateUserPublishedItemVoteResult_t* p_result,
		bool                                              p_ioFailure)
{
	const PublishedFileId_t id = p_result->m_nPublishedFileId;
	
	// FIXME: What to do with failure situations?
	TT_ASSERTMSG(p_ioFailure == false && p_result->m_eResult == k_EResultOK,
	             "Could not update user vote for Workshop item %llu.\n"
	             "I/O failure: %s\nResult (code %d): %s",
	             id, p_ioFailure ? "yes" : "no", p_result->m_eResult,
	             getHumanReadableResult(p_result->m_eResult));
	
	if (p_ioFailure == false && p_result->m_eResult == k_EResultOK)
	{
		//TT_Printf("User vote for item %llu was set successfully.\n", id);
		PublishedFileDetails::iterator it = m_publishedFileDetails.find(id);
		TT_ASSERTMSG(it != m_publishedFileDetails.end(),
		             "The user vote for unknown Workshop item %llu was updated.", id);
		if (it != m_publishedFileDetails.end())
		{
			FileDetails& details((*it).second);
			TT_ASSERTMSG(details.pendingNewUserVote != k_EWorkshopVoteUnvoted,
			             "Tried to set the new user vote for item %llu to 'unvoted': this is not allowed.", id);
			if (details.pendingNewUserVote != k_EWorkshopVoteUnvoted)
			{
				TT_Printf("User vote for item %llu was successfully changed to %d (old vote: %d)\n",
				          id, details.pendingNewUserVote, details.currentUserVote);
				
				details.currentUserVote = details.pendingNewUserVote;
				details.validMask.setFlag(FileDetails::ValidDetails_VotesUser);
				
				notifyObserversOfFileChange(m_observers, id, WorkshopObserver::FileAction_UserVotesAvailable);
			}
		}
	}
}


// User subscribed to a file for the app (from within the app or on the web)
void Workshop::onFileSubscribed(RemoteStoragePublishedFileSubscribed_t* p_details)
{
	TT_Printf("Workshop::onFileSubscribed: User subscribed to file %llu\n", p_details->m_nPublishedFileId);
	// FIXME: Go through a helper function? Do we need to retrieve details for this file right away? Download it?
	m_subscribedFiles.push_back(p_details->m_nPublishedFileId);
	notifyObserversOfFileChange(m_observers, p_details->m_nPublishedFileId, WorkshopObserver::FileAction_Subscribed);
	requestFileDetails(p_details->m_nPublishedFileId);
}


// User unsubscribed from a file for the app (from within the app or on the web)
void Workshop::onFileUnsubscribed(RemoteStoragePublishedFileUnsubscribed_t* p_details)
{
	TT_Printf("Workshop::onFileUnsubscribed: User unsubscribed from file %llu\n", p_details->m_nPublishedFileId);
	removeSubscribedFileInfo(p_details->m_nPublishedFileId);
	notifyObserversOfFileChange(m_observers, p_details->m_nPublishedFileId, WorkshopObserver::FileAction_Unsubscribed);
}


// Published file that a user owns was deleted (from within the app or the web)
void Workshop::onFileDeleted(RemoteStoragePublishedFileDeleted_t* p_details)
{
	TT_Printf("Workshop::onFileDeleted: Published file %llu was deleted.\n", p_details->m_nPublishedFileId);
	removePublishedFileInfo (p_details->m_nPublishedFileId);
	removeSubscribedFileInfo(p_details->m_nPublishedFileId);
	removeCachedFileDetails (p_details->m_nPublishedFileId);
}

// Namespace end
}
}

#endif  // !defined(TT_STEAM_BUILD)
