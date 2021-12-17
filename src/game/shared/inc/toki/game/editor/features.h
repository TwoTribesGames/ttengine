#if !defined(INC_TOKI_GAME_EDITOR_FEATURES_H)
#define INC_TOKI_GAME_EDITOR_FEATURES_H


// Defines the features available in the editor (depending in the build type and target platform)

// Whether the editor can save levels
#define EDITOR_SUPPORTS_SAVING 1


// Whether the editor can save its settings
#define EDITOR_SUPPORTS_SETTINGS_SAVING 1


// Whether the editor can work on the source assets for levels (assets/source/...)
#if defined(TT_BUILD_FINAL) || !defined(TT_PLATFORM_WIN)
	#define EDITOR_SUPPORTS_ASSETS_SOURCE 0
#else
	#define EDITOR_SUPPORTS_ASSETS_SOURCE 1
#endif


// Whether the editor has Subversion integration
#if defined(TT_BUILD_FINAL) || !defined(TT_PLATFORM_WIN)
	#define EDITOR_SUPPORTS_SUBVERSION 0
#else
	#define EDITOR_SUPPORTS_SUBVERSION 1
#endif


// Whether the editor can display a help page
#define EDITOR_SUPPORTS_HELP_PAGE 1


#endif  // !defined(INC_TOKI_GAME_EDITOR_FEATURES_H)
