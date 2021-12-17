#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/TabControl.h>

#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/utils/utils.h>
#include <tt/str/str.h>
#include <tt/system/utils.h>

#include <toki/AppGlobal.h>
#include <toki/game/script/Registry.h>  // NOTE: Community only: needed for progress reset
#include <toki/game/Game.h>             // NOTE: Community only: needed for progress reset
#include <toki/input/Controller.h>
#include <toki/input/Recorder.h>
#include <toki/input/RecorderGui.h>


namespace toki {
namespace input {


RecorderGuiPtr RecorderGui::create(Recorder* p_recorder)
{
	return RecorderGuiPtr(new RecorderGui(p_recorder));
}


RecorderGui::~RecorderGui()
{
}


bool RecorderGui::update(real /*p_deltaTime*/)
{
	// Only perform GUI work when the UI is actually visible
	if (m_visible == false)
	{
		return false;
	}
	
	if (m_stopRecordingThisFrame)
	{
		m_stopRecordingThisFrame = false;
		m_recorder->stopRecording();
		updateControls();
		m_saveIndicator->fadeOut(1.0f);
	}
	
	
	const input::Controller& input(AppGlobal::getController(tt::input::ControllerIndex_One));
	
	const bool handledInput = m_gwenRoot.handleInput(
		input.cur.editor.pointer, input.prev.editor.pointer,
		input.cur.editor.pointerLeft, input.cur.editor.pointerRight,
			input.cur.wheelNotches * 120, m_rootDock);
	
	const bool modifierDown = input.cur.editor.keys[tt::input::Key_Control].down;
	
	// Check for early out
	if (modifierDown == false ||
	    m_resetProgressConfirmationVisible)  // NOTE: Community only: do not handle hotkeys when resetting progress
	{
		updateSaveIndicator();
		return handledInput;
	}
	
	if(m_recordButton->IsDisabled() == false && input.cur.editor.keys[tt::input::Key_R].pressed)
	{
		m_recordButton->Toggle();
		onRecordPressed(0);
	}
	else if(m_stopButton->IsDisabled() == false && input.cur.editor.keys[tt::input::Key_S].pressed)
	{
		onStopPressed(0);
	}
	else if(m_playButton->IsDisabled() == false && input.cur.editor.keys[tt::input::Key_P].pressed)
	{
		m_playButton->Toggle();
		onPlayPressed(0);
	}
	else if(m_ffButton->IsDisabled() == false && input.cur.editor.keys[tt::input::Key_F].pressed)
	{
		m_ffButton->Toggle();
		onFastForwardPressed(0);
		
		AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor.keys[tt::input::Key_F].reset();
	}
	
	updateSaveIndicator();
	
	return handledInput;
}


void RecorderGui::render() const
{
	if (m_visible)
	{
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		renderer->beginHud();
		m_saveIndicator->render();
		renderer->endHud();
		
		m_gwenRoot.render();
	}
}


void RecorderGui::handlePlayPressed()
{
	onPlayPressed(0);
}


void RecorderGui::updateControls()
{
	TT_NULL_ASSERT(m_recorder);
	
	bool hasRecording = m_recorder->hasRecording();
	bool isRecording  = m_recorder->getState() == Recorder::State_Record;
	bool isPlaying    = m_recorder->getState() == Recorder::State_Play;
	
	m_recordButton->SetToggleState(isRecording);
	m_recordButton->SetDisabled(isPlaying);
	
	m_stopButton->SetDisabled((isPlaying || isRecording) == false);
	
	m_playButton->SetToggleState(isPlaying);
	m_playButton->SetDisabled(isRecording || (hasRecording == false));
	
	//m_ffButton->SetToggleState(false);
	m_ffButton->SetDisabled(isRecording || (hasRecording == false));
	
	m_skipFwdButton->SetDisabled((isPlaying && m_recorder->canSkipForward()) == false);
	
	m_skipBackButton->SetDisabled((isPlaying && m_recorder->canSkipBackward()) == false);
	
	m_showFileButton->SetDisabled((isPlaying || isRecording));
	
	m_openFileButton->SetDisabled(isRecording || isPlaying);
	
	// NOTE: Community only! Only enable the "reset progress" button when not recording or playing back
	m_resetProgressButton->SetDisabled(isRecording || isPlaying);
	
	// Update state label
	if (isPlaying)
	{
		if (m_ffButton->GetToggleState())
		{
			m_stateLabel->SetText("Fast Forward >>");
		}
		else
		{
			m_stateLabel->SetText("Playing");
		}
	}
	else if (isRecording)
	{
		m_stateLabel->SetText("Recording");
	}
	else
	{
		m_stateLabel->SetText("Idle");
	}
	
	// Update section label
	if (isRecording || isPlaying)
	{
		m_sectionLabel->SetText("Section: " +
			tt::str::toStr(m_recorder->getCurrentSectionIndex() + 1) + "/" + 
			tt::str::toStr(m_recorder->getTotalSections()));
	}
	else
	{
		m_sectionLabel->SetText("");
	}
}


void RecorderGui::setInfoText(const std::string& p_text)
{
	m_infoLabel->SetText(p_text);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

RecorderGui::RecorderGui(Recorder* p_recorder)
:
Gwen::Event::Handler(),
m_visible(false),
m_gwenRoot("InputRecorder", "gwen_skin", tt::engine::renderer::ViewPortID_Main),
m_rootDock(0),
m_recordButton(0),
m_playButton(0),
m_stopButton(0),
m_ffButton(0),
m_skipFwdButton(0),
m_skipBackButton(0),
m_showFileButton(0),
m_openFileButton(0),
m_stateLabel(0),
m_infoLabel(0),
m_loadWindow(0),
m_fileList(0),
m_submitLabel1(0),
m_submitLabel2(0),
m_resetProgressConfirmationVisible(false),  // NOTE: Community only!
m_resetProgressButton(0),                   // NOTE: Community only!
m_confirmResetProgressLabel(0),             // NOTE: Community only!
m_confirmResetProgressButton(0),            // NOTE: Community only!
m_cancelResetProgressButton(0),             // NOTE: Community only!
m_saveIndicator(tt::engine::renderer::QuadSprite::createQuad(
		tt::engine::renderer::TextureCache::get("saving", "textures.editor.ui"),
		tt::engine::renderer::ColorRGB::white)),
m_stopRecordingThisFrame(false),
m_recorder(p_recorder)
{
	m_saveIndicator->resetFlag(tt::engine::renderer::QuadSprite::Flag_Visible);
	
	TT_NULL_ASSERT(m_recorder);
	setupGui();
}


void RecorderGui::setupGui()
{
	static const s32 buttonSize = 50;
	
	using namespace Gwen::Controls;
	m_rootDock = new DockBase(m_gwenRoot.getCanvas());
	m_rootDock->Dock(Gwen::Pos::Fill);
	m_rootDock->GetBottom()->SetHeight(buttonSize * 2);
	
	Base* base = new Base(m_rootDock->GetBottom());
	base->Dock(Gwen::Pos::Fill);
	
	m_rootDock->GetBottom()->GetTabControl()->AddPage("Input Recording", base);
	base->SetPadding(Gwen::Padding(5,5,5,5));
	
	// Record Button
	m_recordButton = new Button(base);
	m_recordButton->SetSize(buttonSize, buttonSize);
	m_recordButton->SetText("Record");
	m_recordButton->SetPos(0, 0);
	m_recordButton->SetIsToggle(true);
	m_recordButton->onPress.Add(this, &RecorderGui::onRecordPressed);
	m_recordButton->SetToolTip("Start/Stop Recording (Ctrl+R)");
	
	// Stop Button
	m_stopButton = new Button(base);
	m_stopButton->SetSize(buttonSize, buttonSize);
	m_stopButton->SetText("Stop");
	m_stopButton->SetPos(m_recordButton->Right() + 10, 0);
	m_stopButton->onPress.Add(this, &RecorderGui::onStopPressed);
	m_stopButton->SetToolTip("Stop (Ctrl+S)");
	
	// Play Button
	m_playButton = new Button(base);
	m_playButton->SetSize(buttonSize, buttonSize);
	m_playButton->SetText("Play");
	m_playButton->SetPos(m_stopButton->Right() + 10, 0);
	m_playButton->SetIsToggle(true);
	m_playButton->onPress.Add(this, &RecorderGui::onPlayPressed);
	m_playButton->SetToolTip("Play recorded input (Ctrl+P)");
	
	// Fast Forward Button
	m_ffButton = new Button(base);
	m_ffButton->SetSize(buttonSize, buttonSize);
	m_ffButton->SetText("FF");
	m_ffButton->SetPos(m_playButton->Right() + 10, 0);
	m_ffButton->SetIsToggle(true);
	m_ffButton->onPress.Add(this, &RecorderGui::onFastForwardPressed);
	m_ffButton->SetToolTip("Increase playback speed (Ctrl+F)");
	
	// Skip CP Buttons
	m_skipBackButton = new Button(base);
	m_skipBackButton->SetSize(buttonSize/2, buttonSize);
	m_skipBackButton->SetText("<");
	m_skipBackButton->SetPos(m_ffButton->Right() + 10, 0);
	m_skipBackButton->onPress.Add(this, &RecorderGui::onSkipBackPressed);
	m_skipBackButton->SetToolTip("Skip to previous section");
	
	m_skipFwdButton = new Button(base);
	m_skipFwdButton->SetSize(buttonSize/2, buttonSize);
	m_skipFwdButton->SetText(">");
	m_skipFwdButton->SetPos(m_skipBackButton->Right() + 5, 0);
	m_skipFwdButton->onPress.Add(this, &RecorderGui::onSkipForwardPressed);
	m_skipFwdButton->SetToolTip("Skip to next section");
	
	// Show file in navigator Button
	m_showFileButton = new Button(base);
	m_showFileButton->SetSize(buttonSize, buttonSize);
	m_showFileButton->SetText("Show File");
	m_showFileButton->SetPos(m_skipFwdButton->Right() + 10, 0);
	m_showFileButton->onPress.Add(this, &RecorderGui::onShowFilePressed);
	m_showFileButton->SetDisabled(true);
	m_showFileButton->SetToolTip("Show recorded file");
	
	// Open existing recording
	m_openFileButton = new Button(base);
	m_openFileButton->SetSize(buttonSize, buttonSize);
	m_openFileButton->SetText("Open File");
	m_openFileButton->SetPos(m_showFileButton->Right() + 10, 0);
	m_openFileButton->onPress.Add(this, &RecorderGui::onOpenFilePressed);
	m_openFileButton->SetToolTip("Open recording");
	
	// State Label
	m_stateLabel = new Label(base);
	m_stateLabel->SetSize(150, 20);
	m_stateLabel->SetPos(m_openFileButton->Right() + 10, 0);
	
#if defined(TT_STEAM_BUILD)
	{
		m_submitLabel1 = new Label(base);
		m_submitLabel1->SetSize(400, 20);
		m_submitLabel1->SetPos(m_stateLabel->Right() + 10, 10);
		m_submitLabel1->SetText("PLEASE SUBMIT YOUR RECORDINGS TO: tokitori2@twotribes.com\n");
		m_submitLabel1->SetTextColor(Gwen::Colors::Red);
		
		m_submitLabel2 = new Label(base);
		m_submitLabel2->SetSize(400, 20);
		m_submitLabel2->SetPos(m_submitLabel1->X(), m_submitLabel1->Bottom() + 1);
		m_submitLabel2->SetText("FOR LARGE FILES PLEASE USE WETRANSFER.COM\n");
		m_submitLabel2->SetTextColor(Gwen::Colors::Red);
	}
#endif
	
	// Section Label
	m_sectionLabel = new Label(base);
	m_sectionLabel->SetSize(150, 20);
	m_sectionLabel->SetPos(m_stateLabel->X(), m_stateLabel->Bottom() + 1);
	
	// Info Label
	m_infoLabel = new Label(base);
	m_infoLabel->SetSize(150, 20);
	m_infoLabel->SetPos(m_stateLabel->X(), m_sectionLabel->Bottom() + 1);
	
	// Special community-only interface to reset all progress
	{
		Base* resetButtonPanel = new Base(base);
		m_resetProgressButton = new Button(resetButtonPanel);
		m_resetProgressButton->SetSize(100, 20);
		m_resetProgressButton->SetText("Reset All Progress");
		m_resetProgressButton->Dock(Gwen::Pos::Bottom);
		m_resetProgressButton->onPress.Add(this, &RecorderGui::onResetProgressPressed);
		m_resetProgressButton->SetToolTip("Resets all your progress.");
		
		resetButtonPanel->SetWidth(100);
		resetButtonPanel->Dock(Gwen::Pos::Right);
		
		m_confirmResetProgressLabel = new Label(base);
		m_confirmResetProgressLabel->SetText("Are you sure you want to reset all your progress?");
		m_confirmResetProgressLabel->SizeToContents();
		m_confirmResetProgressLabel->SetPos(0, 0);
		m_confirmResetProgressLabel->SetHidden(true);
		
		m_confirmResetProgressButton = new Button(base);
		m_confirmResetProgressButton->SetSize(100, 20);
		m_confirmResetProgressButton->SetPos (0, m_confirmResetProgressLabel->Bottom() + 5);
		m_confirmResetProgressButton->SetText("Reset All Progress");
		m_confirmResetProgressButton->SetTextColor(Gwen::Colors::Red);
		m_confirmResetProgressButton->SetTextColorOverride(Gwen::Colors::Red);
		m_confirmResetProgressButton->onPress.Add(this, &RecorderGui::onConfirmResetProgressPressed);
		m_confirmResetProgressButton->SetToolTip("Confirms that you really wish to reset all your progress.");
		m_confirmResetProgressButton->SetHidden(true);
		
		m_cancelResetProgressButton = new Button(base);
		m_cancelResetProgressButton->SetSize(200, 30);
		m_cancelResetProgressButton->SetPos (m_confirmResetProgressButton->Right() + 50,
		                                     m_confirmResetProgressLabel->Bottom() + 5);
		m_cancelResetProgressButton->SetText("Cancel");
		m_cancelResetProgressButton->onPress.Add(this, &RecorderGui::onCancelResetProgressPressed);
		m_cancelResetProgressButton->SetHidden(true);
	}
	
	// File open window
	createLoadWindow(m_gwenRoot.getCanvas());
	
	// Set buttons to default state
	updateControls();
}


Gwen::Controls::Base* RecorderGui::createLoadWindow(Gwen::Controls::Base* p_parent)
{
	m_loadWindow = new Gwen::Controls::WindowControl(p_parent);
	
	m_loadWindow->SetTitle("Select Recording");
	m_loadWindow->SetMinimumSize(Gwen::Point(50, 50));
	m_loadWindow->SetSize(250, 400);
	m_loadWindow->SetDeleteOnClose(false);
	m_loadWindow->SetClosable(true);
	m_loadWindow->SetHidden(true);
	
	Gwen::Controls::Base* buttonPanel = new Gwen::Controls::Base(m_loadWindow);
	buttonPanel->SetPadding(Gwen::Padding(0, 3, 0, 0));
	
	Gwen::Controls::Button* buttonLoad = new Gwen::Controls::Button(buttonPanel);
	buttonLoad->SetText("Open");
	buttonLoad->SetSize(100, 25);
	buttonLoad->Dock(Gwen::Pos::Right);
	buttonLoad->onPress.Add(this, &RecorderGui::onLoadRecording);
	
	Gwen::Controls::Button* buttonCancel = new Gwen::Controls::Button(buttonPanel);
	buttonCancel->SetText("Cancel");
	buttonCancel->SetSize(100, 25);
	buttonCancel->Dock(Gwen::Pos::Left);
	buttonCancel->onPress.Add(m_loadWindow, &Gwen::Controls::WindowControl::CloseButtonPressed);
	
	buttonPanel->Dock(Gwen::Pos::Bottom);
	buttonPanel->SetHeight(25);
	
	m_fileList= new Gwen::Controls::ListBox(m_loadWindow);
	m_fileList->SetAllowMultiSelect(false);
	m_fileList->Dock(Gwen::Pos::Fill);
	
	return m_loadWindow;
}


void RecorderGui::stopRecordingWithIndicator()
{
	m_stopRecordingThisFrame = true;
	m_saveIndicator->setOpacity(255);
}


void RecorderGui::updateSaveIndicator()
{
	m_saveIndicator->setPosition(tt::engine::renderer::Renderer::getInstance()->getScreenWidth()  * 0.5f,
	                             tt::engine::renderer::Renderer::getInstance()->getScreenHeight() * 0.5f,
	                             0.0f);
	m_saveIndicator->update();
}


// NOTE: Community only! Shows or hides the "reset progress" confirmation UI
void RecorderGui::setResetProgressConfirmationVisible(bool p_visible)
{
	m_resetProgressConfirmationVisible = p_visible;
	
	m_recordButton       ->SetHidden(p_visible);
	m_playButton         ->SetHidden(p_visible);
	m_stopButton         ->SetHidden(p_visible);
	m_ffButton           ->SetHidden(p_visible);
	m_skipFwdButton      ->SetHidden(p_visible);
	m_skipBackButton     ->SetHidden(p_visible);
	m_showFileButton     ->SetHidden(p_visible);
	m_openFileButton     ->SetHidden(p_visible);
	m_stateLabel         ->SetHidden(p_visible);
	m_infoLabel          ->SetHidden(p_visible);
	m_sectionLabel       ->SetHidden(p_visible);
	
	// NOTE: The user-facing "submit your recordings" labels are always kept invisible, since we don't expose this feature anymore
	if (m_submitLabel1 != 0) m_submitLabel1->SetHidden(true /*p_visible*/);
	if (m_submitLabel2 != 0) m_submitLabel2->SetHidden(true /*p_visible*/);
	
	m_resetProgressButton->SetHidden(p_visible);
	
	m_confirmResetProgressLabel ->SetHidden(p_visible == false);
	m_confirmResetProgressButton->SetHidden(p_visible == false);
	m_cancelResetProgressButton ->SetHidden(p_visible == false);
}


void RecorderGui::onRecordPressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	if(m_recordButton->GetToggleState())
	{
		m_recorder->startRecording();
	}
	else
	{
		stopRecordingWithIndicator();
	}
	
	updateControls();
}


void RecorderGui::onStopPressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	if (m_recorder->getState() == Recorder::State_Record)
	{
		stopRecordingWithIndicator();
	}
	else
	{
		m_recorder->stop();
	}
	
	updateControls();
}


void RecorderGui::onPlayPressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	m_recorder->play();
	
	updateControls();
}


void RecorderGui::onFastForwardPressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	if(m_ffButton->GetToggleState())
	{
		// Fast Forward
		m_recorder->fastforwardOn();
	}
	else
	{
		m_recorder->fastforwardOff();
	}
	
	updateControls();
}


void RecorderGui::onSkipForwardPressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	TT_ASSERT(m_recorder->canSkipForward());
	m_recorder->skipForward();
	
	updateControls();
}


void RecorderGui::onSkipBackPressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	TT_ASSERT(m_recorder->canSkipBackward());
	m_recorder->skipBackward();
	
	updateControls();
}


void RecorderGui::onShowFilePressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	
	const std::string filePath = m_recorder->getRecordedFilePath();
	
#if defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX)
	tt::system::openWithDefaultApplication(tt::fs::utils::getDirectory(filePath));
#else
	if (tt::fs::fileExists(filePath))
	{
#if defined(TT_STEAM_BUILD)
		// FIXME: For some reason, showFileInFileNavigator aborts the application in Steam builds,
		//        but not in regular Windows builds. To work around this, do not highlight the file
		//        but simply show the directory instead.
		tt::system::openWithDefaultApplication(tt::fs::utils::getDirectory(filePath));
#else
		tt::system::showFileInFileNavigator(filePath);
#endif
	}
	else if (tt::fs::dirExists(filePath))
	{
		tt::system::openWithDefaultApplication(filePath);
	}
#endif
	
	updateControls();
}


void RecorderGui::onOpenFilePressed(Gwen::Controls::Base* /*p_sender*/)
{
	TT_NULL_ASSERT(m_recorder);
	showFileOpenWindow();
	
	updateControls();
}


void RecorderGui::onLoadRecording(Gwen::Controls::Base* /*p_sender*/)
{
	if (m_fileList == 0)
	{
		return;
	}
	
	const std::string selectedFileName(m_fileList->GetSelectedRowName());
	if (selectedFileName.empty())
	{
		return;
	}
	
	if(m_loadWindow != 0)
	{
		m_loadWindow->CloseButtonPressed();
	}
	
	m_recorder->loadRecordedFile(m_recorder->getRecordingRootPath() + selectedFileName + ".ttrec");
	
	updateControls();
}


// NOTE: Community only! Resets all game progress and loads the default initial level
void RecorderGui::onResetProgressPressed()
{
	setResetProgressConfirmationVisible(true);
}


void RecorderGui::onConfirmResetProgressPressed()
{
	// Clear all progress
	game::script::getRegistry().clear();
	game::script::getRegistry().clearPersistent();
	
	// Load the default level
	game::StartInfo defaultLevel;
	defaultLevel.resetToDefaultLevel();
	AppGlobal::setGameStartInfo(defaultLevel);
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->forceReload();
	}
	
	setResetProgressConfirmationVisible(false);
}


void RecorderGui::onCancelResetProgressPressed()
{
	setResetProgressConfirmationVisible(false);
}


void RecorderGui::showFileOpenWindow()
{
	TT_NULL_ASSERT(m_loadWindow);
	
	if (m_loadWindow == 0)
	{
		return;
	}
	
	if (m_loadWindow->Hidden() == false)
	{
		// Window is already open: don't open it a second time
		return;
	}
	
	refreshFileList();
	
	m_loadWindow->Position(Gwen::Pos::Center);
	m_loadWindow->SetHidden(false);
}


void RecorderGui::refreshFileList()
{
	TT_NULL_ASSERT(m_fileList);
	if (m_fileList == 0) return;
	
	TT_NULL_ASSERT(m_recorder);
	
	// Save currently selected level name, so that the selection can be restored afterwards
	std::string selectedFileName(m_fileList->GetSelectedRowName());
	
	// Repopulate the listbox
	m_fileList->Clear();
	
	if(tt::fs::dirExists(m_recorder->getRecordingRootPath()) == false)
	{
		// No recordings yet
		return;
	}
	
	Gwen::Controls::Layout::TableRow* itemToReselect = 0;
	tt::str::StringSet filenames(tt::fs::utils::getFilesInDir(m_recorder->getRecordingRootPath(), "*.ttrec"));
	for (tt::str::StringSet::reverse_iterator it = filenames.rbegin(); it != filenames.rend(); ++it)
	{
		Gwen::Controls::Layout::TableRow* item = m_fileList->AddItem(*it, *it);
		if (*it == selectedFileName)
		{
			itemToReselect = item;
		}
	}
	
	// And now reselect the previously selected level
	if (itemToReselect != 0)
	{
		m_fileList->SetSelectedRow(itemToReselect);
	}
}


// Namespace end
}
}
