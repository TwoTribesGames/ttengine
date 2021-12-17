#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)

#include <algorithm>

#include <Gwen/Controls/GroupBox.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/TabControl.h>

#include <tt/app/Application.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/helpers.h>
#include <tt/str/common.h>
#include <tt/system/Calendar.h>

#include <toki/game/editor/ui/GenericDialogBox.h>
#include <toki/game/editor/ui/SteamWorkshopTest.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/Game.h>
#include <toki/level/LevelData.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR(SteamWorkshopTest)
,
m_editor                  (0),
m_appID                   ((SteamUtils() != 0) ? SteamUtils()->GetAppID() : 0),
m_labelDetailsTitle       (0),
m_labelDetailsAuthor      (0),
m_labelDetailsFilename    (0),
m_labelDetailsSize        (0),
m_labelDetailsDescription (0),
m_buttonDetailsSubscribe  (0),
m_buttonDetailsDelete     (0),
m_textPublishTitle        (0),
m_textPublishDesc         (0),
m_labelPublishStatus      (0),
m_buttonPublish           (0),
m_publishDetails          (),
m_getPublishedFileDetailsQueue(),
m_publishedFileDetails    (),
m_subscribedFiles         (),
m_selectedFileId          (0),
m_uploadQueue             ()
{
	for (s32 i = 0; i < ListID_Count; ++i)
	{
		m_listFiles    [i] = 0;
		m_buttonRefresh[i] = 0;
	}
	
	SetTitle("Steam Workshop Test");
	SetMinimumSize(Gwen::Point(50, 50));
	SetSize(650, 600);
	SetClosable(true);
	SetDeleteOnClose(true);
}


SteamWorkshopTest::~SteamWorkshopTest()
{
}


SteamWorkshopTest* SteamWorkshopTest::create(Editor*               p_editor,
                                             Gwen::Controls::Base* p_parent)
{
	TT_NULL_ASSERT(p_editor);
	if (p_editor == 0) return 0;
	
	SteamWorkshopTest* dlg = new SteamWorkshopTest(p_parent);
	dlg->m_editor = p_editor;
	dlg->createUi();
	return dlg;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SteamWorkshopTest::createUi()
{
	TT_NULL_ASSERT(m_editor);
	
	using namespace Gwen::Controls;
	
	Base* panelLists = new Base(this);
	panelLists->Dock(Gwen::Pos::Top);  // FIXME: Can I use "fill" more than once?
	panelLists->SetHeight(250);
	
	
	GroupBox* panelTabs = new GroupBox(panelLists);
	panelTabs->Dock(Gwen::Pos::Left);
	panelTabs->SetWidth(200);
	panelTabs->SetText("Workshop Files");
	
	TabControl* tabs = new TabControl(panelTabs);
	tabs->Dock(Gwen::Pos::Fill);
	
	// List of own published files
	{
		Base* panel = new Base(tabs);
		const ListID id = ListID_Published;
		
		m_listFiles[id] = new ListBox(panel);
		m_listFiles[id]->SetMargin(Gwen::Margin(3, 3, 3, 3));
		m_listFiles[id]->Dock(Gwen::Pos::Fill);
		m_listFiles[id]->onRowSelected.Add(this, &SteamWorkshopTest::onSelectFile);
		
		m_buttonRefresh[id] = new Button(panel);
		m_buttonRefresh[id]->SetMargin(Gwen::Margin(3, 3, 3, 3));
		m_buttonRefresh[id]->SetHeight(20);
		m_buttonRefresh[id]->SetText("Refresh");
		m_buttonRefresh[id]->Dock(Gwen::Pos::Bottom);
		m_buttonRefresh[id]->onPress.Add(this, &SteamWorkshopTest::onRefreshPublished);
		
		panel->Dock(Gwen::Pos::Fill);
		TabButton* tab = tabs->AddPage("My Published", panel);
		tab->UserData.Set("listID", id);
		tab->onPress.Add(this, &SteamWorkshopTest::onListTab);
	}
	
	// List of own subscribed files
	{
		Base* panel = new Base(tabs);
		const ListID id = ListID_Subscribed;
		
		m_listFiles[id] = new ListBox(panel);
		m_listFiles[id]->SetMargin(Gwen::Margin(3, 3, 3, 3));
		m_listFiles[id]->Dock(Gwen::Pos::Fill);
		m_listFiles[id]->onRowSelected.Add(this, &SteamWorkshopTest::onSelectFile);
		
		m_buttonRefresh[id] = new Button(panel);
		m_buttonRefresh[id]->SetMargin(Gwen::Margin(3, 3, 3, 3));
		m_buttonRefresh[id]->SetHeight(20);
		m_buttonRefresh[id]->SetText("Refresh");
		m_buttonRefresh[id]->Dock(Gwen::Pos::Bottom);
		m_buttonRefresh[id]->onPress.Add(this, &SteamWorkshopTest::onRefreshSubscribed);
		
		panel->Dock(Gwen::Pos::Fill);
		TabButton* tab = tabs->AddPage("My Subscribed", panel);
		tab->UserData.Set("listID", id);
		tab->onPress.Add(this, &SteamWorkshopTest::onListTab);
	}
	
	// List of public workshop files
	{
		Base* panel = new Base(tabs);
		const ListID id = ListID_AllWorkshop;
		
		m_listFiles[id] = new ListBox(panel);
		m_listFiles[id]->SetMargin(Gwen::Margin(3, 3, 3, 3));
		m_listFiles[id]->Dock(Gwen::Pos::Fill);
		m_listFiles[id]->onRowSelected.Add(this, &SteamWorkshopTest::onSelectFile);
		
		m_buttonRefresh[id] = new Button(panel);
		m_buttonRefresh[id]->SetMargin(Gwen::Margin(3, 3, 3, 3));
		m_buttonRefresh[id]->SetHeight(20);
		m_buttonRefresh[id]->SetText("Refresh");
		m_buttonRefresh[id]->Dock(Gwen::Pos::Bottom);
		m_buttonRefresh[id]->onPress.Add(this, &SteamWorkshopTest::onRefreshAllWorkshop);
		
		panel->Dock(Gwen::Pos::Fill);
		TabButton* tab = tabs->AddPage("All", panel);
		tab->UserData.Set("listID", id);
		tab->onPress.Add(this, &SteamWorkshopTest::onListTab);
	}
	
	// Details of the selected item
	{
		GroupBox* panel = new GroupBox(panelLists);
		panel->SetText("Selected File Details");
		panel->SetMargin(Gwen::Margin(5, 0, 0, 0));
		panel->Dock(Gwen::Pos::Fill);
		
		static const int labelWidth = 100;
		
		{
			Base* row = new Base(panel);
			
			Label* label = new Label(row);
			label->SetText("Title:");
			label->SizeToContents();
			label->SetWidth(labelWidth);
			label->Dock(Gwen::Pos::Left);
			
			m_labelDetailsTitle = new Label(row);
			m_labelDetailsTitle->Dock(Gwen::Pos::Fill);
			
			row->Dock(Gwen::Pos::Top);
			row->SetHeight(label->Height());
		}
		
		{
			Base* row = new Base(panel);
			
			Label* label = new Label(row);
			label->SetText("Author:");
			label->SizeToContents();
			label->SetWidth(labelWidth);
			label->Dock(Gwen::Pos::Left);
			
			m_labelDetailsAuthor = new Label(row);
			m_labelDetailsAuthor->Dock(Gwen::Pos::Fill);
			
			row->Dock(Gwen::Pos::Top);
			row->SetHeight(label->Height());
		}
		
		{
			Base* row = new Base(panel);
			
			Label* label = new Label(row);
			label->SetText("Filename:");
			label->SizeToContents();
			label->SetWidth(labelWidth);
			label->Dock(Gwen::Pos::Left);
			
			m_labelDetailsFilename = new Label(row);
			m_labelDetailsFilename->Dock(Gwen::Pos::Fill);
			
			row->Dock(Gwen::Pos::Top);
			row->SetHeight(label->Height());
		}
		
		{
			Base* row = new Base(panel);
			
			Label* label = new Label(row);
			label->SetText("Size:");
			label->SizeToContents();
			label->SetWidth(labelWidth);
			label->Dock(Gwen::Pos::Left);
			
			m_labelDetailsSize = new Label(row);
			m_labelDetailsSize->Dock(Gwen::Pos::Fill);
			
			row->Dock(Gwen::Pos::Top);
			row->SetHeight(label->Height());
		}
		
		Label* label = new Label(panel);
		label->SetText("Description:");
		label->SizeToContents();
		label->Dock(Gwen::Pos::Top);
		
		m_labelDetailsDescription = new Label(panel);
		m_labelDetailsDescription->SetAlignment(Gwen::Pos::Top | Gwen::Pos::Left);
		m_labelDetailsDescription->SetWrap(true);
		m_labelDetailsDescription->Dock(Gwen::Pos::Fill);
		
		// Button panel below the details
		Base* panelButtons = new Base(panel);
		panelButtons->SetHeight(20);
		panelButtons->SetMargin(Gwen::Margin(0, 3, 0, 0));
		panelButtons->Dock(Gwen::Pos::Bottom);
		
		Button* buttonBrowseWorkshop = new Button(panelButtons);
		buttonBrowseWorkshop->SetText("Browse Workshop...");
		buttonBrowseWorkshop->SetWidth(110);
		buttonBrowseWorkshop->Dock(Gwen::Pos::Left);
		buttonBrowseWorkshop->onPress.Add(this, &SteamWorkshopTest::onBrowseWorkshop);
		
		m_buttonDetailsDelete = new Button(panelButtons);
		m_buttonDetailsDelete->SetText("Delete");
		m_buttonDetailsDelete->Dock(Gwen::Pos::Right);
		m_buttonDetailsDelete->SetDisabled(true);
		m_buttonDetailsDelete->onPress.Add(this, &SteamWorkshopTest::onDeleteLevel);
		
		m_buttonDetailsSubscribe = new Button(panelButtons);
		m_buttonDetailsSubscribe->SetText("Subscribe");
		m_buttonDetailsSubscribe->SetMargin(Gwen::Margin(0, 0, 5, 0));
		m_buttonDetailsSubscribe->Dock(Gwen::Pos::Right);
		m_buttonDetailsSubscribe->SetDisabled(true);
		m_buttonDetailsSubscribe->onPress.Add(this, &SteamWorkshopTest::onSubscribe);
	}
	
	Base* panelBottom = new Base(this);
	panelBottom->SetMargin(Gwen::Margin(0, 5, 0, 0));
	panelBottom->Dock(Gwen::Pos::Fill);
	
	// UI to publish current level
	{
		GroupBox* panel = new GroupBox(panelBottom);
		panel->SetText("Publish Current Level");
		{
			Gwen::Margin margin(panel->GetMargin());
			margin.top += 5;
			panel->SetMargin(margin);
		}
		panel->Dock(Gwen::Pos::Fill);
		
		Label* titleLabel = new Label(panel);
		titleLabel->SetText("Title:");
		titleLabel->SizeToContents();
		titleLabel->Dock(Gwen::Pos::Top);
		
		m_textPublishTitle = new TextBox(panel);
		m_textPublishTitle->SetHeight(20);
		m_textPublishTitle->SetMargin(Gwen::Margin(0, 0, 0, 5));
		m_textPublishTitle->Dock(Gwen::Pos::Top);
		
		
		Label* descLabel = new Label(panel);
		descLabel->SetText("Description:");
		descLabel->SizeToContents();
		descLabel->Dock(Gwen::Pos::Top);
		
		m_textPublishDesc = new TextBoxMultiline(panel);
		m_textPublishDesc->SetMargin(Gwen::Margin(0, 0, 0, 5));
		m_textPublishDesc->Dock(Gwen::Pos::Fill);
		
		
		Base* panelStatus = new Base(panel);
		
		Label* statusLabel = new Label(panelStatus);
		statusLabel->SetText("Status:");
		statusLabel->SizeToContents();
		statusLabel->SetMargin(Gwen::Margin(0, 0, 5, 0));
		statusLabel->Dock(Gwen::Pos::Left);
		
		m_labelPublishStatus = new Label(panelStatus);
		m_labelPublishStatus->Dock(Gwen::Pos::Fill);
		
		panelStatus->SetHeight(statusLabel->Height());
		panelStatus->Dock(Gwen::Pos::Bottom);
		
		
		m_buttonPublish = new Button(panel);
		m_buttonPublish->SetText("Publish");
		m_buttonPublish->SetHeight(20);
		m_buttonPublish->Dock(Gwen::Pos::Bottom);
		m_buttonPublish->onPress.Add(this, &SteamWorkshopTest::onPublishLevel);
	}
	
	// Download a published level? Maybe? Add to one of the lists?
	{
		//
	}
	
	// Perform an initial refresh of the lists
	refreshAllLists();
}


void SteamWorkshopTest::addFileItem(
		const RemoteStorageGetPublishedFileDetailsResult_t& p_details,
		ListID                                              p_list)
{
	// Add item in format "title (filename)"
	Gwen::Controls::Layout::TableRow* item = m_listFiles[p_list]->AddItem(
			tt::str::utf8ToUtf16(p_details.m_rgchTitle) +
			L" (" + tt::str::utf8ToUtf16(p_details.m_pchFileName) + L")");
	item->UserData.Set("publishedFileId", p_details.m_nPublishedFileId);
}


void SteamWorkshopTest::fillList(ListID             p_list,
                                 PublishedFileId_t* p_fileIDs,
                                 int32              p_count,
                                 bool               p_hasError)
{
	m_listFiles    [p_list]->Clear();
	m_buttonRefresh[p_list]->SetDisabled(false);
	
	if (p_hasError)
	{
		m_listFiles[p_list]->AddItem("--- error getting results ---");
		return;
	}
	
	if (p_count > 0)
	{
		const bool queueWasEmpty = m_getPublishedFileDetailsQueue.empty();
		for (int32 i = 0; i < p_count; ++i)
		{
			const PublishedFileId_t id = p_fileIDs[i];
			PublishedFileDetails::iterator detailsIt = m_publishedFileDetails.find(id);
			if (detailsIt == m_publishedFileDetails.end())
			{
				// No details yet: request them from Steam
				m_getPublishedFileDetailsQueue.push_back(std::make_pair(id, p_list));
			}
			else
			{
				// We have a cached result: use that
				// NOTE: This does mean that results can be added to the list out of order
				//       (those already cached mixed with those newly requested)
				addFileItem((*detailsIt).second, p_list);
			}
		}
		
		if (queueWasEmpty && m_getPublishedFileDetailsQueue.empty() == false)
		{
			// Trigger request of details
			getPublishedFileDetails(m_getPublishedFileDetailsQueue.front().first);
		}
	}
	else
	{
		m_listFiles[p_list]->AddItem("--- no results ---");
	}
}


void SteamWorkshopTest::refreshAllLists()
{
	onRefreshPublished();
	onRefreshSubscribed();
	onRefreshAllWorkshop();
}


void SteamWorkshopTest::clearLevelDetails()
{
	m_labelDetailsTitle      ->SetText("");
	m_labelDetailsAuthor     ->SetText("");
	m_labelDetailsFilename   ->SetText("");
	m_labelDetailsSize       ->SetText("");
	m_labelDetailsDescription->SetText("");
	m_buttonDetailsSubscribe ->SetText("Subscribe");
	m_buttonDetailsSubscribe ->SetDisabled(true);
	m_buttonDetailsDelete    ->SetDisabled(true);
}


void SteamWorkshopTest::onRefreshPublished()
{
	m_listFiles    [ListID_Published]->Clear();
	m_listFiles    [ListID_Published]->AddItem("... refreshing ...");
	m_buttonRefresh[ListID_Published]->SetDisabled(true);
	
	SteamAPICall_t handle = SteamRemoteStorage()->EnumerateUserPublishedFiles(0);
	m_callbackUserPublishedFiles.Set(handle, this, &SteamWorkshopTest::onResultPublishedFiles);
}


void SteamWorkshopTest::onRefreshSubscribed()
{
	m_listFiles    [ListID_Subscribed]->Clear();
	m_listFiles    [ListID_Subscribed]->AddItem("... refreshing ...");
	m_buttonRefresh[ListID_Subscribed]->SetDisabled(true);
	
	SteamAPICall_t handle = SteamRemoteStorage()->EnumerateUserSubscribedFiles(0);
	m_callbackUserSubscribedFiles.Set(handle, this, &SteamWorkshopTest::onResultSubscribedFiles);
}


void SteamWorkshopTest::onRefreshAllWorkshop()
{
	m_listFiles    [ListID_AllWorkshop]->Clear();
	m_listFiles    [ListID_AllWorkshop]->AddItem("... refreshing ...");
	m_buttonRefresh[ListID_AllWorkshop]->SetDisabled(true);
	
	SteamAPICall_t handle = SteamRemoteStorage()->EnumeratePublishedWorkshopFiles(
			k_EWorkshopEnumerationTypeRecent, 0, 50, 0, 0, 0);
	m_callbackAllWorkshopFiles.Set(handle, this, &SteamWorkshopTest::onResultAllWorkshopFiles);
}


void SteamWorkshopTest::onPublishLevel()
{
	const std::string& title(m_textPublishTitle->GetText().Get());
	const std::string& desc (m_textPublishDesc ->GetText().Get());
	if (title.empty() || desc.empty())
	{
		TT_PANIC("Fill in a title and a description.");
		return;
	}
	
	// DEBUG: For now, generate a unique filename each time we publish, so that we always publish new entries
	std::string filename;
	std::string previewFilename;
	{
		const std::string&         levelName(m_editor->getCurrentLevelName());
		const tt::system::Calendar now(tt::system::Calendar::getCurrentDate());
		
		std::ostringstream oss;
		oss << "workshop_level_"
		    << levelName
		    << "_"
		    << std::setw(4) << std::setfill('0') << now.getYear()   << "-"
		    << std::setw(2) << std::setfill('0') << now.getMonth()  << "-"
		    << std::setw(2) << std::setfill('0') << now.getDay()    << "_"
		    << std::setw(2) << std::setfill('0') << now.getHour()   << "-"
		    << std::setw(2) << std::setfill('0') << now.getMinute() << "-"
		    << std::setw(2) << std::setfill('0') << now.getSecond();
		
		filename        = oss.str() + ".ttlvl";
		previewFilename = oss.str() + ".png";
	}
	
	// Save the level to Steam Cloud
	level::LevelDataPtr levelData(m_editor->getLevelData());
	const tt::fs::identifier steamCloudFS = tt::app::getApplication()->getSaveFsID();
	AppGlobal::getGame()->getPathMgr().saveTileCachesToLevelData(levelData);
	if (levelData->save(filename, steamCloudFS) == false)
	{
		m_labelPublishStatus->SetText("Could not save level to Steam Cloud.");
		return;
	}
	
	// Also save a preview image to Steam Cloud
	{
		tt::code::BufferPtr sourceContents = tt::fs::getFileContent("workshop_temp_preview_image.png");
		if (sourceContents == 0)
		{
			m_labelPublishStatus->SetText("Could not load source preview image.");
			return;
		}
		
		tt::fs::FilePtr targetFile(tt::fs::open(previewFilename, tt::fs::OpenMode_Write, steamCloudFS));
		if (targetFile == 0)
		{
			m_labelPublishStatus->SetText("Could not create preview image for level.");
			return;
		}
		
		targetFile->write(sourceContents->getData(), sourceContents->getSize());
	}
	
	m_textPublishTitle->SetDisabled(true);
	m_textPublishDesc ->SetDisabled(true);
	m_buttonPublish   ->SetDisabled(true);
	
	// Store the information for the current item to publish
	m_publishDetails.filename        = filename;
	m_publishDetails.previewFilename = previewFilename;
	m_publishDetails.title           = title;
	m_publishDetails.description     = desc;
	// FIXME: Allow user to select visibility? Probably best...
	m_publishDetails.visibility =
			k_ERemoteStoragePublishedFileVisibilityPublic;
	//		k_ERemoteStoragePublishedFileVisibilityFriendsOnly;
	//		k_ERemoteStoragePublishedFileVisibilityPrivate;
	
	
	// Force an upload of the files to Steam Cloud
	TT_ASSERT(m_uploadQueue.empty());
	m_labelPublishStatus->SetText("Uploading level to Steam Cloud...");
	m_uploadQueue.push_back(filename);
	m_uploadQueue.push_back(previewFilename);
	
	SteamAPICall_t handle = SteamRemoteStorage()->FileShare(m_uploadQueue.front().c_str());
	m_callbackFileShare.Set(handle, this, &SteamWorkshopTest::onResultFileShare);
}


void SteamWorkshopTest::onBrowseWorkshop()
{
	tt::steam::openURL("http://steamcommunity.com/workshop/browse/?appid=" + tt::str::toStr(m_appID));
}


void SteamWorkshopTest::onSubscribe()
{
	if (m_selectedFileId == 0)
	{
		// No file selected
		return;
	}
	
	m_buttonDetailsSubscribe->SetDisabled(true);
	
	if (m_subscribedFiles.find(m_selectedFileId) == m_subscribedFiles.end())
	{
		// Not subscribed yet: subscribe now
		m_buttonDetailsSubscribe->SetText("Subscribing...");
		
		SteamAPICall_t handle = SteamRemoteStorage()->SubscribePublishedFile(m_selectedFileId);
		m_callbackSubscribe.Set(handle, this, &SteamWorkshopTest::onResultSubscribe);
	}
	else
	{
		// Already subscribed: unsubscribe
		m_buttonDetailsSubscribe->SetText("Unsubscribing...");
		
		SteamAPICall_t handle = SteamRemoteStorage()->UnsubscribePublishedFile(m_selectedFileId);
		m_callbackUnsubscribe.Set(handle, this, &SteamWorkshopTest::onResultUnsubscribe);
	}
}


void SteamWorkshopTest::onDeleteLevel()
{
	if (m_selectedFileId != 0)
	{
		GenericDialogBox* dlg = m_editor->showGenericDialog(
				L"Delete Level?",
				L"Are you sure you want to delete level '" + m_labelDetailsTitle->GetText().GetUnicode() +
				L"'?\nThis action cannot be undone.",
				DialogButtons_YesNo, true);
		if (dlg != 0)
		{
			dlg->UserData.Set("fileId", m_selectedFileId);
			dlg->onWindowClosed.Add(this, &SteamWorkshopTest::onConfirmDeleteClosed);
		}
	}
}


void SteamWorkshopTest::onSelectFile(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::ListBox* list = gwen_cast<Gwen::Controls::ListBox>(p_sender);
	if (list == 0) return;
	
	Gwen::Controls::Layout::TableRow* row = list->GetSelectedRow();
	if (row == 0 || row->UserData.Exists("publishedFileId") == false)
	{
		// No row selected, or don't have ID to use for getting details
		m_selectedFileId = 0;
		clearLevelDetails();
		return;
	}
	
	const PublishedFileId_t id = row->UserData.Get<PublishedFileId_t>("publishedFileId");
	m_selectedFileId = id;
	PublishedFileDetails::iterator it = m_publishedFileDetails.find(id);
	if (it == m_publishedFileDetails.end())
	{
		TT_Printf("SteamWorkshopTest::onSelectFile: No cached details available for selected file; odd.\n");
		clearLevelDetails();
		return;
	}
	
	const RemoteStorageGetPublishedFileDetailsResult_t& details((*it).second);
	const bool     isSubscribed = (m_subscribedFiles.find(id) != m_subscribedFiles.end());
	const CSteamID authorID(details.m_ulSteamIDOwner);
	using tt::str::utf8ToUtf16;
	
	m_labelDetailsTitle->SetText(utf8ToUtf16(details.m_rgchTitle));
	if (authorID == SteamUser()->GetSteamID())
	{
		// This user is the author
		m_labelDetailsAuthor->SetText(utf8ToUtf16(SteamFriends()->GetPersonaName()));
		m_buttonDetailsDelete->SetDisabled(false);
	}
	else
	{
		// Someone else is the author: get their display name
		const char* authorName = SteamFriends()->GetFriendPersonaName(authorID);
		if (authorName != 0)
		{
			m_labelDetailsAuthor->SetText(utf8ToUtf16(authorName));
		}
		else
		{
			m_labelDetailsAuthor->SetText(
					Gwen::Utility::Format(L"ID %u", static_cast<uint32>(details.m_ulSteamIDOwner)));
		}
		m_buttonDetailsDelete->SetDisabled(true);
	}
	m_labelDetailsFilename   ->SetText(utf8ToUtf16(details.m_pchFileName));
	m_labelDetailsSize       ->SetText(Gwen::Utility::Format(L"%.2f KB", details.m_nFileSize / 1024.0f));
	m_labelDetailsDescription->SetText(utf8ToUtf16(details.m_rgchDescription));
	m_buttonDetailsSubscribe ->SetText(isSubscribed ? "Unsubscribe" : "Subscribe");
	m_buttonDetailsSubscribe ->SetDisabled(false);
}


void SteamWorkshopTest::onListTab(Gwen::Controls::Base* p_sender)
{
	if (p_sender != 0 && p_sender->UserData.Exists("listID"))
	{
		onSelectFile(m_listFiles[p_sender->UserData.Get<ListID>("listID")]);
	}
}


void SteamWorkshopTest::onConfirmDeleteClosed(Gwen::Controls::Base* p_sender)
{
	DialogBoxBase* dialog = gwen_cast<DialogBoxBase>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0) return;
	
	if (dialog->getResult() == DialogBoxBase::Result_Yes)
	{
		// User confirmed deleting this level
		const PublishedFileId_t id = dialog->UserData.Get<PublishedFileId_t>("fileId");
		
		m_buttonDetailsDelete->SetDisabled(true);
		m_buttonDetailsDelete->SetText("Deleting...");
		SteamAPICall_t handle = SteamRemoteStorage()->DeletePublishedFile(id);
		m_callbackDeletePublishedFile.Set(handle, this, &SteamWorkshopTest::onResultDeletePublishedFile);
	}
}


void SteamWorkshopTest::getPublishedFileDetails(PublishedFileId_t p_publishedFileId)
{
	SteamAPICall_t handle = SteamRemoteStorage()->GetPublishedFileDetails(p_publishedFileId, 0);
	m_callbackGetPublishedFileDetails.Set(handle, this, &SteamWorkshopTest::onGetPublishedFileDetails);
}


// Steam callbacks

// Called when SteamRemoteStorage()->EnumerateUserPublishedFiles() returns asynchronously
void SteamWorkshopTest::onResultPublishedFiles(
		RemoteStorageEnumerateUserPublishedFilesResult_t* p_result,
		bool                                              p_ioFailure)
{
	fillList(ListID_Published,
	         p_result->m_rgPublishedFileId,
	         p_result->m_nResultsReturned,
	         p_ioFailure || p_result->m_eResult != k_EResultOK);
}


// Called when SteamRemoteStorage()->EnumerateUserSubscribedFiles() returns asynchronously
void SteamWorkshopTest::onResultSubscribedFiles(
		RemoteStorageEnumerateUserSubscribedFilesResult_t* p_result,
		bool                                               p_ioFailure)
{
	m_subscribedFiles.clear();
	for (int32 i = 0; i < p_result->m_nResultsReturned; ++i)
	{
		m_subscribedFiles.insert(p_result->m_rgPublishedFileId[i]);
	}
	
	fillList(ListID_Subscribed,
	         p_result->m_rgPublishedFileId,
	         p_result->m_nResultsReturned,
	         p_ioFailure || p_result->m_eResult != k_EResultOK);
}


// Called when SteamRemoteStorage()->EnumeratePublishedWorkshopFiles() returns asynchronously
void SteamWorkshopTest::onResultAllWorkshopFiles(
		RemoteStorageEnumerateWorkshopFilesResult_t* p_result,
		bool                                         p_ioFailure)
{
	fillList(ListID_AllWorkshop,
	         p_result->m_rgPublishedFileId,
	         p_result->m_nResultsReturned,
	         p_ioFailure || p_result->m_eResult != k_EResultOK);
}


// Called when SteamRemoteStorage()->PublishWorkshopFile() returns asynchronously
void SteamWorkshopTest::onResultPublishWorkshopFile(
		RemoteStoragePublishFileResult_t* p_result,
		bool                              p_ioFailure)
{
	m_textPublishTitle->SetDisabled(false);
	m_textPublishDesc ->SetDisabled(false);
	m_buttonPublish   ->SetDisabled(false);
	
	m_textPublishTitle->SetText("");
	m_textPublishDesc ->SetText("");
	
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		m_labelPublishStatus->SetText(
				Gwen::Utility::Format(L"Could not publish file to Steam Workshop. Result %d.",
				                      p_result->m_eResult));
	}
	else
	{
		m_labelPublishStatus->SetText(
				Gwen::Utility::Format(L"Successfully published file to Steam Workshop! File ID %llu.",
				                      p_result->m_nPublishedFileId));
		
		onRefreshPublished();
		onRefreshAllWorkshop();
	}
}


void SteamWorkshopTest::onGetPublishedFileDetails(
		RemoteStorageGetPublishedFileDetailsResult_t* p_result,
		bool                                          p_ioFailure)
{
	const PublishedFileId_t id = p_result->m_nPublishedFileId;
	
	// If we got a valid result, store the details
	if (p_ioFailure == false && p_result->m_eResult == k_EResultOK)
	{
		m_publishedFileDetails[id] = *p_result;
		
		// Add an item to all lists that wanted these details
		for (DetailsRequests::iterator it = m_getPublishedFileDetailsQueue.begin();
		     it != m_getPublishedFileDetailsQueue.end(); ++it)
		{
			if ((*it).first == id)
			{
				addFileItem(*p_result, (*it).second);
			}
		}
	}
	
	// Remove all requests for this file ID from the queue
	for (DetailsRequests::iterator it = m_getPublishedFileDetailsQueue.begin();
	     it != m_getPublishedFileDetailsQueue.end(); )
	{
		if ((*it).first == id)
		{
			it = m_getPublishedFileDetailsQueue.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	// Continue with the next item in the queue
	if (m_getPublishedFileDetailsQueue.empty() == false)
	{
		getPublishedFileDetails(m_getPublishedFileDetailsQueue.front().first);
	}
}


void SteamWorkshopTest::onResultFileShare(RemoteStorageFileShareResult_t* p_result, bool p_ioFailure)
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
		m_callbackPublishWorkshopFile.Set(handle, this, &SteamWorkshopTest::onResultPublishWorkshopFile);
	}
	else
	{
		// Continue with the next file to upload
		SteamAPICall_t handle = SteamRemoteStorage()->FileShare(m_uploadQueue.front().c_str());
		m_callbackFileShare.Set(handle, this, &SteamWorkshopTest::onResultFileShare);
	}
}


void SteamWorkshopTest::onResultSubscribe(
		RemoteStorageSubscribePublishedFileResult_t* p_result,
		bool                                         p_ioFailure)
{
	if (p_result->m_nPublishedFileId == m_selectedFileId)
	{
		m_buttonDetailsSubscribe->SetDisabled(false);
	}
	
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		TT_PANIC("Could not subscribe to file.");
		return;
	}
	
	if (p_result->m_nPublishedFileId == m_selectedFileId)
	{
		m_buttonDetailsSubscribe->SetText("Unsubscribe");
	}
	m_subscribedFiles.insert(p_result->m_nPublishedFileId);
	onRefreshSubscribed();
}


void SteamWorkshopTest::onResultUnsubscribe(
		RemoteStorageUnsubscribePublishedFileResult_t* p_result,
		bool                                           p_ioFailure)
{
	if (p_result->m_nPublishedFileId == m_selectedFileId)
	{
		m_buttonDetailsSubscribe->SetDisabled(false);
	}
	
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		TT_PANIC("Could not unsubscribe from file.");
		return;
	}
	
	if (p_result->m_nPublishedFileId == m_selectedFileId)
	{
		m_buttonDetailsSubscribe->SetText("Subscribe");
	}
	m_subscribedFiles.erase(p_result->m_nPublishedFileId);
	onRefreshSubscribed();
}


void SteamWorkshopTest::onResultDeletePublishedFile(
		RemoteStorageDeletePublishedFileResult_t* p_result,
		bool                                      p_ioFailure)
{
	m_buttonDetailsDelete->SetText("Delete");
	
	if (p_ioFailure || p_result->m_eResult != k_EResultOK)
	{
		TT_PANIC("Could not delete published file.");
		return;
	}
	
	// Remove the deleted file from all local caches
	for (DetailsRequests::iterator it = m_getPublishedFileDetailsQueue.begin();
	     it != m_getPublishedFileDetailsQueue.end(); )
	{
		if ((*it).first == p_result->m_nPublishedFileId)
		{
			it = m_getPublishedFileDetailsQueue.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	PublishedFileDetails::iterator detailsIt = m_publishedFileDetails.find(p_result->m_nPublishedFileId);
	if (detailsIt != m_publishedFileDetails.end())
	{
		m_publishedFileDetails.erase(detailsIt);
	}
	
	m_subscribedFiles.erase(p_result->m_nPublishedFileId);
	
	if (m_selectedFileId == p_result->m_nPublishedFileId)
	{
		m_selectedFileId = 0;
		clearLevelDetails();
	}
	
	// Refresh the file lists
	refreshAllLists();
}

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)
