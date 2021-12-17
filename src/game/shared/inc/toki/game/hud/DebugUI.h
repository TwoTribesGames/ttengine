#if !defined(INC_TOKI_GAME_HUD_DEBUGUI_H)
#define INC_TOKI_GAME_HUD_DEBUGUI_H


#include <vector>

#include <Gwen/Controls/CheckBox.h>
#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Events.h>

#include <tt/gwen/RootCanvasWrapper.h>
#include <tt/input/KeyList.h>
#include <tt/platform/tt_types.h>

#include <toki/game/hud/fwd.h>
#include <toki/game/types.h>
#include <toki/constants.h>


namespace toki {
namespace game {
namespace hud {

class DebugUI : public Gwen::Event::Handler
{
#if !defined(TT_BUILD_FINAL)
	
public:
	static DebugUIPtr create();
	~DebugUI();
	
	bool update(real p_deltaTime);
	void render();
	
	void onResetDevice();
	
private:
	struct UiSettings
	{
		enum Window
		{
			Window_DebugRender,
			
			Window_Count
		};
		
		struct WindowSettings
		{
			tt::math::PointRect windowRect;
			
			inline WindowSettings()
			:
			windowRect()
			{ }
		};
		
		
		WindowSettings window[Window_Count];
		
		
		UiSettings();
		
		void setToDefaults();
		
		bool save(const std::string& p_relativeFilename) const;
		bool load(const std::string& p_relativeFilename);
		
		static const char* getWindowName(Window p_window);
	};
	
	enum Modifier
	{
		Modifier_Control,
		Modifier_Alt,
		Modifier_Shift,
		
		Modifier_Count
	};
	typedef tt::code::BitMask<Modifier, Modifier_Count> Modifiers;
	
	struct HotKey
	{
		tt::input::Key actionKey;
		Modifiers      modifiers;
		
		
		inline explicit HotKey(
				tt::input::Key p_actionKey = tt::input::Key_Space,
				Modifiers      p_modifiers = Modifiers())
		:
		actionKey(p_actionKey),
		modifiers(p_modifiers)
		{ }
	};
	
	typedef std::pair<HotKey, DebugRender> DebugRenderHotKey;
	typedef std::vector<DebugRenderHotKey> DebugRenderHotKeys;
	
	typedef void (DebugUI::*ActionHandler)();
	typedef std::pair<HotKey, ActionHandler> GenericHotKey;
	typedef std::vector<GenericHotKey>       GenericHotKeys;
	
	
	DebugUI();
	void setupHotKeys();
	void setupUi();
	void setupUiDebugRender(Gwen::Controls::Base* p_parent);
	void setupUiGameLayers (Gwen::Controls::Base* p_parent);
	void saveUiSettings();
	
	void addDebugRenderHotKey(
			tt::input::Key p_actionKey,
			Modifiers      p_modifiers,
			DebugRender    p_debugRenderFlag);
	void addGenericHotKey(
			tt::input::Key p_actionKey,
			Modifiers      p_modifiers,
			ActionHandler  p_actionHandler);
	
	void toggleVisible();
	void setVisible(bool p_visible);
	
	void toggleGameLayer(GameLayer p_layer);
	
	void onWindowClosed(Gwen::Controls::Base* p_sender);
	void onDebugRenderCheckboxChanged(Gwen::Controls::Base* p_sender);
	void onGameLayerCheckboxChanged  (Gwen::Controls::Base* p_sender);
	
	void hotKeyCycleSectionProfiler();
	void hotKeyToggleCameraFollowEntity();
	void hotKeyToggleFluidGraphics();
	void hotKeyToggleFluidGraphicsDebug();
	void hotKeyToggleTextLabelsBorders();
	inline void hotKeyToggleGameLayerShoeboxBackground() { toggleGameLayer(GameLayer_ShoeboxBackground); }
	inline void hotKeyToggleGameLayerAttributes()        { toggleGameLayer(GameLayer_Attributes);        }
	inline void hotKeyToggleGameLayerShoeboxZero()       { toggleGameLayer(GameLayer_ShoeboxZero);       }
	//inline void hotKeyToggleGameLayerShoeboxForeground() { toggleGameLayer(GameLayer_ShoeboxForeground); } // FIXME: Need shortcut for this...
	//inline void hotKeyToggleGameLayerNotes()             { toggleGameLayer(GameLayer_Notes);             } // FIXME: Need shortcut for this...
	void hotKeyShowLoadLevelDialog();
	void hotKeyToggleFrameCounter();
	void hotKeyResetCameraFovToDefault();
	void hotKeyToggle30FpsMode();
	void hotKeyCrash();
	void hotKeyTakeLevelScreenshot();
	void hotKeyToggleEntityCulling();
	void hotKeyToggleParticleCulling();
	
	// No copying
	DebugUI(const DebugUI&);
	DebugUI& operator=(const DebugUI&);
	
	
	bool                           m_visible;
	tt::gwen::RootCanvasWrapper    m_gwenRoot;
	Gwen::Controls::DockBase*      m_rootDock;
	Gwen::Controls::WindowControl* m_window;
	
	Gwen::Controls::CheckBox* m_debugRenderCheckbox[DebugRender_Count];
	DebugRenderHotKeys        m_debugRenderHotKeys;
	
	Gwen::Controls::CheckBox* m_gameLayerCheckbox[GameLayer_Count];
	
	GenericHotKeys            m_hotKeys;
	
	bool                      m_ignoreEvents;  // hack to make GWEN event handlers ignore being called; used when updating GUI state from code
	
#else
	
public:
	// Empty dummy implementation for final builds
	inline static DebugUIPtr create()        { return DebugUIPtr(new DebugUI); }
	inline bool update(real /*p_deltaTime*/) { return false;                   }
	inline void render()                     {                                 }
	inline void onResetDevice()              {                                 }
	
private:
	inline DebugUI() : Gwen::Event::Handler() { }
	
	// No copying
	DebugUI(const DebugUI&);
	DebugUI& operator=(const DebugUI&);
	
#endif
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_HUD_DEBUGUI_H)
