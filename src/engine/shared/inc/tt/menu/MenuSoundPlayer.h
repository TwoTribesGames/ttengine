#if !defined(INC_TT_MENU_MENUSOUNDPLAYER_H)
#define INC_TT_MENU_MENUSOUNDPLAYER_H


#include <string>

#include <tt/menu/MenuSound.h>


namespace tt {
namespace menu {

/*! \brief Interface class for menu sound players. */
class MenuSoundPlayer
{
public:
	MenuSoundPlayer();
	virtual ~MenuSoundPlayer();
	
	/*! \brief Allows the menu sound player to perform logic if necessary. */
	virtual void update() = 0;
	
	/*! \brief Plays the specified menu sound. */
	virtual void playSound(MenuSound p_sound, bool p_looping) = 0;
	
	/*! \brief Stops all looping sounds. */
	virtual void stopLoopingSounds() = 0;
	
	/*! \brief Plays the menu song. */
	virtual void startSong() = 0;
	
	/*! \brief Stop playing the menu song. */
	virtual void stopSong() = 0;
	
	// MenuSoundPlayer (and derived) objects need to be on the safe heap
	/*
	static void* operator new(std::size_t p_blockSize);
	static void  operator delete(void* p_block);
	//*/
	
	static MenuSound   getSoundEnum(const std::string& p_name);
	static std::string getSoundName(MenuSound p_enum);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUSOUNDPLAYER_H)
