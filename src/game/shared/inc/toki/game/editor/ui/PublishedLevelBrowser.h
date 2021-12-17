#if !defined(INC_TOKI_GAME_EDITOR_UI_PUBLISHEDLEVELBROWSER_H)
#define INC_TOKI_GAME_EDITOR_UI_PUBLISHEDLEVELBROWSER_H

#if defined(TT_STEAM_BUILD)


#include <steam/steam_api.h>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Controls/TextBox.h>
#include <Gwen/Controls/WindowControl.h>

#include <toki/game/editor/fwd.h>
#include <toki/steam/fwd.h>
#include <toki/steam/WorkshopObserver.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Allows users to view their published levels and save them to disk if they don't have them anymore. */
class PublishedLevelBrowser : public Gwen::Controls::WindowControl
{
public:
	GWEN_CONTROL(PublishedLevelBrowser, WindowControl);
	virtual ~PublishedLevelBrowser();
	
	static PublishedLevelBrowser* create(Editor*               p_editor,
	                                     Gwen::Controls::Base* p_parent);
	
	virtual void SetHidden(bool p_hidden);
	
	/*
	void handleFileDetailsAvailable();
	*/
	void onWorkshopFileChange(PublishedFileId_t p_id, steam::WorkshopObserver::FileAction p_action);
	
private:
	void createUi();
	Gwen::Controls::TextBox* createDetailsRow(const Gwen::TextObject& p_label,
	                                          Gwen::Controls::Base*   p_parent);
	
	void refreshLevelList();
	void onLevelSelected();
	void onSaveToDisk();
	void onSaveConfirmationClosed(Gwen::Controls::Base* p_sender);
	
	void showSaveConfirmation(const std::wstring& p_title,
	                          const std::wstring& p_prompt,
	                          PublishedFileId_t   p_id);
	void saveLocalCopy(PublishedFileId_t p_id);
	
	/*
	- Keep list of file IDs busy downloading
	- In Workshop callbacks, check if ID in list
	
	FIXME: Also respond to "published file deleted" callback?
	*/
	
#if 0
	void setUiEnabled(bool p_enabled);
	void setLevelPreviewVisible(bool p_visible);
	void handlePublishResult(EResult p_resultCode, bool p_ioFailure, PublishedFileId_t p_id);
	
	void triggerFileUpload(const std::string& p_filename);
	
	// UI handlers
	void onViewAgreement();
	void onPublishLevel();
	
	void onInputErrorDialogClosed     (Gwen::Controls::Base* p_sender);
	void onPublishErrorDialogClosed   (Gwen::Controls::Base* p_sender);
	void onConfirmPlaytestDialogClosed(Gwen::Controls::Base* p_sender);
	
	// Callbacks for asynchronous Steam calls
	void onResultPublishWorkshopFile(RemoteStoragePublishFileResult_t*         p_result, bool p_ioFailure);
	void onResultUpdatePublishedFile(RemoteStorageUpdatePublishedFileResult_t* p_result, bool p_ioFailure);
	void onResultFileShare          (RemoteStorageFileShareResult_t*           p_result, bool p_ioFailure);
	
	CCallResult<PublishedLevelBrowser, RemoteStoragePublishFileResult_t>         m_callResultPublishWorkshopFile;
	CCallResult<PublishedLevelBrowser, RemoteStorageUpdatePublishedFileResult_t> m_callResultUpdatePublishedFile;
	CCallResult<PublishedLevelBrowser, RemoteStorageFileShareResult_t>           m_callResultFileShare;
#endif
	void onResultEnumerateUserSharedWorkshopFiles(RemoteStorageEnumerateUserSharedWorkshopFilesResult_t* p_result, bool p_ioFailure);
	CCallResult<PublishedLevelBrowser, RemoteStorageEnumerateUserSharedWorkshopFilesResult_t> m_callResultEnumerateUserSharedWorkshopFiles;
	STEAM_CALLBACK_MANUAL(PublishedLevelBrowser, onFileDeleted, RemoteStoragePublishedFileDeleted_t, m_callbackFileDeleted);
	
	// No copying
	PublishedLevelBrowser(const PublishedLevelBrowser&);
	PublishedLevelBrowser& operator=(const PublishedLevelBrowser&);
	
	
	Editor*      m_editor;
	
	Gwen::Controls::ListBox*          m_listLevels;
	
	Gwen::Controls::TextBox*          m_textTitle;
	Gwen::Controls::TextBox*          m_textFilename;
	Gwen::Controls::TextBoxMultiline* m_textDescription;
	
	Gwen::Controls::Button*           m_buttonSave;
	Gwen::Controls::Label*            m_labelLocalCopyStatus;
	
#if 0
	const uint32 m_appID;
	
	Gwen::Controls::TextBox*            m_textPublishTitle;
	Gwen::Controls::TextBoxMultiline*   m_textPublishDesc;
	Gwen::Controls::ComboBox*           m_comboVisibility;
	Gwen::Controls::LabelClickable*     m_labelAgreeToLicense;
	Gwen::Controls::Label*              m_labelStatus;
	tt::gwen::BusyBar*                  m_busyBar;
	Gwen::Controls::Button*             m_buttonPublish;
	Gwen::Controls::Base*               m_previewImagePanel;
	Gwen::Controls::Base*               m_previewImageControl;  // just a dummy control to provide a rectangle to track
	tt::engine::renderer::QuadSpritePtr m_previewImage;
	
	typedef std::vector<std::string> UploadQueue;
	UploadQueue m_uploadQueue;
#endif  // 0
	
	steam::PublishedFileIds m_savingToDisk;  // all file IDs that are currently being downloaded/saved to disk
	
	
	steam::WorkshopObserverPtr m_observer;
};

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD)

#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_PUBLISHEDLEVELBROWSER_H)
