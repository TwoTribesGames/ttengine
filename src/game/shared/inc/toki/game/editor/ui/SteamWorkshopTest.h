#if !defined(INC_TOKI_GAME_EDITOR_UI_STEAMWORKSHOPTEST_H)
#define INC_TOKI_GAME_EDITOR_UI_STEAMWORKSHOPTEST_H

#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)


#include <map>
#include <utility>
#include <vector>
#include <steam/steam_api.h>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Controls/TextBox.h>
#include <Gwen/Controls/WindowControl.h>

#include <toki/game/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Window control for testing out Steam Workshop features (only available in Steam builds). */
class SteamWorkshopTest : public Gwen::Controls::WindowControl
{
public:
	GWEN_CONTROL(SteamWorkshopTest, Gwen::Controls::WindowControl);
	virtual ~SteamWorkshopTest();
	
	static SteamWorkshopTest* create(Editor*               p_editor,
	                                 Gwen::Controls::Base* p_parent);
	
private:
	enum ListID
	{
		ListID_Published,
		ListID_Subscribed,
		ListID_AllWorkshop,
		
		ListID_Count
	};
	
	
	void createUi();
	
	void addFileItem(const RemoteStorageGetPublishedFileDetailsResult_t& p_details,
	                 ListID                                              p_list);
	void fillList(ListID             p_list,
	              PublishedFileId_t* p_fileIDs,
	              int32              p_count,
	              bool               p_hasError);
	void refreshAllLists();
	void clearLevelDetails();
	
	// UI handlers
	void onRefreshPublished();
	void onRefreshSubscribed();
	void onRefreshAllWorkshop();
	void onPublishLevel();
	void onBrowseWorkshop();
	void onSubscribe();
	void onDeleteLevel();
	void onSelectFile(Gwen::Controls::Base* p_sender);
	void onListTab   (Gwen::Controls::Base* p_sender);
	
	void onConfirmDeleteClosed(Gwen::Controls::Base* p_sender);
	
	void getPublishedFileDetails(PublishedFileId_t p_publishedFileId);
	
	// Callbacks for asynchronous Steam calls
	void onResultPublishedFiles     (RemoteStorageEnumerateUserPublishedFilesResult_t*  p_result, bool p_ioFailure);
	void onResultSubscribedFiles    (RemoteStorageEnumerateUserSubscribedFilesResult_t* p_result, bool p_ioFailure);
	void onResultAllWorkshopFiles   (RemoteStorageEnumerateWorkshopFilesResult_t*       p_result, bool p_ioFailure);
	void onResultPublishWorkshopFile(RemoteStoragePublishFileResult_t*                  p_result, bool p_ioFailure);
	void onGetPublishedFileDetails  (RemoteStorageGetPublishedFileDetailsResult_t*      p_result, bool p_ioFailure);
	void onResultFileShare          (RemoteStorageFileShareResult_t*                    p_result, bool p_ioFailure);
	void onResultSubscribe          (RemoteStorageSubscribePublishedFileResult_t*       p_result, bool p_ioFailure);
	void onResultUnsubscribe        (RemoteStorageUnsubscribePublishedFileResult_t*     p_result, bool p_ioFailure);
	void onResultDeletePublishedFile(RemoteStorageDeletePublishedFileResult_t*          p_result, bool p_ioFailure);
	
	CCallResult<SteamWorkshopTest, RemoteStorageEnumerateUserPublishedFilesResult_t>  m_callbackUserPublishedFiles;
	CCallResult<SteamWorkshopTest, RemoteStorageEnumerateUserSubscribedFilesResult_t> m_callbackUserSubscribedFiles;
	CCallResult<SteamWorkshopTest, RemoteStorageEnumerateWorkshopFilesResult_t>       m_callbackAllWorkshopFiles;
	CCallResult<SteamWorkshopTest, RemoteStoragePublishFileResult_t>                  m_callbackPublishWorkshopFile;
	CCallResult<SteamWorkshopTest, RemoteStorageGetPublishedFileDetailsResult_t>      m_callbackGetPublishedFileDetails;
	CCallResult<SteamWorkshopTest, RemoteStorageFileShareResult_t>                    m_callbackFileShare;
	CCallResult<SteamWorkshopTest, RemoteStorageSubscribePublishedFileResult_t>       m_callbackSubscribe;
	CCallResult<SteamWorkshopTest, RemoteStorageUnsubscribePublishedFileResult_t>     m_callbackUnsubscribe;
	CCallResult<SteamWorkshopTest, RemoteStorageDeletePublishedFileResult_t>          m_callbackDeletePublishedFile;
	
	// No copying
	SteamWorkshopTest(const SteamWorkshopTest&);
	SteamWorkshopTest& operator=(const SteamWorkshopTest&);
	
	
	Editor*      m_editor;
	const uint32 m_appID;
	
	Gwen::Controls::ListBox* m_listFiles    [ListID_Count];
	Gwen::Controls::Button*  m_buttonRefresh[ListID_Count];
	
	Gwen::Controls::Label*  m_labelDetailsTitle;
	Gwen::Controls::Label*  m_labelDetailsAuthor;
	Gwen::Controls::Label*  m_labelDetailsFilename;
	Gwen::Controls::Label*  m_labelDetailsSize;
	Gwen::Controls::Label*  m_labelDetailsDescription;
	Gwen::Controls::Button* m_buttonDetailsSubscribe;
	Gwen::Controls::Button* m_buttonDetailsDelete;
	
	Gwen::Controls::TextBox*          m_textPublishTitle;
	Gwen::Controls::TextBoxMultiline* m_textPublishDesc;
	Gwen::Controls::Label*            m_labelPublishStatus;
	Gwen::Controls::Button*           m_buttonPublish;
	
	struct PublishDetails
	{
		std::string                           filename;
		std::string                           previewFilename;
		std::string                           title;
		std::string                           description;
		ERemoteStoragePublishedFileVisibility visibility;
		
		inline PublishDetails()
		:
		filename(),
		previewFilename(),
		title(),
		description(),
		visibility(k_ERemoteStoragePublishedFileVisibilityPublic)
		{ }
	};
	PublishDetails m_publishDetails;
	
	typedef std::pair<PublishedFileId_t, ListID> DetailsRequest;
	typedef std::vector<DetailsRequest>          DetailsRequests;
	DetailsRequests m_getPublishedFileDetailsQueue;
	
	typedef std::map<PublishedFileId_t, RemoteStorageGetPublishedFileDetailsResult_t> PublishedFileDetails;
	PublishedFileDetails m_publishedFileDetails;
	
	typedef std::set<PublishedFileId_t> PublishedFileIdSet;
	PublishedFileIdSet m_subscribedFiles;
	
	PublishedFileId_t m_selectedFileId;
	
	typedef std::vector<std::string> UploadQueue;
	UploadQueue m_uploadQueue;
};

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)

#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_STEAMWORKSHOPTEST_H)
