#include <tt/app/Application.h>
#include <tt/math/Random.h>
#include <tt/system/Time.h>
#include <tt/system/Calendar.h>

#include <toki/game/CheckPointMgr.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/input/Recorder.h>
#include <toki/input/RecorderGui.h>
#include <toki/level/LevelData.h>
#include <toki/savedata/utils.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>

#include <toki/game/script/TimerMgr.h>


namespace toki {
namespace input {


//--------------------------------------------------------------------------------------------------
// Helper functions

static void overrideInput(const InputState& p_newState, Controller::State& p_oldState)
{
	tt::input::setCurrentControllerType(p_newState.controllerType);
	p_oldState.pointer              = p_newState.pointer;
	p_oldState.accept               = p_newState.accept;
	p_oldState.cancel               = p_newState.cancel;
	p_oldState.faceUp               = p_newState.faceUp;
	p_oldState.faceLeft             = p_newState.faceLeft;
	p_oldState.direction            = p_newState.direction;
	p_oldState.scroll               = p_newState.scroll;
	p_oldState.left                 = p_newState.left;
	p_oldState.right                = p_newState.right;
	p_oldState.up                   = p_newState.up;
	p_oldState.down                 = p_newState.down;
	p_oldState.virusUpload          = p_newState.virusUpload;
	p_oldState.jump                 = p_newState.jump;
	p_oldState.primaryFire          = p_newState.primaryFire;
	p_oldState.secondaryFire        = p_newState.secondaryFire;
	p_oldState.selectWeapon1        = p_newState.selectWeapon1;
	p_oldState.selectWeapon2        = p_newState.selectWeapon2;
	p_oldState.selectWeapon3        = p_newState.selectWeapon3;
	p_oldState.selectWeapon4        = p_newState.selectWeapon4;
	p_oldState.toggleWeapons        = p_newState.toggleWeapons;
	p_oldState.demoReset            = p_newState.demoReset;
	p_oldState.respawn              = p_newState.respawn;
	p_oldState.menu                 = p_newState.menu;
	p_oldState.screenSwitch         = p_newState.screenSwitch;
	p_oldState.startupFailSafeLevel = p_newState.startupFailSafeLevel;
	
	p_oldState.debugCheat           = p_newState.debugCheat;
	p_oldState.debugRestart         = p_newState.debugRestart;
}


static std::string getTimeAsString(u64 p_timeInMilliSeconds)
{
	static const u32 secondsToHours(3600);
	
	u64 seconds = p_timeInMilliSeconds / 1000;
	
	const u32 hours = static_cast<u32>(seconds / secondsToHours);
	seconds -= (hours * secondsToHours);
	
	const u32 minutes = static_cast<u32>(seconds / 60);
	seconds -= (minutes * 60);
	
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << (hours) << ":"
	    << std::setw(2) << std::setfill('0') << (minutes) << ":"
	    << std::setw(2) << std::setfill('0') << (seconds);
	
	return oss.str();
}


//--------------------------------------------------------------------------------------------------
// Public member functions

Recorder::Recorder()
:
m_state(State_Idle),
m_currentRecording(),
m_currentFrame(0),
m_currentSectionIndex(0),
m_editorOpen(false),
m_verifyGameState(false),
m_isRestoringCheckpoint(false),
m_playbackSpeed(1),
m_isWaitingForInit(false),
m_isWaitingForLevelChange(false),
m_gui()
#if ENABLE_RECORDER_LOGGING
, m_logRecord(),
m_logPlayback()
#endif
{
#if defined(TT_PLATFORM_WIN)
	createGui();
#else
	TT_WARN("Recorder GUI is disabled for all platforms except windows");
#endif
	
#ifndef TT_BUILD_FINAL
	// Enable game state verification for non final builds
	m_verifyGameState = true;
#endif
}


Recorder::~Recorder()
{
	destroy();
}


void Recorder::startRecording()
{
	TT_ASSERT(isActive() == false);
	
	m_state = State_Record;
	
	if (AppGlobal::hasGame())
	{
		// Disable the debug FOV as that messes with the recordings
		AppGlobal::getGame()->resetAndDisableDebugFOV();
	}
	
	m_currentFrame = 0;
	m_currentSectionIndex = 0;
}


void Recorder::stopRecording()
{
	TT_NULL_ASSERT(m_currentRecording);
	m_state = State_Idle;
	m_currentRecording->stop();
	
#if ENABLE_RECORDER_LOGGING
	m_logRecord.flush();
#endif
}


void Recorder::play()
{
	if (m_state == State_Play)
	{
		// Already playing
		return;
	}
	
	if (AppGlobal::hasGame())
	{
		// Disable the debug FOV as that messes with the recordings
		AppGlobal::getGame()->resetAndDisableDebugFOV();
	}
	
	TT_ASSERT(m_state == State_Idle);
	TT_ASSERT(hasRecording());
	
	m_currentFrame = 0;
	m_currentSectionIndex = 0;
	
	m_state = State_Play;
}


void Recorder::stop()
{
	TT_ASSERT(hasRecording());
	
	if(m_state == State_Play)
	{
		//m_playbackSpeed = 1;
		m_state = State_Idle;
		tt::input::resetCurrentControllerType();
		
#if ENABLE_RECORDER_LOGGING
		m_logPlayback.flush();
#endif
	}
	else if(m_state == State_Record)
	{
		stopRecording();
	}
	
	m_gui->updateControls();
	// FIXME: Move this to GUI?
	m_gui->setInfoText("");
}


void Recorder::fastforwardOn()
{
	m_playbackSpeed = cfg()->getIntegerDirect("toki.input.recording_ff_speed");
}


void Recorder::fastforwardOff()
{
	m_playbackSpeed = 1;
}


void Recorder::skipForward()
{
	if (hasRecording() == false) return;
	
	if(m_state == State_Play && m_currentSectionIndex < m_currentRecording->getTotalSections()-1)
	{
		m_isWaitingForInit = true;
		++m_currentSectionIndex;
		m_currentFrame = 0;
		restoreLevel();
	}
}


void Recorder::skipBackward()
{
	if (hasRecording() == false) return;
	
	if(m_state == State_Play)
	{
		if (m_currentSectionIndex > 1)
		{
			m_isWaitingForInit = true;
			--m_currentSectionIndex;
			m_currentFrame = 0;
			restoreLevel();
		}
		else
		{
			// First section should be handled differently
			stop();
			play();
		}
	}
}


bool Recorder::loadRecordedFile(const std::string& p_filename)
{
	m_currentRecording = Recording::load(p_filename);
	if (m_currentRecording == 0)
	{
		return false;
	}
	
	const RecordingHeader& header(m_currentRecording->getHeader());
	updateTimeInfo(header.startTime, header.startTime, header.stopTime);
	
	if(m_gui->isVisible() == false)
	{
		m_gui->toggleVisibility();
	}
	m_gui->handlePlayPressed();
	
	return true;
}


void Recorder::updateElapsedTime(real* p_elapsedTime)
{
	TT_NULL_ASSERT(p_elapsedTime);
	
	// Don't update Recorder if waiting for a level init
	if (m_isWaitingForInit)
	{
		if (m_state == State_Record ||
		    m_state == State_Play)
		{
			(*p_elapsedTime) = AppGlobal::getFixedDeltaTime();
		}
		return;
	}
	
	switch(m_state)
	{
	case State_Record:
		// deltaTime is updated in update.
		break;
	case State_Play:
		{
			if(m_currentRecording == 0   ||
			   m_currentSectionIndex >= m_currentRecording->getTotalSections())
			{
				break;
			}
			
			const RecordingSection& currentSection = m_currentRecording->getSection(m_currentSectionIndex);
			if (currentSection.frames.empty())
			{
				break;
			}
			const Frame& frame = currentSection.frames[m_currentFrame];
			
			*p_elapsedTime = frame.deltaTime;
		}
		break;
		
	default:
		break;
	}
}


void Recorder::update(real p_elapsedTime)
{
	Controller::State& currentInput = AppGlobal::getController(tt::input::ControllerIndex_One).cur;
	
	if(m_gui != 0)
	{
		const bool inputHandledByGwen = m_gui->update(p_elapsedTime);
		
		if(inputHandledByGwen)
		{
			// Prevent camera zoom when scrolling
			currentInput.wheelNotches = 0;
		}
	}
	
	const bool noModifiersDown =
			currentInput.editor.keys[tt::input::Key_Control].down == false &&
			currentInput.editor.keys[tt::input::Key_Alt    ].down == false &&
			currentInput.editor.keys[tt::input::Key_Shift  ].down == false;
	
	if(
#if !ENABLE_RECORDER
	   m_gui != 0 && m_gui->isVisible() && // Only allow the closing if the gui if recorder is disabled.
#endif
	   currentInput.editor.keys[tt::input::Key_R].pressed && noModifiersDown && m_editorOpen == false)
	{
		toggleGui();
	}
	
	// Don't update Recorder if waiting for a level init
	if (m_isWaitingForInit)
	{
		return;
	}
	
	// Prevent editor from being opened while recording/playback
	if (isActive())
	{
		currentInput.toggleEditor.reset();
		currentInput.editor.keys[tt::input::Key_I].reset(); // Not allowed to jump to other location
		currentInput.editor.keys[tt::input::Key_Control].reset(); // Ctrl debug keys disabled
		
		// Not allowed to switch framerate
		if(currentInput.editor.keys[tt::input::Key_Alt].down)
		{
			currentInput.editor.keys[tt::input::Key_F].reset();
		}
	}
	
	switch(m_state)
	{
	case State_Record:
		{
			if(m_currentFrame == 0 && m_currentSectionIndex == 0)
			{
				handleFirstRecordFrame();
			}
			
			TT_NULL_ASSERT(m_currentRecording);
			
			// Record input state
			Frame frame;
			frame.time            = tt::system::Time::getInstance()->getMilliSeconds();
			frame.deltaTime       = p_elapsedTime;
			getCurrentInputState(frame.state);
			
			m_currentRecording->addFrame(frame);
			
			if(m_currentRecording->isGameStateFrame(m_currentFrame))
			{
				// Record game state
				GameState state;
				getCurrentGameState(state);
				m_currentRecording->addGameState(state);
			}
			
			++m_currentFrame;
			
			// FIXME: Move this somewhere else
			const RecordingHeader& header(m_currentRecording->getHeader());
			updateTimeInfo(frame.time, header.startTime, frame.time);
		}
		break;
		
	case State_Play:
		{
			if(m_currentFrame == 0 && m_currentSectionIndex == 0)
			{
				handleFirstPlaybackFrame();
			}
			
			TT_NULL_ASSERT(m_currentRecording);
			
			// Stop condition
			if(m_currentSectionIndex >= m_currentRecording->getTotalSections())
			{
				// End of recording
				stop();
				break;
			}
			
			// FIXME: Move this logic to Recording();
			const RecordingSection& currentSection = m_currentRecording->getSection(m_currentSectionIndex);
			if (currentSection.frames.empty())
			{
				// End of section
				++m_currentSectionIndex;
				m_currentFrame = 0;
				break;
			}
			const Frame& frame = currentSection.frames[m_currentFrame];
			
			p_elapsedTime = frame.deltaTime;
			overrideInput(frame.state, currentInput);
			
			const RecordingHeader& header(m_currentRecording->getHeader());
			if(m_verifyGameState && m_currentRecording->isGameStateFrame(m_currentFrame))
			{
				GameState currentState;
				getCurrentGameState(currentState);
				
				const u32 gameStateFrame = m_currentFrame / header.gameStateFrequency;
				
				TT_ASSERT(gameStateFrame < currentSection.gameStates.size());
				verifyGameState(currentSection.gameStates[gameStateFrame], currentState);
			}
			
			// FIXME: Move this somewhere else
			updateTimeInfo(frame.time, header.startTime, header.stopTime);
			
			++m_currentFrame;
			if (m_currentFrame >= currentSection.frames.size())
			{
				// End of section
				++m_currentSectionIndex;
				m_currentFrame = 0;
			}
		}
		break;
		
	default:
		break;
	}

#if ENABLE_RECORDER_LOGGING
	log() << "*** FRAME ****\n";
#endif
}


void Recorder::renderGui() const
{
	if(m_gui != 0) m_gui->render();
}


void Recorder::createGui()
{
	m_gui = RecorderGui::create(this);
}


void Recorder::toggleGui()
{
	TT_NULL_ASSERT(m_gui);
	m_gui->toggleVisibility();
}



void Recorder::onGameInit(game::StartInfo* p_startInfo)
{
	TT_ASSERT(p_startInfo != 0);
	
	// Only make new section if m_isWaitingForLevelChange is set and when there is a recording
	if (m_isWaitingForLevelChange == false || m_currentRecording == 0)
	{
		return;
	}
	
	if(m_state == State_Record)
	{
		// Start new input recording section
		RecordingSection newSection;
		
		newSection.levelName          = p_startInfo->getLevelName();
		newSection.randomSeed         = tt::math::Random::getStatic().getContextSeedValue();
		
		for (s32 progType = 0; progType < ProgressType_Count; ++progType)
		{
			const ProgressType type = static_cast<ProgressType>(progType);
			newSection.progress[type].registry    = game::script::getRegistry(type);
			newSection.progress[type].checkPoints = AppGlobal::getCheckPointMgr(type).clone();
		}
		
		TT_Printf("**** Starting new recording section. Index: %d Name: %d ****",
			m_currentRecording->getTotalSections(), newSection.levelName.c_str());
		
		m_currentRecording->addSection(newSection);
		
		++m_currentSectionIndex;
		m_currentFrame = 0;
	}
	else if(m_state == State_Play)
	{
		restoreData(p_startInfo);
	}
	
	m_gui->updateControls();
	
	// Reset flags
	m_isWaitingForInit        = false;
	m_isWaitingForLevelChange = false;
}


void Recorder::onLevelExit()
{
	if (m_state == State_Play || m_state == State_Record)
	{
		m_isWaitingForLevelChange = true;
		m_isWaitingForInit        = true;
	}
}


void Recorder::onEditorOpened()
{
	m_editorOpen = true;
	if(m_gui->isVisible()) m_gui->toggleVisibility();
}


void Recorder::onEditorClosed()
{
	m_editorOpen = false;
}


const std::string Recorder::getRecordingRootPath() const
{
	return tt::fs::getSaveRootDir() + "input" + tt::fs::getDirSeparator();
}


const std::string Recorder::getRecordedFilePath() const
{
	if (hasRecording())
	{
		return m_currentRecording->getFilePath();
	}
	
	return getRecordingRootPath();
}


void Recorder::destroy()
{
	// Destroy GUI
	m_gui.reset();
}

bool Recorder::canSkipForward() const
{
	return hasRecording() && (m_currentSectionIndex < m_currentRecording->getTotalSections()-1);
}


bool Recorder::canSkipBackward() const
{
	return hasRecording() && (m_currentSectionIndex > 0);
}


const std::string& Recorder::getCurrentSectionName() const
{
	static std::string emptyString;
	return hasRecording() ? m_currentRecording->getSection(m_currentSectionIndex).levelName : emptyString;
}


s32 Recorder::getCurrentSectionIndex() const
{
	return hasRecording() ? static_cast<s32>(m_currentSectionIndex) : -1;
}


#if ENABLE_RECORDER_LOGGING
std::ostream& Recorder::log()
{
	if(m_state == State_Record)
	{
		return m_logRecord;
	}
	else if(m_state == State_Play)
	{
		return m_logPlayback;
	}
	return std::cout;
}
#endif


//--------------------------------------------------------------------------------------------------
// Private member functions

void Recorder::verifyGameState(const GameState& p_lhs, const GameState& p_rhs)
{
	TT_ASSERTMSG(tt::math::realEqual(p_lhs.currentPosition.x, p_rhs.currentPosition.x),
		"Recorded position.x (%g) is not equal to current(%g)",
		p_lhs.currentPosition.x, p_rhs.currentPosition.x);
		
	TT_ASSERTMSG(tt::math::realEqual(p_lhs.currentPosition.y, p_rhs.currentPosition.y),
		"Recorded position.y (%g) is not equal to current (%g)",
		p_lhs.currentPosition.y, p_rhs.currentPosition.y);
		
	TT_ASSERTMSG(p_lhs.activeEntities == p_rhs.activeEntities,
		"Recorded active entities (%d) is not equal to current (%d)",
		p_lhs.activeEntities, p_rhs.activeEntities);
		
	TT_ASSERTMSG(p_lhs.randomSeedValue == p_rhs.randomSeedValue,
		"Recorded seed (%llu) is not equal to current (%llu)", p_lhs.randomSeedValue, p_rhs.randomSeedValue);
}


void Recorder::getCurrentGameState(GameState& p_state)
{
	// Get important state variables
	
	// Start with clean values.
	p_state.activeEntities = 0;
	p_state.currentPosition.setValues(0.0f, 0.0f);
	
	if (AppGlobal::hasGame())
	{
		toki::game::entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
		p_state.activeEntities  = entityMgr.getActiveEntitiesCount();
	
		// FIXME: controllingentity
		/*
		toki::game::entity::EntityHandle entityHandle = AppGlobal::getGame()->getControllingEntity();
		if(entityMgr.getEntity(entityHandle) != 0)
		{
			p_state.currentPosition = entityMgr.getEntity(entityHandle)->getPosition();
		}
		*/
	}
	
	p_state.randomSeedValue = tt::math::Random::getStatic().getContextSeedValue();
}


void Recorder::getCurrentInputState(InputState& p_state)
{
	const Controller::State& currentInput = AppGlobal::getController(tt::input::ControllerIndex_One).cur;
	p_state.controllerType       = tt::input::getCurrentControllerType();
	p_state.pointer              = currentInput.pointer;
	p_state.accept               = currentInput.accept;
	p_state.cancel               = currentInput.cancel;
	p_state.faceUp               = currentInput.faceUp;
	p_state.faceLeft             = currentInput.faceLeft;
	p_state.left                 = currentInput.left;
	p_state.right                = currentInput.right;
	p_state.up                   = currentInput.up;
	p_state.down                 = currentInput.down;
	p_state.virusUpload          = currentInput.virusUpload;
	p_state.jump                 = currentInput.jump;
	p_state.primaryFire          = currentInput.primaryFire;
	p_state.secondaryFire        = currentInput.secondaryFire;
	p_state.selectWeapon1        = currentInput.selectWeapon1;
	p_state.selectWeapon2        = currentInput.selectWeapon2;
	p_state.selectWeapon3        = currentInput.selectWeapon3;
	p_state.selectWeapon4        = currentInput.selectWeapon4;
	p_state.toggleWeapons        = currentInput.toggleWeapons;
	p_state.demoReset            = currentInput.demoReset;
	p_state.respawn              = currentInput.respawn;
	p_state.menu                 = currentInput.menu;
	p_state.screenSwitch         = currentInput.screenSwitch;
	p_state.startupFailSafeLevel = currentInput.startupFailSafeLevel;
	p_state.direction            = currentInput.direction;
	p_state.gyroStick            = currentInput.gyroStick;
	p_state.scroll               = currentInput.scroll;
	
	p_state.debugCheat           = currentInput.debugCheat;
	p_state.debugRestart         = currentInput.debugRestart;
	
	//tt::input::Button  panCamera;
	//tt::input::Button  toggleEditor;
	//s32 wheelNotches;
	//tt::input::Button toggleHudVisible;
}


void Recorder::handleFirstRecordFrame()
{
	// Store the start time of the recording
	m_currentRecording = Recording::create();
	if (m_currentRecording == 0)
	{
		return;
	}
	
	// Start first section
	RecordingSection newSection;
	
	newSection.levelName          = AppGlobal::getGameStartInfo().getLevelName();
	newSection.randomSeed         = tt::math::Random::getStatic().getContextSeedValue();
	for (s32 progType = 0; progType < ProgressType_Count; ++progType)
	{
		const ProgressType type = static_cast<ProgressType>(progType);
		newSection.progress[type].registry    = game::script::getRegistry(type);
		newSection.progress[type].checkPoints = AppGlobal::getCheckPointMgr(type).clone();
	}
	
	TT_Printf("******** Starting new recording ********\n");
	
	m_currentRecording->addSection(newSection);
	m_gui->updateControls();
	
#if ENABLE_RECORDER_LOGGING
	m_logRecord.open("record.log", std::ios::out|std::ios::trunc);
	game::script::TimerMgr::logTimers();
#endif
}


void Recorder::handleFirstPlaybackFrame()
{
	TT_ASSERT(hasRecording());
	TT_NULL_ASSERT(m_currentRecording->getHeader().startState);
	
	tt::app::getApplication()->setTargetFPS(m_currentRecording->getHeader().targetFPS);
	
	const RecordingSection& currentSection = m_currentRecording->getSection(0);
	
	for (s32 progType = 0; progType < ProgressType_Count; ++progType)
	{
		const ProgressType type = static_cast<ProgressType>(progType);
		// Set persistent registry first, as script onProgressRestored callback depends on it
		game::script::getRegistry(type).copyPersistentRegistry(currentSection.progress[type].registry);
	}
	
	m_isRestoringCheckpoint = true;
	
	// Restore initial checkpoint
	AppGlobal::getGame()->unserializeAll(*m_currentRecording->getHeader().startState, "recorder");
	
	m_isRestoringCheckpoint = false;
	
	// Finally restore rng seed etc
	restoreData(0);
	
#if ENABLE_RECORDER_LOGGING
	m_logPlayback.open("playback.log", std::ios::out|std::ios::trunc);
	game::script::TimerMgr::logTimers();
#endif
}


void Recorder::restoreLevel()
{
	TT_ASSERT(hasRecording());
	
	TT_ASSERT(m_currentSectionIndex < m_currentRecording->getTotalSections());
	if (m_currentSectionIndex != 0 && m_currentSectionIndex >= m_currentRecording->getTotalSections())
	{
		return;
	}
	
	m_isWaitingForLevelChange = true;
	
	const RecordingSection& currentSection = m_currentRecording->getSection(m_currentSectionIndex);
	
	// start level
	game::Game::loadLevel(currentSection.levelName);
}


void Recorder::restoreData(game::StartInfo* p_startInfo) const
{
	TT_ASSERT(m_currentSectionIndex < m_currentRecording->getTotalSections());
	if (m_currentSectionIndex != 0 && m_currentSectionIndex >= m_currentRecording->getTotalSections())
	{
		return;
	}
	
	const RecordingSection& currentSection = m_currentRecording->getSection(m_currentSectionIndex);
	
	for (s32 progType = 0; progType < ProgressType_Count; ++progType)
	{
		const ProgressType type = static_cast<ProgressType>(progType);
		const ProgressSection& progressSection = currentSection.progress[type];
		
		// Set registries
		game::script::getRegistry(type) = progressSection.registry;
		
		// Set the checkpoints
		TT_NULL_ASSERT(progressSection.checkPoints);
		if (progressSection.checkPoints != 0)
		{
			AppGlobal::setCheckPointMgr(progressSection.checkPoints, type);
		}
	}
	// Set seed
	tt::math::Random::getStatic().setContextSeedValue(currentSection.randomSeed);
	
	// Set the level name
	if (p_startInfo != 0)
	{
		p_startInfo->setLevel(currentSection.levelName);
	}
}


void Recorder::updateTimeInfo(u64 p_current, u64 p_start, u64 p_stop)
{
	m_gui->setInfoText(
		getTimeAsString(p_current - p_start) + " / " +
		getTimeAsString(p_stop    - p_start));
}

// Namespace end
}
}
