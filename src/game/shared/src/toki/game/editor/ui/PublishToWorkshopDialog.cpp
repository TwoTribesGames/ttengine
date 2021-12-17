#if defined(TT_STEAM_BUILD)

#include <algorithm>

#include <Gwen/Controls/Layout/Position.h>
#include <Gwen/Controls/GroupBox.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/TabControl.h>

#include <tt/app/Application.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/helpers.h>
#include <tt/str/common.h>
#include <tt/system/Calendar.h>

#include <toki/game/editor/ui/GenericDialogBox.h>
#include <toki/game/editor/ui/PublishToWorkshopDialog.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/game/Game.h>
#include <toki/level/LevelData.h>
#include <toki/steam/Workshop.h>
#include <toki/steam/WorkshopObserver.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

// To notify the dialog when the requested file details are available
class DetailsObserver : public steam::WorkshopObserver
{
public:
	static steam::WorkshopObserverPtr create(PublishToWorkshopDialog* p_targetDialog,
	                                         PublishedFileId_t        p_interestedId)
	{
		TT_NULL_ASSERT(p_targetDialog);
		TT_ASSERT(p_interestedId != 0);
		return steam::WorkshopObserverPtr(new DetailsObserver(p_targetDialog, p_interestedId));
	}
	
	virtual void onWorkshopFileChange(PublishedFileId_t p_id, steam::WorkshopObserver::FileAction p_action)
	{
		if (p_id == m_interestedId && p_action == steam::WorkshopObserver::FileAction_DetailsAvailable)
		{
			m_targetDialog->handleFileDetailsAvailable();
		}
	}
	
private:
	inline DetailsObserver(PublishToWorkshopDialog* p_targetDialog,
	                       PublishedFileId_t        p_interestedId)
	:
	steam::WorkshopObserver(),
	m_targetDialog(p_targetDialog),
	m_interestedId(p_interestedId)
	{ }
	
	// No copying
	DetailsObserver(const DetailsObserver&);
	const DetailsObserver& operator=(const DetailsObserver&);
	
	
	PublishToWorkshopDialog* m_targetDialog;
	const PublishedFileId_t  m_interestedId;
};


class PublishTitleTextBox : public Gwen::Controls::TextBox
{
public:
	GWEN_CONTROL_INLINE(PublishTitleTextBox, TextBox)
	{ }
	virtual ~PublishTitleTextBox() { }
	
protected:
	virtual bool IsTextAllowed(const Gwen::UnicodeString& p_typedText, int p_insertionPos)
	{
		if (p_typedText.empty())
		{
			return true;
		}
		
		// Limit the title to the number of UTF-8 bytes(!) that can be stored
		Gwen::UnicodeString newFullText(GetText().GetUnicode());
		newFullText.insert(p_insertionPos, p_typedText);
		const std::string asUtf8(tt::str::utf16ToUtf8(newFullText));
		if (asUtf8.length() > k_cchPublishedDocumentTitleMax - 1)  // leave room for null terminator
		{
			return false;
		}
		
		return true;
	}
};


class PublishDescriptionTextBox : public Gwen::Controls::TextBoxMultiline
{
public:
	GWEN_CONTROL_INLINE(PublishDescriptionTextBox, TextBoxMultiline)
	{ }
	virtual ~PublishDescriptionTextBox() { }
	
protected:
	virtual bool IsTextAllowed(const Gwen::UnicodeString& p_typedText, int p_insertionPos)
	{
		if (p_typedText.empty())
		{
			return true;
		}
		
		// Limit the description to the number of UTF-8 bytes(!) that can be stored
		Gwen::UnicodeString newFullText(GetText().GetUnicode());
		newFullText.insert(p_insertionPos, p_typedText);
		const std::string asUtf8(tt::str::utf16ToUtf8(newFullText));
		if (asUtf8.length() > k_cchPublishedDocumentDescriptionMax - 1)  // leave room for null terminator
		{
			return false;
		}
		
		return true;
	}
};


//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR(PublishToWorkshopDialog)
,
m_callResultPublishWorkshopFile(),
m_callResultUpdatePublishedFile(),
m_callResultFileShare          (),
m_editor             (0),
m_consumerAppID      (201420),  // FIXME: Get this from somewhere? Can't use current app ID
m_textPublishTitle   (0),
m_textPublishDesc    (0),
m_comboVisibility    (0),
m_labelAgreeToLicense(0),
m_labelStatus        (0),
m_busyBar            (0),
m_buttonPublish      (0),
m_previewImagePanel  (0),
m_previewImageControl(0),
m_previewImage       (tt::engine::renderer::QuadSprite::createQuad(
		tt::engine::renderer::TexturePtr(), tt::engine::renderer::ColorRGB::white)),
m_publishDetails     (),
m_uploadQueue        (),
m_observer           ()
{
	destroyAutoCreatedUi();
	
	SetMinimumSize(Gwen::Point(100, 100));
	SetSize(300, 400);
	SetClosable(true);
	SetDeleteOnClose(false);
	SetHidden(true);
}


PublishToWorkshopDialog::~PublishToWorkshopDialog()
{
	if (m_observer != 0 && steam::Workshop::hasInstance())
	{
		steam::Workshop::getInstance()->unregisterObserver(m_observer);
	}
}


PublishToWorkshopDialog* PublishToWorkshopDialog::create(Editor*               p_editor,
                                                         Gwen::Controls::Base* p_parent)
{
	TT_NULL_ASSERT(p_editor);
	if (p_editor == 0) return 0;
	
	PublishToWorkshopDialog* dlg = new PublishToWorkshopDialog(p_parent);
	dlg->m_editor = p_editor;
	dlg->createUi();
	return dlg;
}


void PublishToWorkshopDialog::SetHidden(bool p_hidden)
{
	BaseClass::SetHidden(p_hidden);
	
	if (p_hidden == false && m_editor != 0)
	{
		const u64 publishedId = m_editor->getLevelData()->getPublishedFileId();
		if (publishedId != 0 && steam::Workshop::getInstance()->isPublishedFile(publishedId))
		{
			// Updating already published item
			SetTitle(translateString("WINDOW_PUBLISH_UPDATE_TITLE"));
		}
		else
		{
			// Creating new item
			SetTitle(translateString("WINDOW_PUBLISH_TITLE"));
		}
	}
}


void PublishToWorkshopDialog::levelPlayTestSucceeded()
{
	// Perform the actual publishing now
	TT_ASSERTMSG(AppGlobal::getGame()->hasScheduledScreenshot() == false,
	             "Level was completed before a screenshot could be taken.");
	doPublishLevel();
}


void PublishToWorkshopDialog::levelPlayTestFailed()
{
	// Hide this dialog: user needs to fix playtest errors first
	CloseButtonPressed();
	errorPopup("WINDOW_PUBLISH_LEVEL_ERRORS_TITLE", "WINDOW_PUBLISH_LEVEL_ERRORS_PROMPT");
}


void PublishToWorkshopDialog::handleFileDetailsAvailable()
{
	steam::Workshop* workshop = steam::Workshop::getInstance();
	
	const u64 publishedId = m_editor->getLevelData()->getPublishedFileId();
	TT_ASSERT(publishedId != 0 && workshop->isPublishedFile(publishedId));
	
	const steam::Workshop::FileDetails* fileDetails(workshop->getCachedFileDetails(publishedId));
	TT_NULL_ASSERT(fileDetails);
	if (fileDetails != 0)
	{
		const RemoteStorageGetPublishedFileDetailsResult_t& details(fileDetails->details);
		m_textPublishTitle->SetText(tt::str::utf8ToUtf16(details.m_rgchTitle));
		m_textPublishDesc ->SetText(tt::str::utf8ToUtf16(details.m_rgchDescription));
		m_comboVisibility->SelectItemByName("vis_" + tt::str::toStr(details.m_eVisibility));
	}
	
	setUiEnabled(true);
	
	// No longer need to observe events for this file
	TT_NULL_ASSERT(m_observer);
	//workshop->unregisterObserver(m_observer);  // FIXME: Modifying observers while iterating observer container causes errors
	m_observer.reset();
}


void PublishToWorkshopDialog::renderPostGwen()
{
	if (m_previewImageControl->Visible() == false)
	{
		return;
	}
	
	const Gwen::Rect& bounds(m_previewImageControl->GetRenderBounds());
	const Gwen::Point pos   (m_previewImageControl->LocalPosToCanvas());
	
	real centerX = static_cast<real>(pos.x + (bounds.w / 2));
	real centerY = static_cast<real>(pos.y + (bounds.h / 2));
#if defined(TT_PLATFORM_WIN)
	centerX -= 0.5f;
	centerY -= 0.5f;
#endif
	m_previewImage->setPosition(centerX, centerY, 0.0f);
	m_previewImage->setWidth   (static_cast<real>(bounds.w));
	m_previewImage->setHeight  (static_cast<real>(bounds.h));
	m_previewImage->update();
	m_previewImage->render();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void PublishToWorkshopDialog::createUi()
{
	TT_NULL_ASSERT(m_editor);
	
	using namespace Gwen::Controls;
	
	Base* panel = new Base(this);
	panel->Dock(Gwen::Pos::Fill);
	
	// Title:
	Label* titleLabel = new Label(panel);
	titleLabel->SetText(translateString("WINDOW_PUBLISH_LABEL_TITLE"));
	titleLabel->SizeToContents();
	titleLabel->Dock(Gwen::Pos::Top);
	
	m_textPublishTitle = new PublishTitleTextBox(panel);
	m_textPublishTitle->SetHeight(20);
	m_textPublishTitle->SetMargin(Gwen::Margin(0, 0, 0, 5));
	m_textPublishTitle->Dock(Gwen::Pos::Top);
	
	// Description:
	Label* descLabel = new Label(panel);
	descLabel->SetText(translateString("WINDOW_PUBLISH_LABEL_DESCRIPTION"));
	descLabel->SizeToContents();
	descLabel->Dock(Gwen::Pos::Top);
	
	m_textPublishDesc = new PublishDescriptionTextBox(panel);
	m_textPublishDesc->SetMargin(Gwen::Margin(0, 0, 0, 5));
	m_textPublishDesc->Dock(Gwen::Pos::Fill);
	
	// Status and busy bar (shown during publishing)
	m_busyBar = new tt::gwen::BusyBar(panel);
	m_busyBar->SetHeight(20);
	m_busyBar->SetHidden(true);
	m_busyBar->Dock(Gwen::Pos::Bottom);
	
	m_labelStatus = new Label(panel);
	m_labelStatus->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	m_labelStatus->SetWrap(true);
	m_labelStatus->SetHeight(50);
	m_labelStatus->SetHidden(true);
	m_labelStatus->Dock(Gwen::Pos::Bottom);
	
	// Publish button (hidden during publishing)
	m_buttonPublish = new Button(panel);
	m_buttonPublish->SetText(translateString("WINDOW_PUBLISH_BUTTON_PUBLISH"));
	m_buttonPublish->SetHeight(20);
	m_buttonPublish->Dock(Gwen::Pos::Bottom);
	m_buttonPublish->onPress.Add(this, &PublishToWorkshopDialog::onPublishLevel);
	
	// Agree To License label
	m_labelAgreeToLicense = new LabelClickable(panel);
	m_labelAgreeToLicense->SetWrap(true);
	m_labelAgreeToLicense->SetText(translateString("WINDOW_PUBLISH_LABEL_AGREE_TO_LICENSE"));
	m_labelAgreeToLicense->SetWidth(Width());
	m_labelAgreeToLicense->SizeToContents();
	m_labelAgreeToLicense->SetMargin(Gwen::Margin(0, 0, 0, 10));
	m_labelAgreeToLicense->Dock(Gwen::Pos::Bottom);
	m_labelAgreeToLicense->onPress.Add(this, &PublishToWorkshopDialog::onViewAgreement);
	
	// Visibility:
	{
		m_comboVisibility = new ComboBox(panel);
		m_comboVisibility->SetMargin(Gwen::Margin(0, 0, 0, 10));
		m_comboVisibility->SetHeight(20);
		m_comboVisibility->Dock(Gwen::Pos::Bottom);
		
		Label* label = new Label(panel);
		label->SetText(translateString("WINDOW_PUBLISH_LABEL_VISIBILITY"));
		label->SizeToContents();
		label->Dock(Gwen::Pos::Bottom);
		
		struct Item
		{
			ERemoteStoragePublishedFileVisibility visibility;
			const char*                           displayName;
		};
		static const Item items[] =
		{
			{ k_ERemoteStoragePublishedFileVisibilityPublic,      "WINDOW_PUBLISH_VISIBILITY_PUBLIC"      },
			{ k_ERemoteStoragePublishedFileVisibilityFriendsOnly, "WINDOW_PUBLISH_VISIBILITY_FRIENDSONLY" },
			{ k_ERemoteStoragePublishedFileVisibilityPrivate,     "WINDOW_PUBLISH_VISIBILITY_PRIVATE"     }
		};
		static const size_t itemCount = sizeof(items) / sizeof(items[0]);
		
		static const ERemoteStoragePublishedFileVisibility defaultVisibility =
				k_ERemoteStoragePublishedFileVisibilityPublic;
		
		for (size_t i = 0; i < itemCount; ++i)
		{
			MenuItem* item = m_comboVisibility->AddItem(
					translateString(items[i].displayName),
					"vis_" + tt::str::toStr(items[i].visibility));
			item->UserData.Set("visibility", items[i].visibility);
			if (items[i].visibility == defaultVisibility)
			{
				m_comboVisibility->SelectItem(item);  // default to Public
			}
		}
	}
	
	// Level preview image
	{
		m_previewImagePanel = new Layout::Center(this);
		m_previewImagePanel->Dock(Gwen::Pos::Right);
		
		m_previewImageControl = new Base(m_previewImagePanel);
		m_previewImageControl->SetSize(637, 358);
		
		m_previewImagePanel->SetMargin(Gwen::Margin(10, 0, 0, 0));
		m_previewImagePanel->SetWidth(m_previewImageControl->Width());
		m_previewImagePanel->SetHidden(true);
	}
	
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
	else
	{
		// Use the level name as default title
		m_textPublishTitle->SetText(m_editor->getCurrentLevelName());
	}
	
	m_textPublishTitle->MoveCaretToEnd();
	m_textPublishTitle->Focus();
}


void PublishToWorkshopDialog::setUiEnabled(bool p_enabled)
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


void PublishToWorkshopDialog::setLevelPreviewVisible(bool p_visible)
{
	const bool wasVisible = (m_previewImagePanel->Hidden() == false);
	if (p_visible == wasVisible)
	{
		// No change
		return;
	}
	
	if (p_visible)
	{
		// Set the quad's current texture to null so that the preview image is removed from the cache
		// (because we want to reload it in case it changed)
		m_previewImage->setTexture(tt::engine::renderer::TexturePtr());
		
		// Load the preview image
		TT_ASSERT(m_publishDetails.previewFilename.empty() == false);
		
		const std::string&               filename(m_publishDetails.previewFilename);
		const tt::fs::identifier         steamCloudFS = tt::app::getApplication()->getSaveFsID();
		tt::engine::renderer::TexturePtr previewTex;
		
		if (tt::fs::fileExists(filename, steamCloudFS))
		{
			tt::engine::renderer::TexturePtr tex(tt::engine::cache::FileTextureCache::get(
					filename, steamCloudFS));
			if (tex != 0)
			{
				previewTex = tex;
			}
			else
			{
				TT_WARN("Could not load level screenshot '%s' as a texture.", filename.c_str());
			}
		}
		else
		{
			TT_WARN("No level screenshot available during publishing! Expected file '%s'.",
			        filename.c_str());
		}
		
		if (previewTex == 0)
		{
			// Have no image to display: do not become visible
			// FIXME: Instead, perhaps load a fallback image?
			return;
		}
		
		m_previewImage->setTexture(previewTex);
	}
	
	// Resize the window to accomodate the preview panel
	const Gwen::Margin& margin(m_previewImagePanel->GetMargin());
	const int panelWidth = m_previewImagePanel->Width() + margin.left + margin.right;
	const int oldWindowWidth = Width();
	int newWindowWidth = oldWindowWidth;
	if (p_visible)
	{
		newWindowWidth += panelWidth;
	}
	else
	{
		newWindowWidth -= panelWidth;
	}
	
	SetBounds(X() + ((oldWindowWidth - newWindowWidth) / 2), Y(), newWindowWidth, Height());
	
	// Now toggle the panel
	m_previewImagePanel->SetHidden(p_visible == false);
}


void PublishToWorkshopDialog::doPublishLevel()
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


void PublishToWorkshopDialog::doSubmitToWorkshop()
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
					&PublishToWorkshopDialog::onResultUpdatePublishedFile);
		}
		else
		{
			setUiEnabled(true);
			setLevelPreviewVisible(false);
			
			ui::GenericDialogBox* dlg = m_editor->showGenericDialog(
					translateString("WINDOW_PUBLISH_PUBLISH_FAILED_TITLE"),
					translateString("WINDOW_PUBLISH_PUBLISH_FAILED_UPDATE_PROMPT"),
					DialogButtons_RetryCancel);
			dlg->onWindowClosed.Add(this, &PublishToWorkshopDialog::onPublishErrorDialogClosed);
		}
	}
	else
	{
		// This is the first time this level is being published
		publishNewItem();
	}
}


void PublishToWorkshopDialog::publishNewItem()
{
	m_labelStatus->SetText(translateString("WINDOW_PUBLISH_STATUS_PUBLISH_NEW"));
	
	SteamAPICall_t handle = SteamRemoteStorage()->PublishWorkshopFile(
			m_publishDetails.filename.c_str(),
			m_publishDetails.previewFilename.c_str(),
			m_consumerAppID,
			m_publishDetails.title.c_str(),
			m_publishDetails.description.c_str(),
			m_publishDetails.visibility,
			&m_publishDetails.tagsAsArray,
			k_EWorkshopFileTypeCommunity);
	m_callResultPublishWorkshopFile.Set(handle, this, &PublishToWorkshopDialog::onResultPublishWorkshopFile);
}


void PublishToWorkshopDialog::showPublishError(EResult p_resultCode, bool p_ioFailure)
{
	std::wstring errorCode;
	
	if (p_ioFailure)
	{
		errorCode = translateString("WINDOW_PUBLISH_PUBLISH_FAILED_IO_FAILURE");
	}
	else
	{
		errorCode = tt::str::widen(tt::str::toStr(p_resultCode));
		
		// See if we can provide more information than just the error code
		std::wstring reason;
		switch (p_resultCode)
		{
		case k_EResultFail:                                     reason = L"generic failure"; break;
		case k_EResultNoConnection:                             reason = L"no/failed network connection"; break;
		case k_EResultInvalidPassword:                          reason = L"password/ticket is invalid"; break;
		case k_EResultLoggedInElsewhere:                        reason = L"same user logged in elsewhere"; break;
		case k_EResultInvalidProtocolVer:                       reason = L"protocol version is incorrect"; break;
		case k_EResultInvalidParam:                             reason = L"a parameter is incorrect"; break;
		case k_EResultFileNotFound:                             reason = L"file was not found"; break;
		case k_EResultBusy:                                     reason = L"called method busy - action not taken"; break;
		case k_EResultInvalidState:                             reason = L"called object was in an invalid state"; break;
		case k_EResultInvalidName:                              reason = L"name is invalid"; break;
		case k_EResultInvalidEmail:                             reason = L"email is invalid"; break;
		case k_EResultDuplicateName:                            reason = L"name is not unique"; break;
		case k_EResultAccessDenied:                             reason = L"access is denied"; break;
		case k_EResultTimeout:                                  reason = L"operation timed out"; break;
		case k_EResultBanned:                                   reason = L"VAC2 banned"; break;
		case k_EResultAccountNotFound:                          reason = L"account not found"; break;
		case k_EResultInvalidSteamID:                           reason = L"Steam ID is invalid"; break;
		case k_EResultServiceUnavailable:                       reason = L"the requested service is currently unavailable"; break;
		case k_EResultNotLoggedOn:                              reason = L"the user is not logged on"; break;
		case k_EResultPending:                                  reason = L"request is pending (may be in process, or waiting on third party)"; break;
		case k_EResultEncryptionFailure:                        reason = L"encryption or decryption failed"; break;
		case k_EResultInsufficientPrivilege:                    reason = L"insufficient privilege"; break;
		case k_EResultLimitExceeded:                            reason = L"limit exceeded"; break;
		case k_EResultRevoked:                                  reason = L"access has been revoked (used for revoked guest passes)"; break;
		case k_EResultExpired:                                  reason = L"license/Guest pass the user is trying to access is expired"; break;
		case k_EResultAlreadyRedeemed:                          reason = L"guest pass has already been redeemed by account, cannot be acked again"; break;
		case k_EResultDuplicateRequest:                         reason = L"the request is a duplicate and the action has already occurred in the past, ignored this time"; break;
		case k_EResultAlreadyOwned:                             reason = L"all the games in this guest pass redemption request are already owned by the user"; break;
		case k_EResultIPNotFound:                               reason = L"IP address not found"; break;
		case k_EResultPersistFailed:                            reason = L"failed to write change to the data store"; break;
		case k_EResultLockingFailed:                            reason = L"failed to acquire access lock for this operation"; break;
		case k_EResultLogonSessionReplaced:                     reason = L"logon session replaced"; break;
		case k_EResultConnectFailed:                            reason = L"connect failed"; break;
		case k_EResultHandshakeFailed:                          reason = L"handshake failed"; break;
		case k_EResultIOFailure:                                reason = L"I/O failure"; break;
		case k_EResultRemoteDisconnect:                         reason = L"remote disconnect"; break;
		case k_EResultShoppingCartNotFound:                     reason = L"failed to find the shopping cart requested"; break;
		case k_EResultBlocked:                                  reason = L"a user didn't allow it"; break;
		case k_EResultIgnored:                                  reason = L"target is ignoring sender"; break;
		case k_EResultNoMatch:                                  reason = L"nothing matching the request found"; break;
		case k_EResultAccountDisabled:                          reason = L"account disabled"; break;
		case k_EResultServiceReadOnly:                          reason = L"this service is not accepting content changes right now"; break;
		case k_EResultAccountNotFeatured:                       reason = L"account doesn't have value, so this feature isn't available"; break;
		case k_EResultAdministratorOK:                          reason = L"allowed to take this action, but only because requester is admin"; break;
		case k_EResultContentVersion:                           reason = L"a version mismatch in content transmitted within the Steam protocol"; break;
		case k_EResultTryAnotherCM:                             reason = L"the current CM can't service the user making a request, user should try another"; break;
		case k_EResultPasswordRequiredToKickSession:            reason = L"you are already logged in elsewhere, this cached credential login has failed"; break;
		case k_EResultAlreadyLoggedInElsewhere:                 reason = L"you are already logged in elsewhere, you must wait"; break;
		case k_EResultSuspended:                                reason = L"long running operation (content download) suspended/paused"; break;
		case k_EResultCancelled:                                reason = L"operation canceled (typically by user: content download)"; break;
		case k_EResultDataCorruption:                           reason = L"operation canceled because data is ill formed or unrecoverable"; break;
		case k_EResultDiskFull:                                 reason = L"operation canceled - not enough disk space"; break;
		case k_EResultRemoteCallFailed:                         reason = L"a remote call or IPC call failed"; break;
		case k_EResultPasswordUnset:                            reason = L"password could not be verified as it's unset server side"; break;
		case k_EResultExternalAccountUnlinked:                  reason = L"external account (PSN, Facebook...) is not linked to a Steam account"; break;
		case k_EResultExternalAccountAlreadyLinked:             reason = L"external account (PSN, Facebook...) is already linked to some other account, must explicitly request to replace/delete the link first"; break;
		case k_EResultRemoteFileConflict:                       reason = L"the sync cannot resume due to a conflict between the local and remote files"; break;
		case k_EResultIllegalPassword:                          reason = L"the requested new password is not legal"; break;
		case k_EResultSameAsPreviousValue:                      reason = L"new value is the same as the old one ( secret question and answer )"; break;
		case k_EResultAccountLogonDenied:                       reason = L"account login denied due to 2nd factor authentication failure"; break;
		case k_EResultCannotUseOldPassword:                     reason = L"the requested new password is not legal"; break;
		case k_EResultInvalidLoginAuthCode:                     reason = L"account login denied due to auth code invalid"; break;
		case k_EResultAccountLogonDeniedNoMail:                 reason = L"account login denied due to 2nd factor auth failure - and no mail has been sent"; break;
		case k_EResultHardwareNotCapableOfIPT:                  reason = L"hardware not capable of IPT"; break;
		case k_EResultIPTInitError:                             reason = L"IPT init error"; break;
		case k_EResultParentalControlRestricted:                reason = L"operation failed due to parental control restrictions for current user"; break;
		case k_EResultFacebookQueryError:                       reason = L"Facebook query returned an error"; break;
		case k_EResultExpiredLoginAuthCode:                     reason = L"account login denied due to auth code expired"; break;
		case k_EResultIPLoginRestrictionFailed:                 reason = L"IP login restriction failed"; break;
		case k_EResultAccountLockedDown:                        reason = L"account locked down"; break;
		case k_EResultAccountLogonDeniedVerifiedEmailRequired:  reason = L"account logon denied: verified email required"; break;
		case k_EResultNoMatchingURL:                            reason = L"no matching URL"; break;
		case k_EResultBadResponse:                              reason = L"parse failure, missing field, etc."; break;
		case k_EResultRequirePasswordReEntry:                   reason = L"the user cannot complete the action until they re-enter their password"; break;
		case k_EResultValueOutOfRange:                          reason = L"the value entered is outside the acceptable range"; break;
		default:                                                break;  // have nothing to add to the error code
		}
		
		if (reason.empty() == false)
		{
			errorCode += L" (" + reason + L")";
		}
	}
	
	ui::GenericDialogBox* dlg = m_editor->showGenericDialog(
			translateString("WINDOW_PUBLISH_PUBLISH_FAILED_TITLE"),
			translateString("WINDOW_PUBLISH_PUBLISH_FAILED_PROMPT", errorCode),
			DialogButtons_RetryCancel);
	dlg->onWindowClosed.Add(this, &PublishToWorkshopDialog::onPublishErrorDialogClosed);
}


void PublishToWorkshopDialog::handlePublishResult(
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


void PublishToWorkshopDialog::inputErrorPopup(
		Gwen::Controls::Base* p_inputControl,
		const std::string&    p_titleID,
		const std::string&    p_textID)
{
	GenericDialogBox* dlg = m_editor->showGenericDialog(translateString(p_titleID),
	                                                    translateString(p_textID));
	dlg->UserData.Set("focusControl", p_inputControl);
	dlg->onWindowClosed.Add(this, &PublishToWorkshopDialog::onInputErrorDialogClosed);
}


void PublishToWorkshopDialog::errorPopup(const std::string& p_titleID, const std::string& p_textID)
{
	m_editor->showGenericDialog(translateString(p_titleID), translateString(p_textID));
}


void PublishToWorkshopDialog::triggerFileUpload(const std::string& p_filename)
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
	m_callResultFileShare.Set(handle, this, &PublishToWorkshopDialog::onResultFileShare);
}


void PublishToWorkshopDialog::onViewAgreement()
{
	tt::steam::openURL("http://steamcommunity.com/sharedfiles/workshoplegalagreement");
}


void PublishToWorkshopDialog::onPublishLevel()
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
	dlg->onWindowClosed.Add(this, &PublishToWorkshopDialog::onConfirmPlaytestDialogClosed);
}


void PublishToWorkshopDialog::onInputErrorDialogClosed(Gwen::Controls::Base* p_sender)
{
	TT_NULL_ASSERT(p_sender);
	Gwen::Controls::Base* focusControl = p_sender->UserData.Get<Gwen::Controls::Base*>("focusControl");
	if (focusControl != 0)
	{
		focusControl->Focus();
	}
}


void PublishToWorkshopDialog::onPublishErrorDialogClosed(Gwen::Controls::Base* p_sender)
{
	DialogBoxBase* dialog = gwen_cast<DialogBoxBase>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0 || dialog->getResult() != Result_Retry)
	{
		return;
	}
	
	doPublishLevel();
}


void PublishToWorkshopDialog::onConfirmPlaytestDialogClosed(Gwen::Controls::Base* p_sender)
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


// Called when SteamRemoteStorage()->PublishWorkshopFile() returns asynchronously
void PublishToWorkshopDialog::onResultPublishWorkshopFile(
		RemoteStoragePublishFileResult_t* p_result,
		bool                              p_ioFailure)
{
	handlePublishResult(p_result->m_eResult, p_ioFailure, p_result->m_nPublishedFileId);
}


void PublishToWorkshopDialog::onResultUpdatePublishedFile(
		RemoteStorageUpdatePublishedFileResult_t* p_result,
		bool                                      p_ioFailure)
{
	handlePublishResult(p_result->m_eResult, p_ioFailure, p_result->m_nPublishedFileId);
}


void PublishToWorkshopDialog::onResultFileShare(RemoteStorageFileShareResult_t* p_result, bool p_ioFailure)
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

// Namespace end
}
}
}
}


#endif  // defined(TT_STEAM_BUILD)
