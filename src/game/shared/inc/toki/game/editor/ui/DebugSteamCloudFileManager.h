#if !defined(INC_TOKI_GAME_EDITOR_UI_DEBUGSTEAMCLOUDFILEMANAGER_H)
#define INC_TOKI_GAME_EDITOR_UI_DEBUGSTEAMCLOUDFILEMANAGER_H

#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)


//#include <steam/steam_api.h>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Controls/TextBox.h>
#include <Gwen/Controls/WindowControl.h>

#include <toki/game/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Debug interface for viewing and managing Steam Cloud files. */
class DebugSteamCloudFileManager : public Gwen::Controls::WindowControl
{
public:
	GWEN_CONTROL(DebugSteamCloudFileManager, Gwen::Controls::WindowControl);
	virtual ~DebugSteamCloudFileManager();
	
	static DebugSteamCloudFileManager* create(Editor*               p_editor,
	                                          Gwen::Controls::Base* p_parent);
	
private:
	void createUi();
	Gwen::Controls::TextBox* createDetailsRow(const Gwen::TextObject& p_label,
	                                          Gwen::Controls::Base*   p_parent);
	
	void refreshFileList();
	void onFileSelected();
	void onSaveFileToDisk();
	void onDeleteFile();
	void onUploadFile();
	
	void onConfirmDeleteClosed(Gwen::Controls::Base* p_sender);
	
	void onSaveFilenamePicked(Gwen::Event::Info p_info);
	
	/*
	// Callbacks for asynchronous Steam calls
	void onResultFileShare(RemoteStorageFileShareResult_t* p_result, bool p_ioFailure);
	CCallResult<DebugSteamCloudFileManager, RemoteStorageFileShareResult_t> m_callbackFileShare;
	*/
	
	// No copying
	DebugSteamCloudFileManager(const DebugSteamCloudFileManager&);
	DebugSteamCloudFileManager& operator=(const DebugSteamCloudFileManager&);
	
	
	Editor*                  m_editor;
	
	Gwen::Controls::ListBox* m_listFiles;
	Gwen::Controls::Label*   m_labelTotals;
	Gwen::Controls::Button*  m_buttonSaveToDisk;
	Gwen::Controls::Button*  m_buttonDeleteFile;
	Gwen::Controls::Button*  m_buttonUploadToCloud;
	Gwen::Controls::TextBox* m_textDetailsFilename;
	Gwen::Controls::TextBox* m_textDetailsFileSize;
	Gwen::Controls::TextBox* m_textDetailsModifiedDate;
	Gwen::Controls::TextBox* m_textDetailsSyncPlatforms;
	
	std::string              m_filenameToOperateOn;
};

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)

#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_DEBUGSTEAMCLOUDFILEMANAGER_H)
