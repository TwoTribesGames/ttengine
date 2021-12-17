#if !defined(INC_TOKI_INPUT_CONTROLLER_H)
#define INC_TOKI_INPUT_CONTROLLER_H


#include <tt/cfg/Handle.h>
#include <tt/input/Button.h>
#include <tt/input/KeyBindings.h>
#include <tt/input/KeyList.h>
#include <tt/input/Pointer.h>
#include <tt/input/Stick.h>
#include <tt/input/Trigger.h>

#include <toki/input/types.h>


namespace toki {
namespace input {


// Game actions that support keybindings
enum BindableAction
{
	BindableAction_Accept,
	BindableAction_Cancel,
	BindableAction_Left,
	BindableAction_Right,
	BindableAction_Up,
	BindableAction_Down,
	BindableAction_OpenMenu,
	BindableAction_Scroll,
	
	BindableAction_Jump,
	BindableAction_Hack,
	BindableAction_ToggleWeapons,
	
	BindableAction_SelectWeapon1,
	BindableAction_SelectWeapon2,
	BindableAction_SelectWeapon3,
	BindableAction_SelectWeapon4,
	
	BindableAction_Count,
	BindableAction_Invalid,
};
inline bool isValidBindableAction(BindableAction p_value)
{ return p_value >= 0 && p_value < BindableAction_Count; }
const char*           getBindableActionName(BindableAction p_enum);
BindableAction getBindableActionFromName(const std::string& p_name);

typedef std::map<BindableAction, tt::input::KeyList> KeyMapping;


class Controller
{
public:
	struct State
	{
		tt::input::Pointer pointer;
		
		// Menu buttons
		tt::input::Button  accept;
		tt::input::Button  cancel;
		tt::input::Button  faceUp;
		tt::input::Button  faceLeft;
		
		// Player Controls
		tt::input::Button left;
		tt::input::Button right;
		tt::input::Button up;
		tt::input::Button down;
		tt::input::Button virusUpload;
		tt::input::Button jump;
		tt::input::Button primaryFire;
		tt::input::Button secondaryFire;
		tt::input::Button selectWeapon1;
		tt::input::Button selectWeapon2;
		tt::input::Button selectWeapon3;
		tt::input::Button selectWeapon4;
		tt::input::Button toggleWeapons;
		tt::input::Button demoReset;
		tt::input::Button respawn;
		tt::input::Button menu; // 'ingame' menu
		tt::input::Button screenSwitch;
		tt::input::Button startupFailSafeLevel;
		tt::input::Stick  direction;
		tt::input::Stick  gyroStick;  // 'stick' based on gyroscope input (pitch and roll currently)
		tt::input::Stick  scroll;
		
		// Debug buttons
		tt::input::Button debugCheat;
		tt::input::Button debugRestart;
		
		tt::input::Button  panCamera;
		
		tt::input::Button  toggleEditor;
		
		s32 wheelNotches;
		
		tt::input::Button toggleHudVisible;
		
		
		struct EditorState
		{
			tt::input::Pointer pointer;
			tt::input::Button  pointerLeft;
			tt::input::Button  pointerMiddle;
			tt::input::Button  pointerRight;
			tt::input::Button  keys[tt::input::Key_Count];
			std::wstring       chars;
			bool               capsLockOn;
			bool               scrollLockOn;
			bool               numLockOn;
			
			
			inline EditorState()
			:
			pointer(),
			pointerLeft(),
			pointerMiddle(),
			pointerRight(),
			chars(),
			capsLockOn(false),
			scrollLockOn(false),
			numLockOn(false)
			{ }
		};
		EditorState editor;
		
		
		inline void reset() { (*this) = State(); }
		
		void mergeState( const State& p_state); // Keep current state and merge new state.  (e.g. a down key stays down.)
		void updateState(const State& p_state, bool p_doNonPlatform); // Update current state based on new state.
		void releaseAll();
		void resetAllAndBlockUntilReleased();
		
		inline State()
		:
		pointer(),
		accept(),
		cancel(),
		faceUp(),
		faceLeft(),
		left(),
		right(),
		up(),
		down(),
		virusUpload(),
		jump(),
		primaryFire(),
		secondaryFire(),
		selectWeapon1(),
		selectWeapon2(),
		selectWeapon3(),
		selectWeapon4(),
		toggleWeapons(),
		demoReset(),
		respawn(),
		menu(),
		screenSwitch(),
		startupFailSafeLevel(),
		direction(),
		gyroStick(),
		scroll(),
		debugCheat(),
		debugRestart(),
		panCamera(),
		toggleEditor(),
		wheelNotches(0),
		toggleHudVisible(),
		editor()
		{ }
	};
	
	Controller();
	~Controller();
	
	void init();
	
	void update(real p_time);   //!< Implemented in platform-specific code
	void updatePlatformState(); //!< Implemented in platform-specific code
	inline void clearPlatformState() { platform.releaseAll(); }
	inline void releaseInput() { cur.releaseAll(); prev.releaseAll(); }
	inline void resetInputAndBlockUntilReleased() { cur.resetAllAndBlockUntilReleased(); prev.resetAllAndBlockUntilReleased(); }
	inline void makeAllPressedDownOnly()          { prev = cur; cur.updateState(prev, false); }
	inline bool isRumbleEnabled() const           { return m_rumbleEnabled; }
	inline void setRumbleEnabled(bool p_enabled)  { m_rumbleEnabled = p_enabled; }
	void        setPointerVisible(bool p_visible);                                // Implemented in platform-specific code
	inline bool isPointerVisible() const { return m_pointerVisible; }
	
	void rumble(RumbleStrength p_strength, real p_durationInSeconds, real p_panning);
	void stopRumble(bool p_immediately = false);
	
	inline void setPointerAutoVisibility(bool p_autoVisibility) { m_pointerAutoVisibility = p_autoVisibility; }
	inline bool getPointerAutoVisibility() const { return m_pointerAutoVisibility; }
	
	// Set whether the controller is required to be connected (shows an error on CAT if required and not connected)
	void setConnectionRequired(bool p_required);  // Implemented in platform-specific code
	
	void onPlatformMenuEnter();
	void onPlatformMenuExit();
	
#if !defined(TT_BUILD_FINAL)
	void debugRender() const;
#else
	inline void debugRender() const { }
#endif
	
	inline GamepadControlScheme getGamepadControlScheme() const { return m_gamepadControlScheme; }
	inline void                 setGamepadControlScheme(GamepadControlScheme p_scheme)  { m_gamepadControlScheme = p_scheme; }
	
	State cur;
	State prev;
	State platform; //!< Platform state is updated every render frame otherwise we might miss input.
	                //   cur is determined based on platform. (Which is updated every gametick, 30 fps.)
	
	void updateCustomKeyBindings(const std::string& p_controllerID, const tt::input::ActionMap& p_bindings);
	static void saveCustomKeyBindings();
	static void loadCustomKeyBindings();
	
private:
	void initRumble(); // Called by init();
	
	tt::input::Stick getNormalizedStick(const tt::input::Stick& p_stick) const;
	void updatePointerVisibility(real p_elapsedTime);
	void startRumble(RumbleStrength p_strength, real p_durationInSeconds, real p_panning); // Implemented in platform-specific code
	void stopRumbleImpl(bool p_immediately);                               // Implemented in platform-specific code
	
	static void copyDefaultKeyBindings(const char* p_dest);
	static void assignKeyBindings();
	
	bool isKeyDown(BindableAction p_action) const; //!< Implemented in platform-specific code
	
	// No copying
	Controller(const Controller&);
	Controller& operator=(const Controller&);
	
	
	bool                 m_rumbleEnabled;
	RumbleStrength       m_rumbleStrength;
	real                 m_rumbleTime;
	real                 m_rumblePanning;
	GamepadControlScheme m_gamepadControlScheme;
	
	bool m_platformMenuOpen;      // Whether the platform menu (e.g. Steam overlay) is currently open
	bool m_pointerAutoVisibility; // Whether pointer's visibility is based on cursor movement
	real m_pointerStationaryTime; // Number of seconds the pointer hasn't moved
	bool m_pointerVisible;
	tt::cfg::HandleReal m_pointerAutohideTimeout;
	
	KeyMapping m_keyMapping;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_INPUT_CONTROLLER_H)
