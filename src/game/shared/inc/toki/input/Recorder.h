#if !defined(INC_TOKI_INPUT_RECORDER_H)
#define INC_TOKI_INPUT_RECORDER_H

#include <fstream>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>

#include <toki/game/fwd.h>
#include <toki/input/Recording.h>
#include <toki/serialization/fwd.h>


#define ENABLE_RECORDER_LOGGING 0


namespace toki {
namespace input {


class Recorder
{
public:
	Recorder();
	~Recorder();
	
	enum State
	{
		State_Idle,
		State_Record,
		State_Play
	};
	
	// Recording
	void startRecording();
	void stopRecording();
	
	// Playback
	void play();
	void stop();
	void fastforwardOn();
	void fastforwardOff();
	void skipForward();
	void skipBackward();
	
	bool loadRecordedFile(const std::string& p_filename);
	
	void updateElapsedTime(real* p_elapsedTime);
	void update(real p_elapsedTime);
	
	// GUI
	void renderGui() const;
	void createGui();
	void toggleGui();

	// Callbacks
	void onGameInit(game::StartInfo* p_startInfo);
	void onLevelExit();
	void onEditorOpened();
	void onEditorClosed();
	
	const std::string getRecordingRootPath() const;
	const std::string getRecordedFilePath() const;
	
	void destroy();
	inline void setGameVerificationEnabled(bool p_enabled) { m_verifyGameState = p_enabled; }
	
	inline bool isRestoringCheckpoint() const { return m_isRestoringCheckpoint;  }
	
	inline s32 getPlaybackSpeed() const { return m_playbackSpeed; }
	
	inline State getState() const { return m_state; }
	
	inline bool hasRecording() const { return m_currentRecording != 0; }
	bool canSkipForward() const;
	bool canSkipBackward() const;
	const std::string& getCurrentSectionName() const;
	s32 getCurrentSectionIndex() const;
	inline u32 getTotalSections() const
	{
		return hasRecording() ? m_currentRecording->getTotalSections() : 0;
	}
	
	inline bool isActive() const { return m_state != State_Idle; }
	
#if ENABLE_RECORDER_LOGGING
	std::ostream& log();
#endif
	
private:
	void verifyGameState(const GameState& p_lhs, const GameState& p_rhs);
	void getCurrentGameState(GameState& p_state);
	void getCurrentInputState(InputState& p_state);
	
	void handleFirstRecordFrame();
	void handleFirstPlaybackFrame();
	void restoreLevel();
	void restoreData(game::StartInfo* p_startInfo) const;
	
	void initRecordFile();
	void writeFrameToFile();
	void finishRecordFile();
	
	void updateTimeInfo(u64 p_current, u64 p_start, u64 p_stop);
	
	State           m_state;
	RecordingPtr    m_currentRecording;
	u32             m_currentFrame;
	u32             m_currentSectionIndex;
	bool            m_editorOpen;
	bool            m_verifyGameState;
	bool            m_isRestoringCheckpoint;
	u32             m_playbackSpeed;
	bool            m_isWaitingForInit;
	bool            m_isWaitingForLevelChange;
	
	RecorderGuiPtr  m_gui;
	
#if ENABLE_RECORDER_LOGGING
	std::ofstream   m_logRecord;
	std::ofstream   m_logPlayback;
#endif
};


// Namespace end
}
}

#endif // INC_TOKI_INPUT_RECORDER_H
