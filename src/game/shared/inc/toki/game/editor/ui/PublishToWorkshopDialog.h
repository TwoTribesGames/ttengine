#if !defined(INC_TOKI_GAME_EDITOR_UI_PUBLISHTOWORKSHOPDIALOG_H)
#define INC_TOKI_GAME_EDITOR_UI_PUBLISHTOWORKSHOPDIALOG_H

#if defined(TT_STEAM_BUILD)


#include <vector>
#include <steam/steam_api.h>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/LabelClickable.h>
#include <Gwen/Controls/TextBox.h>

#include <tt/gwen/BusyBar.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/str/str_types.h>

#include <toki/game/editor/ui/DialogBoxBase.h>
#include <toki/game/editor/fwd.h>
#include <toki/steam/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Dialog that allows a user to publish their level to Steam Workshop. */
class PublishToWorkshopDialog : public DialogBoxBase
{
public:
	GWEN_CONTROL(PublishToWorkshopDialog, DialogBoxBase);
	virtual ~PublishToWorkshopDialog();
	
	static PublishToWorkshopDialog* create(Editor*               p_editor,
	                                       Gwen::Controls::Base* p_parent);
	
	virtual void SetHidden(bool p_hidden);
	
	void levelPlayTestSucceeded();
	void levelPlayTestFailed();  // playtest failed because level has errors
	
	void handleFileDetailsAvailable();
	
	void renderPostGwen();
	
private:
	void createUi();
	void setUiEnabled(bool p_enabled);
	void setLevelPreviewVisible(bool p_visible);
	void doPublishLevel();      // starts the whole "publish to Workshop" process
	void doSubmitToWorkshop();  // only performs the last part of the process: creating a Workshop item
	void publishNewItem();      // creates a new Workshop item unconditionally (not updating an existing one if already published)
	void showPublishError(EResult p_resultCode, bool p_ioFailure);
	void handlePublishResult(EResult p_resultCode, bool p_ioFailure, PublishedFileId_t p_id);
	
	void inputErrorPopup(Gwen::Controls::Base* p_inputControl, const std::string& p_titleID, const std::string& p_textID);
	void errorPopup(const std::string& p_titleID, const std::string& p_textID);
	
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
	
	CCallResult<PublishToWorkshopDialog, RemoteStoragePublishFileResult_t>         m_callResultPublishWorkshopFile;
	CCallResult<PublishToWorkshopDialog, RemoteStorageUpdatePublishedFileResult_t> m_callResultUpdatePublishedFile;
	CCallResult<PublishToWorkshopDialog, RemoteStorageFileShareResult_t>           m_callResultFileShare;
	
	// No copying
	PublishToWorkshopDialog(const PublishToWorkshopDialog&);
	PublishToWorkshopDialog& operator=(const PublishToWorkshopDialog&);
	
	
	Editor*      m_editor;
	const uint32 m_consumerAppID;  // Steam app ID of the app that will consume (use) the published files
	
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
	
	// NOTE: This information has been pre-translated into a format for the Steam API
	struct PublishDetails
	{
		std::string                           filename;
		std::string                           previewFilename;
		std::string                           title;        // NOTE: This is a UTF-8 string
		std::string                           description;  // NOTE: This is a UTF-8 string
		ERemoteStoragePublishedFileVisibility visibility;
		tt::str::StringSet                    tags;
		SteamParamStringArray_t               tagsAsArray;  // set up to point to the 'tags' member
		
		inline PublishDetails()
		:
		filename(),
		previewFilename(),
		title(),
		description(),
		visibility(k_ERemoteStoragePublishedFileVisibilityPublic),
		tags(),
		tagsAsArray()
		{
			tagsAsArray.m_nNumStrings = 0;
			tagsAsArray.m_ppStrings   = 0;
		}
		
		inline ~PublishDetails()
		{
			delete[] tagsAsArray.m_ppStrings;
		}
	};
	PublishDetails m_publishDetails;
	
	typedef std::vector<std::string> UploadQueue;
	UploadQueue m_uploadQueue;
	
	steam::WorkshopObserverPtr m_observer;
};

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD)

#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_PUBLISHTOWORKSHOPDIALOG_H)
