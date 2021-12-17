#if !defined(INC_TT_SCRIPT_WRAPPERS_VISUALBOYWRAPPER_H)
#define INC_TT_SCRIPT_WRAPPERS_VISUALBOYWRAPPER_H


namespace tt {
namespace script {
namespace wrappers {

/*! \brief 'VisualBoy' in Squirrel. */
class VisualBoyWrapper
{
public:
	static void load(const std::string& p_rom);
	static void setSoundVolume(real p_volume);
	static void setFilter(const std::string& p_filter);
	static void pause();
	static bool isPaused();
	static void unpause();
	static void unload();
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};


// Namespace end
}
}
}

#endif // !defined(INC_TT_SCRIPT_WRAPPERS_VISUALBOYWRAPPER_H)
