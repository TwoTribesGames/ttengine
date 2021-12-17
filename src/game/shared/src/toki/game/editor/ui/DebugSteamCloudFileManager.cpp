#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)

#include <ctime>

#include <steam/steam_api.h>

#include <Gwen/Controls/Dialogs/FileOpen.h>
#include <Gwen/Controls/Dialogs/FileSave.h>
#include <Gwen/Controls/Layout/Position.h>
#include <Gwen/Controls/GroupBox.h>
#include <Gwen/Controls/Label.h>

#include <tt/app/Application.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/game/editor/ui/DebugSteamCloudFileManager.h>
#include <toki/game/editor/ui/GenericDialogBox.h>
#include <toki/game/editor/Editor.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR(DebugSteamCloudFileManager)
,
m_editor(0),
m_listFiles(0),
m_labelTotals(0),
m_buttonSaveToDisk(0),
m_buttonDeleteFile(0),
m_buttonUploadToCloud(0),
m_textDetailsFilename(0),
m_textDetailsFileSize(0),
m_textDetailsModifiedDate(0),
m_filenameToOperateOn()
{
	SetTitle("Steam Cloud File Manager");
	SetMinimumSize(Gwen::Point(50, 50));
	SetSize(650, 500);
	SetClosable(true);
	SetDeleteOnClose(true);
}


DebugSteamCloudFileManager::~DebugSteamCloudFileManager()
{
}


DebugSteamCloudFileManager* DebugSteamCloudFileManager::create(Editor*               p_editor,
                                                               Gwen::Controls::Base* p_parent)
{
	TT_NULL_ASSERT(p_editor);
	if (p_editor == 0) return 0;
	
	DebugSteamCloudFileManager* dlg = new DebugSteamCloudFileManager(p_parent);
	dlg->m_editor = p_editor;
	dlg->createUi();
	return dlg;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void DebugSteamCloudFileManager::createUi()
{
	TT_NULL_ASSERT(m_editor);
	
	using namespace Gwen::Controls;
	
	// File list on the left size of the dialog:
	{
		Base* panelList = new Base(this);
		panelList->SetWidth(250);
		panelList->Dock(Gwen::Pos::Left);
		
		Label* labelFiles = new Label(panelList);
		labelFiles->SetText(L"Files In Steam Cloud:");
		labelFiles->SizeToContents();
		labelFiles->Dock(Gwen::Pos::Top);
		
		m_listFiles = new ListBox(panelList);
		m_listFiles->SetMargin(Gwen::Margin(0, 2, 0, 0));
		m_listFiles->Dock(Gwen::Pos::Fill);
		m_listFiles->onRowSelected.Add(this, &DebugSteamCloudFileManager::onFileSelected);
		
		Button* buttonRefresh = new Button(panelList);
		buttonRefresh->SetText(L"Refresh");
		buttonRefresh->SetMargin(Gwen::Margin(0, 5, 0, 0));
		buttonRefresh->SetHeight(20);
		buttonRefresh->Dock(Gwen::Pos::Bottom);
		buttonRefresh->onPress.Add(this, &DebugSteamCloudFileManager::refreshFileList);
		
		m_labelTotals = new Label(panelList);
		m_labelTotals->SetText(L"Totals");
		m_labelTotals->SizeToContents();
		m_labelTotals->Dock(Gwen::Pos::Bottom);
	}
	
	Base* panelRight = new Base(this);
	panelRight->Dock(Gwen::Pos::Fill);
	
	// Details of the selected file:
	{
		GroupBox* panelDetails = new GroupBox(panelRight);
		panelDetails->SetMargin(Gwen::Margin(5, 0, 0, 0));
		Gwen::Padding detailsPadding(panelDetails->GetPadding());
		detailsPadding.left   += 5;
		detailsPadding.bottom += 5;
		detailsPadding.right  += 5;
		panelDetails->SetPadding(detailsPadding);
		
		panelDetails->SetText("Selected File Details");
		panelDetails->Dock(Gwen::Pos::Fill);
		
		m_textDetailsFilename      = createDetailsRow(L"Filename:",       panelDetails);
		m_textDetailsFileSize      = createDetailsRow(L"Size:",           panelDetails);
		m_textDetailsModifiedDate  = createDetailsRow(L"Modified:",       panelDetails);
		m_textDetailsSyncPlatforms = createDetailsRow(L"Sync Platforms:", panelDetails);
		
		Base* panelButtons = new Base(panelDetails);
		panelButtons->SetHeight(20);
		panelButtons->Dock(Gwen::Pos::Bottom);
		
		m_buttonSaveToDisk = new Button(panelButtons);
		m_buttonSaveToDisk->SetText(L"Save To Disk...");
		m_buttonSaveToDisk->SetWidth(150);
		m_buttonSaveToDisk->Dock(Gwen::Pos::Left);
		m_buttonSaveToDisk->onPress.Add(this, &DebugSteamCloudFileManager::onSaveFileToDisk);
		
		m_buttonDeleteFile = new Button(panelButtons);
		m_buttonDeleteFile->SetText(L"Delete...");
		m_buttonDeleteFile->SetWidth(100);
		m_buttonDeleteFile->Dock(Gwen::Pos::Right);
		m_buttonDeleteFile->onPress.Add(this, &DebugSteamCloudFileManager::onDeleteFile);
	}
	
	// Upload button for adding file to Cloud:
	{
		Layout::Position* panelUpload = new Layout::Position(panelRight);
		panelUpload->SetPosition(Gwen::Pos::Right);
		panelUpload->SetMargin(Gwen::Margin(0, 5, 0, 0));
		panelUpload->SetHeight(20);
		panelUpload->Dock(Gwen::Pos::Bottom);
		
		m_buttonUploadToCloud = new Button(panelUpload);
		m_buttonUploadToCloud->SetWidth(200);
		m_buttonUploadToCloud->SetText(L"Upload File To Steam Cloud...");
		m_buttonUploadToCloud->onPress.Add(this, &DebugSteamCloudFileManager::onUploadFile);
		
		// Mark "Upload" as unavailable (because it isn't implemented yet)
		m_buttonUploadToCloud->SetDisabled(true);
		m_buttonUploadToCloud->SetToolTip("This feature hasn't been implemented yet.");
	}
	
	refreshFileList();
}


Gwen::Controls::TextBox* DebugSteamCloudFileManager::createDetailsRow(
		const Gwen::TextObject& p_label,
		Gwen::Controls::Base*   p_parent)
{
	using namespace Gwen::Controls;
	
	Label* label = new Label(p_parent);
	label->SetText(p_label);
	label->SizeToContents();
	label->SetMargin(Gwen::Margin(0, 5, 0, 0));
	label->Dock(Gwen::Pos::Top);
	
	TextBox* value = new TextBox(p_parent);
	value->SetHeight(20);
	value->SetMargin(Gwen::Margin(0, 0, 0, 5));
	value->Dock(Gwen::Pos::Top);
	value->SetDisabled(true);
	
	return value;
}


void DebugSteamCloudFileManager::refreshFileList()
{
	const std::string selectedFile(m_listFiles->GetSelectedRowName());
	
	m_listFiles->Clear();
	
	ISteamRemoteStorage* cloud = SteamRemoteStorage();
	
	const int32        fileCount        = cloud->GetFileCount();
	int32              totalSizeInBytes = 0;
	tt::str::StringSet filenames;
	
	// Create a sorted list of Cloud filenames
	for (int32 i = 0; i < fileCount; ++i)
	{
		int32       fileSize = 0;
		const char* filename = cloud->GetFileNameAndSize(i, &fileSize);
		totalSizeInBytes += fileSize;
		
		filenames.insert(filename);
	}
	
	// Add a listbox item for each file
	for (tt::str::StringSet::iterator it = filenames.begin(); it != filenames.end(); ++it)
	{
		Gwen::Controls::Layout::TableRow* item = m_listFiles->AddItem(*it, *it);
		if (*it == selectedFile)
		{
			m_listFiles->SetSelectedRow(item);
		}
	}
	
	// Update the total file size (and file count)
	m_labelTotals->SetText(Gwen::Utility::Format(L"Total: %.2f KB in %d files",
	                                             totalSizeInBytes / 1024.0f, fileCount));
	
	if (m_listFiles->GetSelectedRow() == 0)
	{
		// Refresh details for "no file selected"
		onFileSelected();
	}
}


void DebugSteamCloudFileManager::onFileSelected()
{
	TT_NULL_ASSERT(m_listFiles);
	Gwen::Controls::Layout::TableRow* row = m_listFiles->GetSelectedRow();
	if (row == 0 || row->GetName().empty())
	{
		m_buttonSaveToDisk->SetDisabled(true);
		m_buttonDeleteFile->SetDisabled(true);
		m_textDetailsFilename     ->SetText(L"");
		m_textDetailsFileSize     ->SetText(L"");
		m_textDetailsModifiedDate ->SetText(L"");
		m_textDetailsSyncPlatforms->SetText(L"");
		return;
	}
	
	m_buttonSaveToDisk->SetDisabled(false);
	m_buttonDeleteFile->SetDisabled(false);
	
	ISteamRemoteStorage* cloud = SteamRemoteStorage();
	const std::string    filename(row->GetName());
	
	// Filename:
	m_textDetailsFilename->SetText(filename);
	
	// File size:
	const int32 fileSizeInBytes = cloud->GetFileSize(filename.c_str());
	m_textDetailsFileSize->SetText(Gwen::Utility::Format(
			L"%d bytes (%.2f KB)", fileSizeInBytes, fileSizeInBytes / 1024.0f));
	
	// Modified date:
	const int64  modifiedUnixTime = cloud->GetFileTimestamp(filename.c_str());
	const time_t timestamp        = static_cast<time_t>(modifiedUnixTime);
	std::string timeUTC  (std::asctime(std::gmtime   (&timestamp)));
	std::string timeLocal(std::asctime(std::localtime(&timestamp)));
	tt::str::replace(timeUTC,   "\n", "");
	tt::str::replace(timeLocal, "\n", "");
	m_textDetailsModifiedDate->SetText(timeUTC + " UTC (" + timeLocal + " local)");
	
	// Sync platforms:
	const ERemoteStoragePlatform syncPlatforms = cloud->GetSyncPlatforms(filename.c_str());
	if (syncPlatforms == k_ERemoteStoragePlatformNone)
	{
		m_textDetailsSyncPlatforms->SetText(L"None");
	}
	else if (syncPlatforms == k_ERemoteStoragePlatformAll)
	{
		m_textDetailsSyncPlatforms->SetText(L"All");
	}
	else
	{
		tt::str::Strings platforms;
		if ((syncPlatforms & k_ERemoteStoragePlatformWindows) != 0) platforms.push_back("Windows");
		if ((syncPlatforms & k_ERemoteStoragePlatformOSX)     != 0) platforms.push_back("Mac OS X");
		if ((syncPlatforms & k_ERemoteStoragePlatformPS3)     != 0) platforms.push_back("PS3");
		m_textDetailsSyncPlatforms->SetText(tt::str::implode(platforms, ", "));
	}
}


void DebugSteamCloudFileManager::onSaveFileToDisk()
{
	const std::string filename(m_listFiles->GetSelectedRowName());
	if (filename.empty())
	{
		return;
	}
	
	m_filenameToOperateOn = filename;
	
	// This will only work in our internal directory setup: compose a path to the root of the game's working copy
	std::string initialFilename(tt::fs::utils::compactPath(
			tt::app::getApplication()->getAssetRootDir() + "../../../",
			"\\/"));
	initialFilename += filename;
	
	// Compose some filters for the file extension
	const std::string fileExt(tt::fs::utils::getExtension(filename));
	std::string extensionFilters("Original Extension (*." + fileExt + ")|*." + fileExt);
	extensionFilters += "|Any Extension (*.*)|*.*";
	
	// Allow the user to pick a filename to save to
	Gwen::Dialogs::FileSave(true, "Save Steam Cloud file to disk", initialFilename, extensionFilters,
	                        this, &DebugSteamCloudFileManager::onSaveFilenamePicked);
}


void DebugSteamCloudFileManager::onDeleteFile()
{
	const std::string filename(m_listFiles->GetSelectedRowName());
	if (filename.empty())
	{
		return;
	}
	
	GenericDialogBox* dlg = m_editor->showGenericDialog(
			L"Delete File?",
			L"Are you sure you want to delete file '" + tt::str::widen(filename) +
			L"' from Steam Cloud?\nThis action cannot be undone.",
			DialogButtons_YesNo);
	if (dlg != 0)
	{
		dlg->SetSize(450, 150);
		m_editor->centerInScreen(dlg);
		dlg->UserData.Set("filename", filename);
		dlg->onWindowClosed.Add(this, &DebugSteamCloudFileManager::onConfirmDeleteClosed);
	}
}


void DebugSteamCloudFileManager::onUploadFile()
{
	/* TODO:
	- Show file picker
	- If file selected, write file to Cloud
	- Force file share
	*/
}


void DebugSteamCloudFileManager::onConfirmDeleteClosed(Gwen::Controls::Base* p_sender)
{
	DialogBoxBase* dialog = gwen_cast<DialogBoxBase>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0 || dialog->getResult() != DialogBoxBase::Result_Yes)
	{
		return;
	}
	
	// User confirmed deleting this file
	const std::string filename(dialog->UserData.Get<std::string>("filename"));
	if (filename.empty() == false &&
	    SteamRemoteStorage()->FileDelete(filename.c_str()))
	{
		// File should have been deleted: select another level and remove this file from the list
		using namespace Gwen::Controls;
		
		Layout::TableRow* fileItem = gwen_cast<Layout::TableRow>(m_listFiles->FindChildByName(filename, true));
		TT_NULL_ASSERT(fileItem);
		if (fileItem == 0)
		{
			// File does not appear to be in the list: odd. Just refresh the list as a fail-safe
			refreshFileList();
			return;
		}
		
		Layout::TableRow* selectedItem = m_listFiles->GetSelectedRow();
		if (selectedItem == fileItem)
		{
			// Try to select the next item
			Gwen::Controls::Base* listAsBase = m_listFiles;
			listAsBase->OnKeyDown(true);
			if (m_listFiles->GetSelectedRow() == selectedItem)
			{
				// Apparently there is no next item: try the previous item
				listAsBase->OnKeyUp(true);
			}
		}
		
		m_listFiles->RemoveItem(fileItem);
		if (m_listFiles->GetSelectedRow() == 0)
		{
			onFileSelected();
		}
	}
	else
	{
		m_editor->showGenericDialog(
				L"Delete Failed",
				L"Could not delete file '" + tt::str::widen(filename) + L"' from Steam Cloud.");
	}
}


void DebugSteamCloudFileManager::onSaveFilenamePicked(Gwen::Event::Info p_info)
{
	const std::string targetFilename(p_info.String.Get());
	TT_ASSERT(targetFilename.empty() == false);
	TT_ASSERT(m_filenameToOperateOn.empty() == false);
	if (targetFilename.empty() || m_filenameToOperateOn.empty())
	{
		return;
	}
	
	ISteamRemoteStorage* cloud = SteamRemoteStorage();
	
	if (cloud->FileExists(m_filenameToOperateOn.c_str()) == false)
	{
		m_editor->showGenericDialog(
				L"Steam Cloud File Does Not Exist",
				L"Steam Cloud file '" + tt::str::widen(m_filenameToOperateOn) +
				L"' does not exist any more. Cannot save it to disk.");
		return;
	}
	
	// Read the Cloud file into memory
	const int32 sourceSize  = cloud->GetFileSize(m_filenameToOperateOn.c_str());
	tt::code::BufferPtrForCreator sourceContents(new tt::code::Buffer(sourceSize));
	
	const int32 bytesRead = cloud->FileRead(m_filenameToOperateOn.c_str(), sourceContents->getData(), sourceSize);
	if (bytesRead != sourceSize)
	{
		m_editor->showGenericDialog(
				L"Reading Steam Cloud File Failed",
				L"Could not load the contents of Steam Cloud file '" + tt::str::widen(m_filenameToOperateOn) +
				L"'.");
		return;
	}
	
	// Save the file to disk
	tt::fs::FilePtr targetFile(tt::fs::open(targetFilename, tt::fs::OpenMode_Write));
	if (targetFile == 0)
	{
		m_editor->showGenericDialog(
				L"Saving Steam Cloud File To Disk Failed",
				L"Could not open file '" + tt::str::widen(targetFilename) + L"' for writing.");
		return;
	}
	
	if (targetFile->write(sourceContents->getData(), sourceContents->getSize()) != sourceContents->getSize())
	{
		m_editor->showGenericDialog(
				L"Saving Steam Cloud File To Disk Failed",
				L"Could not write all Steam Cloud file '" + tt::str::widen(m_filenameToOperateOn) +
				L"' contents to file '" + tt::str::widen(targetFilename) + L"'.");
		return;
	}
	
	m_filenameToOperateOn.clear();
}


#if 0

void DebugSteamCloudFileManager::onResultFileShare(RemoteStorageFileShareResult_t* p_result, bool p_ioFailure)
{
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		m_labelPublishStatus->SetText("Could not upload level to Steam Cloud.");
		m_textPublishTitle->SetDisabled(false);
		m_textPublishDesc ->SetDisabled(false);
		m_buttonPublish   ->SetDisabled(false);
		m_uploadQueue.clear();
		return;
	}
	
	TT_ASSERT(m_uploadQueue.empty() == false);
	m_uploadQueue.erase(m_uploadQueue.begin());
	
	if (m_uploadQueue.empty())
	{
		// All file have been uploaded to Steam Cloud: now we can publish it on Steam Workshop
		m_labelPublishStatus->SetText("Level uploaded to Steam Cloud; publishing to Steam Workshop...");
		
		SteamParamStringArray_t* tags = 0;  // optional: the tags to associate with the item
		SteamAPICall_t handle = SteamRemoteStorage()->PublishWorkshopFile(
				m_publishDetails.filename.c_str(),
				m_publishDetails.previewFilename.c_str(),
				m_appID,
				m_publishDetails.title.c_str(),
				m_publishDetails.description.c_str(),
				m_publishDetails.visibility,
				tags,
				k_EWorkshopFileTypeCommunity);
		m_callbackPublishWorkshopFile.Set(handle, this, &DebugSteamCloudFileManager::onResultPublishWorkshopFile);
	}
	else
	{
		// Continue with the next file to upload
		SteamAPICall_t handle = SteamRemoteStorage()->FileShare(m_uploadQueue.front().c_str());
		m_callbackFileShare.Set(handle, this, &DebugSteamCloudFileManager::onResultFileShare);
	}
}

#endif  // 0

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)
