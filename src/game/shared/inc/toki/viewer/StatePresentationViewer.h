#if !defined(INC_TOKI_VIEWER_STATEPRESENTATIONVIEWER_H)
#define INC_TOKI_VIEWER_STATEPRESENTATIONVIEWER_H


#include <map>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/CheckBox.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/GroupBox.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Events.h>

#include <tt/code/State.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/gwen/RootCanvasWrapper.h>
#include <tt/input/Button.h>
#include <tt/pres/CallbackTriggerInterface.h>
#include <tt/pres/fwd.h>
#include <tt/str/str_types.h>

#include <toki/game/Camera.h>
#include <toki/game/TextView.h>


namespace toki {
namespace viewer {

class StatePresentationViewer : public Gwen::Event::Handler, public tt::code::State, public tt::pres::CallbackTriggerInterface
{
public:
	StatePresentationViewer(tt::code::StateMachine* p_stateMachine);
	virtual ~StatePresentationViewer();
	
	virtual void enter();
	virtual void exit();
	virtual void update(real p_deltaTime);
	virtual void updateForRender(real p_deltaTime);
	virtual void render();
	virtual void handleVBlankInterrupt();
	virtual tt::code::StateID getPathToState(tt::code::StateID p_targetState) const;
	
	virtual void onAppActive();
	virtual void onResetDevice();
	virtual void onRequestReloadAssets();
	
	virtual void callback(const std::string& p_data, const tt::pres::PresentationObjectPtr& p_object);
	
private:
	typedef std::vector<Gwen::Controls::CheckBoxWithLabel*> CheckBoxes;
	
	struct UI
	{
		Gwen::Controls::DockBase*          rootDock;
		Gwen::Controls::Base*              contentPanel;
		Gwen::Controls::ListBox*           fileList;
		Gwen::Controls::CheckBoxWithLabel* showGrid;
		Gwen::Controls::CheckBoxWithLabel* flipHorizontal;
		Gwen::Controls::CheckBoxWithLabel* showCallbacks;
		Gwen::Controls::ComboBox*          floorDirection;
		Gwen::Controls::Label*             playStatus;
		Gwen::Controls::ListBox*           namesList;
		Gwen::Controls::ListBox*           renderPassList;
		Gwen::Controls::GroupBox*          tagsGroup;
		CheckBoxes                         tags;
		
		
		inline UI()
		:
		rootDock(0),
		contentPanel(0),
		fileList(0),
		showGrid(0),
		flipHorizontal(0),
		showCallbacks(0),
		floorDirection(0),
		playStatus(0),
		namesList(0),
		renderPassList(0),
		tagsGroup(0),
		tags()
		{ }
	};
	
	
	void setupUi();
	void createGrid();
	tt::str::StringSet getPresentationFilesInDir(const std::string& p_path);
	tt::str::Strings   getRecursiveFileList(const std::string& p_rootPath, bool p_addRootToPath = false);
	void refreshFileList();
	void loadPresentationFile(const std::string& p_filename, bool p_keepSelectedTags);
	
	void setScaleFromUI(const tt::pres::PresentationObjectPtr& p_presentation,
	                    Gwen::Controls::CheckBox*              p_checkBox);
	void setRotationFromUI(const tt::pres::PresentationObjectPtr& p_presentation,
	                       Gwen::Controls::ComboBox*              p_comboBox);
	
	void onFileSelected(Gwen::Controls::Base* p_sender);
	void onFlipHorizontalChanged(Gwen::Controls::Base* p_sender);
	void onFloorDirectionSelected(Gwen::Controls::Base* p_sender);
	void onPickBgColor(Gwen::Controls::Base* p_sender);
	void onBgColorChanged(Gwen::Controls::Base* p_sender);
	void onTagCheckBoxChanged(Gwen::Controls::Base* p_sender);
	void onNameSelected(Gwen::Controls::Base* p_sender);
	void onRenderPassSelected(Gwen::Controls::Base* p_sender);
	
	
	game::Camera*                                m_camera;
	tt::pres::PresentationMgrPtr                 m_presentationMgr;
	tt::pres::PresentationObjectPtr              m_presentationObject;
	std::string                                  m_presentationFilename;
	tt::fs::time_type                            m_presentationFileTimestamp;
	std::string                                  m_nameToStart;
	std::string                                  m_renderPass;
	tt::pres::Tags                               m_tagsToStart;
	tt::input::Button                            m_spacebarScrollMode;
	tt::engine::renderer::TrianglestripBufferPtr m_grid;
	
	tt::str::Strings m_fileList; // Cached copy of the entire list of presentation files (so that changes can be detected)
	
	tt::gwen::RootCanvasWrapper* m_gwenRoot;
	UI                           m_ui;

	game::TextView m_callbackView;
	real           m_mipBias;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_VIEWER_STATEPRESENTATIONVIEWER_H)
