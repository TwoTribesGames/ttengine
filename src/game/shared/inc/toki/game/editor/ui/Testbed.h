#if !defined(INC_TOKI_GAME_EDITOR_UI_TESTBED_H)
#define INC_TOKI_GAME_EDITOR_UI_TESTBED_H


#include <Gwen/Controls/WindowControl.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Debugging testbed for testing controls. */
class Testbed : public Gwen::Controls::WindowControl
{
public:
	GWEN_CONTROL(Testbed, Gwen::Controls::WindowControl);
	virtual ~Testbed() { }
	
private:
	// ...
	
	// Disable copy and assignment
	Testbed(const Testbed&);
	Testbed& operator=(const Testbed&);
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_TESTBED_H)
