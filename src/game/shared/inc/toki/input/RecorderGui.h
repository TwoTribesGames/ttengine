#if !defined(INC_TOKI_INPUT_RECORDERGUI_H)
#define INC_TOKI_INPUT_RECORDERGUI_H


#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Events.h>

#include <tt/engine/renderer/fwd.h>
#include <tt/gwen/RootCanvasWrapper.h>
#include <tt/platform/tt_types.h>

#include <toki/input/fwd.h>


namespace toki {
namespace input {


class RecorderGui : public Gwen::Event::Handler
{
public:
	static RecorderGuiPtr create(Recorder* p_recorder);
	virtual ~RecorderGui();
	
	bool update(real p_deltaTime);
	void render() const;
	
	void handlePlayPressed();
	inline void toggleVisibility()
	{
		m_visible = !m_visible;
		setResetProgressConfirmationVisible(false);  // NOTE: Community only: toggling UI disables confirmation
	}
	inline bool isVisible() const { return m_visible; }
	
	void updateControls();
	void setInfoText(const std::string& p_text);
	
private:
	RecorderGui(Recorder* p_recorder);
	
	// No copying
	RecorderGui(const RecorderGui&);
	RecorderGui& operator=(const RecorderGui&);
	
	void setupGui();
	Gwen::Controls::Base* createLoadWindow(Gwen::Controls::Base* p_parent);
	
	/*! \brief Shows a "Saving..." indicator and stops recording the next frame
	           (so the indicator has a chance to render / become visible). */
	void stopRecordingWithIndicator();
	void updateSaveIndicator();
	
	// NOTE: Community only! Shows or hides the "reset progress" confirmation UI
	void setResetProgressConfirmationVisible(bool p_visible);
	
	// Event Handlers
	void onRecordPressed     (Gwen::Controls::Base* p_sender);
	void onStopPressed       (Gwen::Controls::Base* p_sender);
	void onPlayPressed       (Gwen::Controls::Base* p_sender);
	void onFastForwardPressed(Gwen::Controls::Base* p_sender);
	void onSkipForwardPressed(Gwen::Controls::Base* p_sender);
	void onSkipBackPressed   (Gwen::Controls::Base* p_sender);
	void onShowFilePressed   (Gwen::Controls::Base* p_sender);
	void onOpenFilePressed   (Gwen::Controls::Base* p_sender);
	void onLoadRecording     (Gwen::Controls::Base* p_sender);
	
	void onResetProgressPressed();         // NOTE: Community only!
	void onConfirmResetProgressPressed();  // NOTE: Community only!
	void onCancelResetProgressPressed();   // NOTE: Community only!
	
	void showFileOpenWindow();
	void refreshFileList();
	
	bool                           m_visible;
	tt::gwen::RootCanvasWrapper    m_gwenRoot;
	Gwen::Controls::DockBase*      m_rootDock;
	Gwen::Controls::Button*        m_recordButton;
	Gwen::Controls::Button*        m_playButton;
	Gwen::Controls::Button*        m_stopButton;
	Gwen::Controls::Button*        m_ffButton;
	Gwen::Controls::Button*        m_skipFwdButton;
	Gwen::Controls::Button*        m_skipBackButton;
	Gwen::Controls::Button*        m_showFileButton;
	Gwen::Controls::Button*        m_openFileButton;
	Gwen::Controls::Label*         m_stateLabel;
	Gwen::Controls::Label*         m_infoLabel;
	Gwen::Controls::Label*         m_sectionLabel;
	Gwen::Controls::WindowControl* m_loadWindow;
	Gwen::Controls::ListBox*       m_fileList;
	
	Gwen::Controls::Label*         m_submitLabel1;
	Gwen::Controls::Label*         m_submitLabel2;
	
	// NOTE: Community only! Variables needed for the reset progress UI
	bool                           m_resetProgressConfirmationVisible;
	Gwen::Controls::Button*        m_resetProgressButton;
	Gwen::Controls::Label*         m_confirmResetProgressLabel;
	Gwen::Controls::Button*        m_confirmResetProgressButton;
	Gwen::Controls::Button*        m_cancelResetProgressButton;
	
	tt::engine::renderer::QuadSpritePtr m_saveIndicator;
	bool                                m_stopRecordingThisFrame;
	
	Recorder*                      m_recorder;
};


// Namespace end
}
}

#endif // INC_TOKI_INPUT_RECORDERGUI_H
