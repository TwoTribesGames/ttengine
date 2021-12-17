#if !defined(INC_TT_MENU_MENUTRANSITION_H)
#define INC_TT_MENU_MENUTRANSITION_H


#include <tt/math/Rect.h>


namespace tt {
namespace menu {

class MenuTreeNode;


/*! \brief Provides the transition animation between two menus. */
class MenuTransition
{
public:
	enum Direction
	{
		Direction_Left,  // Camera moves left (so X coord goes up)
		Direction_Right  // Camera moves right (so X coord goes down)
	};
	
	
	MenuTransition(Direction     p_direction,
	               MenuTreeNode* p_currentMenu,
	               MenuTreeNode* p_targetMenu);
	~MenuTransition();
	
	void update();
	void render(s32 p_z);
	bool isDone() const;
	
	MenuTreeNode* getTargetMenu();
	
private:
	enum
	{
		Screen_Width     = 256,
		Screen_Height    = 192
		//Transition_Speed = 10
	};
	
	enum TransitionState
	{
		State_CurrentOut,
		State_TargetIn,
		State_Done
	};
	
	
	Direction     m_direction;
	MenuTreeNode* m_currentMenu;
	MenuTreeNode* m_targetMenu;
	
	math::PointRect m_currentRect;
	math::PointRect m_targetRect;
	s32             m_transitionOffset;
	TransitionState m_transitionState;
	
	
	void setStateTargetIn();
	void setStateDone();
	
	// Menu transitions may not be copied
	MenuTransition(const MenuTransition&);
	const MenuTransition& operator=(const MenuTransition&);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUTRANSITION_H)
