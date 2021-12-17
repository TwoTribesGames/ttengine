#if defined(TT_STEAM_BUILD)

#include <algorithm>

#include <Gwen/Controls/Layout/Position.h>
#include <Gwen/Controls/SplitterBar.h>
#include <Gwen/Controls/Splitters.h>

/*
#include <tt/app/Application.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/helpers.h>
*/
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/str/str.h>

#include <toki/game/editor/ui/GenericDialogBox.h>
#include <toki/game/editor/ui/helpers.h>
#include <toki/game/editor/ui/PublishedLevelBrowser.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
/*
#include <toki/game/Game.h>
#include <toki/level/LevelData.h>
*/
#include <toki/steam/Workshop.h>
#include <toki/steam/WorkshopObserver.h>
//#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

// Private implementation helpers. These aren't in the header to keep include dependencies to a minimum.

// FIXME: Code duplication: this path is mentioned in more than one place
static const std::string g_userlevelsDir("userlevels/");


enum LocalCopyStatus
{
	LocalCopyStatus_Unknown,
	LocalCopyStatus_NoLocalCopy,
	LocalCopyStatus_UpToDate,
	LocalCopyStatus_NewerThanWorkshop,
	LocalCopyStatus_OlderThanWorkshop
};


static std::string makeLocalCopyFilename(const steam::Workshop::FileDetails* p_details)
{
	TT_NULL_ASSERT(p_details);
	if (p_details == 0)
	{
		return std::string();
	}
	
	std::string filename(p_details->details.m_pchFileName);
	static const std::string filenamePrefix("workshop_level_");
	if (tt::str::startsWith(filename, filenamePrefix))
	{
		filename.erase(0, filenamePrefix.length());
	}
	else
	{
		TT_WARN("Workshop level filename '%s' does not have expected prefix '%s'!",
		        filename.c_str(), filenamePrefix.c_str());
	}
	
	return filename;
}


static LocalCopyStatus getLocalCopyStatus(const steam::Workshop::FileDetails* p_details)
{
	if (p_details == 0)
	{
		return LocalCopyStatus_Unknown;
	}
	
	const std::string localCopyFilename(g_userlevelsDir + makeLocalCopyFilename(p_details));
	
	if (tt::fs::fileExists(localCopyFilename))
	{
		tt::fs::FilePtr file(tt::fs::open(localCopyFilename, tt::fs::OpenMode_Read));
		if (file == 0)
		{
			return LocalCopyStatus_Unknown;
		}
		else
		{
			const s64 localCopyModified = tt::fs::convertToUnixTime(tt::fs::getWriteTime(file));
			const s64 workshopModified  = static_cast<s64>(p_details->details.m_rtimeUpdated);
			const s64 gracePeriod       = 120;  // grace period (in seconds) before considering local copy older or newer
			
			if ((localCopyModified - gracePeriod) > workshopModified)
			{
				return LocalCopyStatus_NewerThanWorkshop;
			}
			else if ((localCopyModified + gracePeriod) < workshopModified)
			{
				return LocalCopyStatus_OlderThanWorkshop;
			}
			else
			{
				return LocalCopyStatus_UpToDate;
			}
		}
	}
	else
	{
		return LocalCopyStatus_NoLocalCopy;
	}
}


static const steam::Workshop::FileDetails* getSelectedLevelDetails(Gwen::Controls::ListBox* p_list)
{
	TT_NULL_ASSERT(p_list);
	if (p_list == 0)
	{
		return 0;
	}
	
	Gwen::Controls::Layout::TableRow* item = p_list->GetSelectedRow();
	if (item == 0 || item->UserData.Exists("id") == false)
	{
		return 0;
	}
	
	return steam::Workshop::getInstance()->getCachedFileDetails(item->UserData.Get<PublishedFileId_t>("id"));
}


// Helper class so that PublishedLevelBrowser doesn't need to derive from WorkshopObserver
// (because GWEN doesn't work with smart pointers)
class LevelBrowserWorkshopObserver : public steam::WorkshopObserver
{
public:
	static steam::WorkshopObserverPtr create(PublishedLevelBrowser* p_targetDialog)
	{
		TT_NULL_ASSERT(p_targetDialog);
		return steam::WorkshopObserverPtr(new LevelBrowserWorkshopObserver(p_targetDialog));
	}
	
	virtual void onWorkshopFileChange(PublishedFileId_t p_id, steam::WorkshopObserver::FileAction p_action)
	{
		if (m_targetDialog != 0)
		{
			m_targetDialog->onWorkshopFileChange(p_id, p_action);
		}
	}
	
private:
	explicit inline LevelBrowserWorkshopObserver(PublishedLevelBrowser* p_targetDialog)
	:
	steam::WorkshopObserver(),
	m_targetDialog(p_targetDialog)
	{ }
	
	// No copying
	LevelBrowserWorkshopObserver(const LevelBrowserWorkshopObserver&);
	const LevelBrowserWorkshopObserver& operator=(const LevelBrowserWorkshopObserver&);
	
	
	PublishedLevelBrowser* m_targetDialog;
};


//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR(PublishedLevelBrowser)
,
/*
m_callResultPublishWorkshopFile(),
m_callResultUpdatePublishedFile(),
m_callResultFileShare          (),
// */
m_callResultEnumerateUserSharedWorkshopFiles(),
m_callbackFileDeleted (),
m_editor              (0),
m_listLevels          (0),
m_textTitle           (0),
m_textFilename        (0),
m_textDescription     (0),
m_buttonSave          (0),
m_labelLocalCopyStatus(0),
m_savingToDisk        (),
m_observer            ()
{
	SetMinimumSize(Gwen::Point(100, 100));
	SetSize(600, 400);
	SetClosable(true);
	SetDeleteOnClose(false);
	SetHidden(true);
	// FIXME: Localize this!
	SetTitle(L"Published Levels");
	
	m_callbackFileDeleted.Register(this, &PublishedLevelBrowser::onFileDeleted);
}


PublishedLevelBrowser::~PublishedLevelBrowser()
{
	if (m_observer != 0 && steam::Workshop::hasInstance())
	{
		steam::Workshop::getInstance()->unregisterObserver(m_observer);
	}
}


PublishedLevelBrowser* PublishedLevelBrowser::create(Editor*               p_editor,
                                                     Gwen::Controls::Base* p_parent)
{
	TT_NULL_ASSERT(p_editor);
	if (p_editor == 0) return 0;
	
	PublishedLevelBrowser* dlg = new PublishedLevelBrowser(p_parent);
	dlg->m_editor = p_editor;
	dlg->createUi();
	return dlg;
}


void PublishedLevelBrowser::SetHidden(bool p_hidden)
{
	BaseClass::SetHidden(p_hidden);
	
	if (p_hidden == false && m_editor != 0)
	{
		// Only start observing Workshop events the first time this dialog is shown, not upon creation
		if (m_observer == 0)
		{
			m_observer = LevelBrowserWorkshopObserver::create(this);
			steam::Workshop::getInstance()->registerObserver(m_observer);
			
			//* DEBUG: Test the kind of results we get from this API
			SteamAPICall_t handle = SteamRemoteStorage()->EnumerateUserSharedWorkshopFiles(SteamUser()->GetSteamID(), 0, 0, 0);
			m_callResultEnumerateUserSharedWorkshopFiles.Set(handle, this, &PublishedLevelBrowser::onResultEnumerateUserSharedWorkshopFiles);
			// END DEBUG */
		}
		
		// Refresh the list of levels each time the dialog opens
		refreshLevelList();
	}
}


void PublishedLevelBrowser::onWorkshopFileChange(PublishedFileId_t                   p_id,
                                                 steam::WorkshopObserver::FileAction p_action)
{
	switch (p_action)
	{
	case steam::WorkshopObserver::FileAction_DetailsAvailable:
		if (steam::Workshop::getInstance()->isPublishedFile(p_id) &&
		    findItemByUserData(m_listLevels, "id", p_id) == 0)
		{
			// Details are available for a published file that we don't have in the list yet
			const steam::Workshop::FileDetails* details = steam::Workshop::getInstance()->getCachedFileDetails(p_id);
			TT_NULL_ASSERT(details);
			if (details != 0)
			{
				Gwen::Controls::Layout::TableRow* item = m_listLevels->AddItem(
						tt::str::utf8ToUtf16(details->details.m_rgchTitle));
				item->UserData.Set("id", p_id);
			}
		}
		break;
		
	case steam::WorkshopObserver::FileAction_FileDownloadedToCustomLocation:
		// File download was completed... yay
		// FIXME: Add error handling + update UI
		{
			steam::PublishedFileIds::iterator it = std::find(m_savingToDisk.begin(), m_savingToDisk.end(), p_id);
			if (it != m_savingToDisk.end())
			{
				m_savingToDisk.erase(it);
			}
		}
		break;
		
	default:
		// Not interested in this action
		break;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void PublishedLevelBrowser::createUi()
{
	TT_NULL_ASSERT(m_editor);
	
	using namespace Gwen::Controls;
	
	m_listLevels = new ListBox(this);
	m_listLevels->Dock(Gwen::Pos::Fill);
	m_listLevels->onRowSelected.Add(this, &PublishedLevelBrowser::onLevelSelected);
	
	
	Base* panelDetails = new Base(this);
	panelDetails->Dock(Gwen::Pos::Fill);
	
	// FIXME: Localize all of this!
	m_textTitle    = createDetailsRow(L"Title:",    panelDetails);
	m_textFilename = createDetailsRow(L"Filename:", panelDetails);
	
	{
		Label* label = new Label(panelDetails);
		label->SetText(L"Description:");
		label->SizeToContents();
		label->SetMargin(Gwen::Margin(0, 0, 0, 0));
		label->Dock(Gwen::Pos::Top);
		
		m_textDescription = new TextBoxMultiline(panelDetails);
		m_textDescription->SetMargin(Gwen::Margin(0, 0, 0, 5));
		m_textDescription->Dock(Gwen::Pos::Fill);
		m_textDescription->SetDisabled(true);
	}
	
	m_buttonSave = new Button(panelDetails);
	m_buttonSave->SetText(L"Save To Userlevels");
	m_buttonSave->SetHeight(20);
	m_buttonSave->Dock(Gwen::Pos::Bottom);
	m_buttonSave->onPress.Add(this, &PublishedLevelBrowser::onSaveToDisk);
	
	m_labelLocalCopyStatus = new Label(panelDetails);
	m_labelLocalCopyStatus->SetWrap(true);
	m_labelLocalCopyStatus->SetWidth(Width() - m_listLevels->Width());
	m_labelLocalCopyStatus->SetMargin(Gwen::Margin(0, 0, 0, 5));
	m_labelLocalCopyStatus->Dock(Gwen::Pos::Bottom);
	
	// Stick the interface in a splitter (list on the left, details on the right)
	SplitterHorizontal* splitter = new SplitterHorizontal(this);
	splitter->Dock(Gwen::Pos::Fill);
	splitter->SetScaling(false, 200);
	splitter->SetPanels(m_listLevels, panelDetails);
	
	/*
	// Pre-populate the controls with values
	const u64 publishedId = m_editor->getLevelData()->getPublishedFileId();
	if (publishedId != 0 && steam::Workshop::getInstance()->isPublishedFile(publishedId))
	{
		// Level has been published before: fill in the previous publish details
		typedef steam::Workshop::FileDetails FileDetails;
		const FileDetails* fileDetails(steam::Workshop::getInstance()->getCachedFileDetails(publishedId));
		if (fileDetails != 0 &&
		    fileDetails->validMask.checkFlag(FileDetails::ValidDetails_File) &&
		    fileDetails->keepRefreshingUntilNewerThan == 0)
		{
			// Already have details available: fill them in right away
			const RemoteStorageGetPublishedFileDetailsResult_t& details(fileDetails->details);
			m_textPublishTitle->SetText(tt::str::utf8ToUtf16(details.m_rgchTitle));
			m_textPublishDesc ->SetText(tt::str::utf8ToUtf16(details.m_rgchDescription));
			m_comboVisibility->SelectItemByName("vis_" + tt::str::toStr(details.m_eVisibility));
		}
		else
		{
			// Details weren't available yet: request them, so that we can get them later
			m_labelStatus->SetText(translateString("WINDOW_PUBLISH_STATUS_GET_DETAILS"));
			setUiEnabled(false);
			SetClosable(true);
			m_observer = DetailsObserver::create(this, publishedId);
			steam::Workshop::getInstance()->registerObserver(m_observer);
			steam::Workshop::getInstance()->requestFileDetails(publishedId);
		}
	}
	// */
}


Gwen::Controls::TextBox* PublishedLevelBrowser::createDetailsRow(
		const Gwen::TextObject& p_label,
		Gwen::Controls::Base*   p_parent)
{
	using namespace Gwen::Controls;
	
	Label* label = new Label(p_parent);
	label->SetText(p_label);
	label->SizeToContents();
	label->SetMargin(Gwen::Margin(0, 0, 0, 0));
	label->Dock(Gwen::Pos::Top);
	
	TextBox* value = new TextBox(p_parent);
	value->SetHeight(20);
	value->SetMargin(Gwen::Margin(0, 0, 0, 10));
	value->Dock(Gwen::Pos::Top);
	value->SetDisabled(true);
	
	return value;
}


void PublishedLevelBrowser::refreshLevelList()
{
	using Gwen::Controls::Layout::TableRow;
	using steam::PublishedFileIds;
	using steam::Workshop;
	
	PublishedFileId_t selectedId = 0;
	if (m_listLevels->GetSelectedRow() != 0 &&
	    m_listLevels->GetSelectedRow()->UserData.Exists("id"))
	{
		selectedId = m_listLevels->GetSelectedRow()->UserData.Get<PublishedFileId_t>("id");
	}
	
	m_listLevels->Clear();
	
	TableRow* firstItem = 0;
	
	Workshop* workshop = Workshop::getInstance();
	const PublishedFileIds& publishedFiles(workshop->getPublishedFiles());
	for (PublishedFileIds::const_iterator it = publishedFiles.begin(); it != publishedFiles.end(); ++it)
	{
		const Workshop::FileDetails* details = workshop->getCachedFileDetails(*it);
		if (details != 0)
		{
			TableRow* item = m_listLevels->AddItem(tt::str::utf8ToUtf16(details->details.m_rgchTitle));
			item->UserData.Set("id", *it);
			
			if (*it == selectedId)
			{
				m_listLevels->SetSelectedRow(item);
			}
			
			if (firstItem == 0)
			{
				firstItem = item;
			}
		}
	}
	
	// If the selection could not be restored, select the first item
	if (m_listLevels->GetSelectedRow() == 0)
	{
		if (firstItem != 0)
		{
			m_listLevels->SetSelectedRow(firstItem);
		}
		else
		{
			// Have nothing to select: update details panel for no selection
			onLevelSelected();
		}
	}
}


void PublishedLevelBrowser::onLevelSelected()
{
	const steam::Workshop::FileDetails* details = getSelectedLevelDetails(m_listLevels);
	
	if (details == 0)
	{
		m_textTitle           ->SetText(L"");
		m_textFilename        ->SetText(L"");
		m_textDescription     ->SetText(L"");
		m_buttonSave          ->SetDisabled(true);
		m_labelLocalCopyStatus->SetHidden(true);
		m_labelLocalCopyStatus->SetText(L"");
	}
	else
	{
		const std::string filename(makeLocalCopyFilename(details));
		
		m_textTitle      ->SetText(tt::str::utf8ToUtf16(details->details.m_rgchTitle));
		m_textFilename   ->SetText(tt::fs::utils::getFileTitle(filename));
		m_textDescription->SetText(tt::str::utf8ToUtf16(details->details.m_rgchDescription));
		
		m_buttonSave->SetDisabled(false);
		
		const LocalCopyStatus status = getLocalCopyStatus(details);
		std::wstring statusText;
		switch (status)
		{
		case LocalCopyStatus_Unknown:
			statusText = L"Could not get time stamp of local copy to compare to the Steam Workshop version.";
			break;
			
		case LocalCopyStatus_NoLocalCopy:
			statusText = L"You don't have a local copy of this level on this computer.";
			break;
			
		case LocalCopyStatus_UpToDate:
			statusText = L"Your local copy of this level is up to date.";
			break;
			
		case LocalCopyStatus_NewerThanWorkshop:
			statusText = L"Your local copy of this level is newer than the Steam Workshop version.";
			break;
			
		case LocalCopyStatus_OlderThanWorkshop:
			statusText = L"The Steam Workshop version of this level is newer than your local copy.";
			break;
			
		default:
			TT_PANIC("Unsupported local copy status: %d", status);
			break;
		}
		
		m_labelLocalCopyStatus->SetText(statusText);
		m_labelLocalCopyStatus->SetHidden(false);
		m_labelLocalCopyStatus->SizeToContents();
	}
}


void PublishedLevelBrowser::onSaveToDisk()
{
	const steam::Workshop::FileDetails* details = getSelectedLevelDetails(m_listLevels);
	if (details == 0)
	{
		return;
	}
	
	const PublishedFileId_t id = details->details.m_nPublishedFileId;
	
	// Extra sanity check: do not allow downloading if already downloading
	// (button should be disabled in this case, so this shouldn't happen)
	if (std::find(m_savingToDisk.begin(), m_savingToDisk.end(), id) != m_savingToDisk.end())
	{
		return;
	}
	
	const std::string     localCopyFilename(makeLocalCopyFilename(details));
	const LocalCopyStatus status          = getLocalCopyStatus(details);
	const bool            localCopyExists = tt::fs::fileExists(g_userlevelsDir + localCopyFilename);
	
	/*
	if (localCopyExists &&
	    m_editor->hasUnsavedChanges() &&
	    tt::str::endsWith(m_editor->getCurrentLevelInfo().getLevelFilePath(), localCopyFilename))
	{
		// The selected level is currently open in the editor and the user has made unsaved changes to it:
		// ask what to do with these changes
		//
	}
	else */ if (status == LocalCopyStatus_NewerThanWorkshop)
	{
		tt::str::StringFormatter formatter(
				L"Your local copy of level '$VAR1$' (filename '$VAR2$') is newer than the Steam Workshop version.\n"
				L"Saving the Workshop version to disk will revert all your local changes.\n"
				L"Are you sure you want to do this? This action cannot be undone.");
		formatter << tt::str::utf8ToUtf16(details->details.m_rgchTitle) << localCopyFilename;
		
		showSaveConfirmation(L"Overwrite Local Copy?", formatter.getResult(), id);
	}
	else if (localCopyExists)
	{
		tt::str::StringFormatter formatter(
				L"Are you sure you want to overwrite your local copy of level '$VAR1$' "
				L"(filename '$VAR2$') with the Steam Workshop version?\n"
				L"This action cannot be undone.");
		formatter << tt::str::utf8ToUtf16(details->details.m_rgchTitle) << localCopyFilename;
		
		showSaveConfirmation(L"Overwrite Local Copy?", formatter.getResult(), id);
	}
	else
	{
		saveLocalCopy(id);
	}
}


void PublishedLevelBrowser::onSaveConfirmationClosed(Gwen::Controls::Base* p_sender)
{
	GenericDialogBox* dialog = gwen_cast<GenericDialogBox>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0 || dialog->getResult() != GenericDialogBox::Result_Yes)
	{
		return;
	}
	
	TT_ASSERT(dialog->UserData.Exists("id"));
	saveLocalCopy(dialog->UserData.Get<PublishedFileId_t>("id"));
}


void PublishedLevelBrowser::showSaveConfirmation(const std::wstring& p_title,
                                                 const std::wstring& p_prompt,
                                                 PublishedFileId_t   p_id)
{
	GenericDialogBox* dlg = m_editor->showGenericDialog(p_title, p_prompt, DialogButtons_YesNo);
	if (dlg != 0)
	{
		dlg->SetSize(500, 140);
		m_editor->centerInScreen(dlg);
		dlg->UserData.Set("id", p_id);
		dlg->onWindowClosed.Add(this, &PublishedLevelBrowser::onSaveConfirmationClosed);
	}
}


void PublishedLevelBrowser::saveLocalCopy(PublishedFileId_t p_id)
{
	(void)p_id;
	TT_Printf("\nSaving ALL teh levels! Published file ID %llu.\n\n", p_id);
	/*
	if (steam::Workshop::getInstance()->requestFileDownloadToLocation(id, path, fsid))
	{
		m_savingToDisk.push_back(p_id);
	}
	*/
	//m_listLevels->SetDisabled(true);
}


#if 0

void PublishedLevelBrowser::setUiEnabled(bool p_enabled)
{
	m_textPublishTitle->SetDisabled(p_enabled == false);
	m_textPublishDesc ->SetDisabled(p_enabled == false);
	m_comboVisibility ->SetDisabled(p_enabled == false);
	m_buttonPublish   ->SetHidden  (p_enabled == false);
	m_labelAgreeToLicense->SetHidden(p_enabled == false);
	m_labelStatus     ->SetHidden  (p_enabled);
	m_busyBar         ->SetHidden  (p_enabled);
	SetClosable(p_enabled);
}


void PublishedLevelBrowser::doPublishLevel()
{
	Gwen::UnicodeString title(m_textPublishTitle->GetText().GetUnicode());
	Gwen::UnicodeString desc (m_textPublishDesc ->GetText().GetUnicode());
	TT_ASSERT(title.empty() == false);
	TT_ASSERT(desc.empty() == false);
	
	setUiEnabled(false);
	setLevelPreviewVisible(true);
	
	// The title isn't allowed to have any newlines
	tt::str::replace(title, L"\r", L"");
	tt::str::replace(title, L"\n", L"");
	
	// Convert Windows line endings in the description to just newlines
	// (Windows line endings can end up in the text box if the user pastes text from elsewhere)
	tt::str::replace(desc, L"\r\n", L"\n");
	tt::str::replace(desc, L"\r",   L"");  // remove stray carriage returns
	
	// Store the information for the current item to publish
	TT_ASSERT(m_publishDetails.filename.empty() == false);
	TT_ASSERT(m_publishDetails.previewFilename.empty() == false);
	m_publishDetails.title       = tt::str::utf16ToUtf8(title);
	m_publishDetails.description = tt::str::utf16ToUtf8(desc);
	m_publishDetails.visibility  = k_ERemoteStoragePublishedFileVisibilityPublic;
	
	TT_NULL_ASSERT(m_comboVisibility->GetSelectedItem());
	if (m_comboVisibility->GetSelectedItem() != 0)
	{
		TT_ASSERT(m_comboVisibility->GetSelectedItem()->UserData.Exists("visibility"));
		m_publishDetails.visibility = m_comboVisibility->GetSelectedItem()->UserData.Get<
				ERemoteStoragePublishedFileVisibility>("visibility");
	}
	
	// Force an upload of the files to Steam Cloud
	TT_ASSERT(m_uploadQueue.empty());
	m_uploadQueue.push_back(m_publishDetails.filename);
	m_uploadQueue.push_back(m_publishDetails.previewFilename);
	
	triggerFileUpload(m_uploadQueue.front());
}


void PublishedLevelBrowser::doSubmitToWorkshop()
{
	level::LevelDataPtr levelData = m_editor->getLevelData();
	if (levelData->getPublishedFileId() != 0)
	{
		// The level was already published before: update the published details
		ISteamRemoteStorage* cloud = SteamRemoteStorage();
		
		PublishedFileUpdateHandle_t updateHandle =
				cloud->CreatePublishedFileUpdateRequest(levelData->getPublishedFileId());
		if (updateHandle == k_PublishedFileUpdateHandleInvalid)
		{
			// FIXME: What are the reasons this could fail? For now, assume the cached
			//        file ID is no longer valid, so always upload a new item
			publishNewItem();
			return;
		}
		
		bool ok = true;
		ok = ok && cloud->UpdatePublishedFileFile       (updateHandle, m_publishDetails.filename.c_str());
		ok = ok && cloud->UpdatePublishedFilePreviewFile(updateHandle, m_publishDetails.previewFilename.c_str());
		ok = ok && cloud->UpdatePublishedFileTitle      (updateHandle, m_publishDetails.title.c_str());
		ok = ok && cloud->UpdatePublishedFileDescription(updateHandle, m_publishDetails.description.c_str());
		ok = ok && cloud->UpdatePublishedFileVisibility (updateHandle, m_publishDetails.visibility);
		ok = ok && cloud->UpdatePublishedFileTags       (updateHandle, &m_publishDetails.tagsAsArray);
		
		if (ok)
		{
			m_labelStatus->SetText(translateString("WINDOW_PUBLISH_STATUS_PUBLISH_UPDATE"));
			
			SteamAPICall_t handle = cloud->CommitPublishedFileUpdate(updateHandle);
			m_callResultUpdatePublishedFile.Set(handle, this,
					&PublishedLevelBrowser::onResultUpdatePublishedFile);
		}
		else
		{
			setUiEnabled(true);
			setLevelPreviewVisible(false);
			
			ui::GenericDialogBox* dlg = m_editor->showGenericDialog(
					translateString("WINDOW_PUBLISH_PUBLISH_FAILED_TITLE"),
					translateString("WINDOW_PUBLISH_PUBLISH_FAILED_UPDATE_PROMPT"),
					DialogButtons_RetryCancel);
			dlg->onWindowClosed.Add(this, &PublishedLevelBrowser::onPublishErrorDialogClosed);
		}
	}
	else
	{
		// This is the first time this level is being published
		publishNewItem();
	}
}


void PublishedLevelBrowser::publishNewItem()
{
	m_labelStatus->SetText(translateString("WINDOW_PUBLISH_STATUS_PUBLISH_NEW"));
	
	SteamAPICall_t handle = SteamRemoteStorage()->PublishWorkshopFile(
			m_publishDetails.filename.c_str(),
			m_publishDetails.previewFilename.c_str(),
			m_appID,
			m_publishDetails.title.c_str(),
			m_publishDetails.description.c_str(),
			m_publishDetails.visibility,
			&m_publishDetails.tagsAsArray,
			k_EWorkshopFileTypeCommunity);
	m_callResultPublishWorkshopFile.Set(handle, this, &PublishedLevelBrowser::onResultPublishWorkshopFile);
}


void PublishedLevelBrowser::handlePublishResult(
		EResult           p_resultCode,
		bool              p_ioFailure,
		PublishedFileId_t p_id)
{
	setUiEnabled(true);
	setLevelPreviewVisible(false);
	
	if (p_ioFailure || p_resultCode != k_EResultOK)
	{
		showPublishError(p_resultCode, p_ioFailure);
	}
	else
	{
		// Embed the published file ID in the level data, so that we can know whether this file was published before
		m_editor->getLevelData()->setPublishedFileId(p_id);
		m_editor->saveLevel();
		
		// Notify our back-end that a file was published
		steam::Workshop::getInstance()->notifyFileWasPublished(p_id);
		
		// Dismiss the publish dialog and open the Steam overlay to show the newly published item
		CloseButtonPressed();
		tt::steam::openURL("http://steamcommunity.com/sharedfiles/filedetails/?id=" +
		                   tt::str::toStr(p_id));
	}
}


void PublishedLevelBrowser::triggerFileUpload(const std::string& p_filename)
{
	// Show the current publish progress (bit hacky...)
	if (p_filename == m_publishDetails.filename)
	{
		m_labelStatus->SetText(translateString("WINDOW_PUBLISH_STATUS_UPLOAD_LEVEL"));
	}
	else if (p_filename == m_publishDetails.previewFilename)
	{
		m_labelStatus->SetText(translateString("WINDOW_PUBLISH_STATUS_UPLOAD_PREVIEW"));
	}
	
	// Tell Steam to upload the file to Cloud
	SteamAPICall_t handle = SteamRemoteStorage()->FileShare(p_filename.c_str());
	m_callResultFileShare.Set(handle, this, &PublishedLevelBrowser::onResultFileShare);
}


void PublishedLevelBrowser::onPublishLevel()
{
	// Validate the user's input
	const Gwen::UnicodeString& title(m_textPublishTitle->GetText().GetUnicode());
	const Gwen::UnicodeString& desc (m_textPublishDesc ->GetText().GetUnicode());
	if (tt::str::isEmptyOrWhitespace(title))
	{
		inputErrorPopup(m_textPublishTitle, "WINDOW_PUBLISH_NOTITLE_TITLE", "WINDOW_PUBLISH_NOTITLE_PROMPT");
		return;
	}
	if (tt::str::isEmptyOrWhitespace(desc))
	{
		inputErrorPopup(m_textPublishDesc, "WINDOW_PUBLISH_NODESCRIPTION_TITLE", "WINDOW_PUBLISH_NODESCRIPTION_PROMPT");
		return;
	}
	
	// The user must prove the level can be completed by playtesting the level.
	// Before starting playtest, show a dialog box telling the user what is about to happen
	GenericDialogBox* dlg = m_editor->showGenericDialog(
			translateString("WINDOW_PUBLISH_PROVE_PLAYABLE_TITLE"),
			translateString("WINDOW_PUBLISH_PROVE_PLAYABLE_PROMPT"),
			DialogButtons_OKCancel);
	dlg->SetSize(500, 130);
	m_editor->centerInScreen(dlg);
	dlg->onWindowClosed.Add(this, &PublishedLevelBrowser::onConfirmPlaytestDialogClosed);
}


void PublishedLevelBrowser::onInputErrorDialogClosed(Gwen::Controls::Base* p_sender)
{
	TT_NULL_ASSERT(p_sender);
	Gwen::Controls::Base* focusControl = p_sender->UserData.Get<Gwen::Controls::Base*>("focusControl");
	if (focusControl != 0)
	{
		focusControl->Focus();
	}
}


void PublishedLevelBrowser::onPublishErrorDialogClosed(Gwen::Controls::Base* p_sender)
{
	DialogBoxBase* dialog = gwen_cast<DialogBoxBase>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0 || dialog->getResult() != Result_Retry)
	{
		return;
	}
	
	doPublishLevel();
}


void PublishedLevelBrowser::onConfirmPlaytestDialogClosed(Gwen::Controls::Base* p_sender)
{
	DialogBoxBase* dialog = gwen_cast<DialogBoxBase>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0 || dialog->getResult() != Result_OK)
	{
		return;
	}
	
	// Reset the publish details
	m_publishDetails = PublishDetails();
	
	// Before starting playtest, save the publish data to Steam Cloud so that save errors can be caught early
	const tt::fs::identifier steamCloudFS = tt::app::getApplication()->getSaveFsID();
	const std::string prefix("workshop_level_" + m_editor->getCurrentLevelName());
	const std::string filename       (prefix + ".ttlvl");
	const std::string previewFilename(prefix + "_preview.png");
	
	// - Save the level to Steam Cloud
	// NOTE: No need to update pathmgr in leveldata, because level was already saved before publish dialog was opened
	level::LevelDataPtr levelData(m_editor->getLevelData());
	if (levelData->save(filename, steamCloudFS) == false)
	{
		errorPopup("WINDOW_PUBLISH_SAVE_FAILED_TITLE", "WINDOW_PUBLISH_SAVE_FAILED_LEVEL_PROMPT");
		return;
	}
	
	m_publishDetails.filename        = filename;
	m_publishDetails.previewFilename = previewFilename;
	
	// Gather the Workshop tags for all the entities that are in the level
	{
		tt::str::StringSet entityTypesInLevel;
		
		const level::entity::EntityInstances& entities(levelData->getAllEntities());
		for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
		{
			TT_NULL_ASSERT(*it);
			if (*it != 0)
			{
				entityTypesInLevel.insert((*it)->getType());
			}
		}
		
		TT_ASSERT(m_publishDetails.tags.empty());
		
		const entity::EntityLibrary& entityLib(AppGlobal::getEntityLibrary());
		for (tt::str::StringSet::iterator it = entityTypesInLevel.begin(); it != entityTypesInLevel.end(); ++it)
		{
			const level::entity::EntityInfo* info = entityLib.getEntityInfo(*it);
			if (info != 0 && info->getWorkshopTags().empty() == false)
			{
				m_publishDetails.tags.insert(info->getWorkshopTags().begin(), info->getWorkshopTags().end());
			}
		}
		
		// Set up the SteamParamStringArray_t to point into the tags container
		TT_ASSERT(m_publishDetails.tagsAsArray.m_nNumStrings == 0);
		TT_ASSERT(m_publishDetails.tagsAsArray.m_ppStrings == 0);
		
		m_publishDetails.tagsAsArray.m_nNumStrings = static_cast<int32>(m_publishDetails.tags.size());
		m_publishDetails.tagsAsArray.m_ppStrings   = new const char*[m_publishDetails.tagsAsArray.m_nNumStrings];
		
		s32 tagIndex = 0;
		for (tt::str::StringSet::iterator it = m_publishDetails.tags.begin();
		     it != m_publishDetails.tags.end(); ++it, ++tagIndex)
		{
			m_publishDetails.tagsAsArray.m_ppStrings[tagIndex] = (*it).c_str();
		}
	}
	
	// Tell the game to take a screenshot for the Workshop preview image
	ScreenshotSettings settings;
	settings.type       = ScreenshotType_LevelPreview;
	settings.width      = 637;
	settings.height     = 358;
	settings.filename   = previewFilename;
	settings.fileSystem = steamCloudFS;
	settings.failureFallbackImage = "workshop_temp_preview_image.png";
	AppGlobal::getGame()->scheduleScreenshot(settings, 1.0f);
	
	// Data was saved to Steam Cloud: time to start the playtest!
	m_editor->startPlayTest();
}


void PublishedLevelBrowser::onResultFileShare(RemoteStorageFileShareResult_t* p_result, bool p_ioFailure)
{
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		setUiEnabled(true);
		setLevelPreviewVisible(false);
		m_uploadQueue.clear();
		showPublishError(p_result->m_eResult, p_ioFailure);
		return;
	}
	
	TT_ASSERT(m_uploadQueue.empty() == false);
	m_uploadQueue.erase(m_uploadQueue.begin());
	
	if (m_uploadQueue.empty())
	{
		// All file have been uploaded to Steam Cloud: now we can publish it on Steam Workshop
		doSubmitToWorkshop();
	}
	else
	{
		// Continue with the next file to upload
		triggerFileUpload(m_uploadQueue.front());
	}
}

#endif  // 0


void PublishedLevelBrowser::onResultEnumerateUserSharedWorkshopFiles(
		RemoteStorageEnumerateUserSharedWorkshopFilesResult_t* p_result,
		bool                                                   p_ioFailure)
{
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		TT_PANIC("Could not retrieve user's published files.\n"
		         "I/O failure: %s\nResult: %d",
		         p_ioFailure ? "yes" : "no", p_result->m_eResult);
		return;
	}
	
	/*
	if (p_result->m_nResultsReturned > 0)
	{
		m_publishedFiles.reserve(static_cast<PublishedFileIds::size_type>(p_result->m_nResultsReturned));
		m_publishedFiles.assign(p_result->m_rgPublishedFileId, p_result->m_rgPublishedFileId + p_result->m_nResultsReturned);
	}
	*/
	//EWorkshopFileAction action;
	//k_EWorkshopFileActionPlayed = 0,
	//k_EWorkshopFileActionCompleted = 1,
	//SteamRemoteStorage()->SetUserPublishedFileAction(id, action);
	TT_Printf("PublishedLevelBrowser::onResultEnumerateUserSharedWorkshopFiles: Got %d files:\n",
	          p_result->m_nResultsReturned);
	steam::Workshop* workshop = steam::Workshop::getInstance();
	for (int32 i = 0; i < p_result->m_nResultsReturned; ++i)
	{
		const PublishedFileId_t id = p_result->m_rgPublishedFileId[i];
		const steam::Workshop::FileDetails* details = workshop->getCachedFileDetails(id);
		//TT_Printf("PublishedLevelBrowser::onResultEnumerateUserSharedWorkshopFiles: - %llu\n", id);
		if (details != 0)
		{
			TT_Printf("PublishedLevelBrowser::onResultEnumerateUserSharedWorkshopFiles: - %llu ('%s'), creator %u, consumer %u\n",
			          id, details->details.m_rgchTitle, details->details.m_nCreatorAppID, details->details.m_nConsumerAppID);
		}
		else
		{
			TT_Printf("PublishedLevelBrowser::onResultEnumerateUserSharedWorkshopFiles: - %llu (details pending)\n", id);
			workshop->requestFileDetails(id);
		}
	}
}


void PublishedLevelBrowser::onFileDeleted(RemoteStoragePublishedFileDeleted_t* p_details)
{
	if (m_listLevels == 0)
	{
		return;
	}
	
	// If this file was in the list, remove it
	Gwen::Controls::Layout::TableRow* item = findItemByUserData(m_listLevels, "id", p_details->m_nPublishedFileId);
	if (item != 0)
	{
		removeItemKeepingSelection(m_listLevels, item);
	}
}

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD)
