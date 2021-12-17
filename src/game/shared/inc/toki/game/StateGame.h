#if !defined(INC_TOKI_GAME_STATEGAME_H)
#define INC_TOKI_GAME_STATEGAME_H


#include <tt/code/State.h>


namespace toki {
namespace game {

class Game;


class StateGame : public tt::code::State
{
public:
	explicit StateGame(tt::code::StateMachine* p_stateMachine);
	virtual ~StateGame();
	
	virtual void enter();
	virtual void exit();
	virtual void update(real p_deltaTime);
	virtual void updateForRender(real p_deltaTime);
	virtual void render();
	virtual void handleVBlankInterrupt();
	virtual tt::code::StateID getPathToState(tt::code::StateID p_targetState) const;
	virtual bool getPrerequisite(tt::code::StateID& p_prerequisite) const;
	
	virtual void onResetDevice();
	virtual void onRequestReloadAssets();
	
	virtual void onAppActive();
	virtual void onAppEnteredBackground();
	virtual void onAppLeftBackground();
	
private:
	Game* m_game;
	bool  m_gameIsLoaded;
	bool  m_firstFrame;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_STATEGAME_H)
