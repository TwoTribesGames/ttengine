#ifndef INC_SAVEFS_CARDINTERFACE_H
#define INC_SAVEFS_CARDINTERFACE_H

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/savefs/CardTypes.h>


namespace tt {
namespace savefs {

// forward declaration
class CardErrorHandler;

class CardInterface
{
public:
	static bool initialize(CardType          p_type,
	                       CardErrorHandler* p_handler,
	                       fs::identifier    p_baseFSType = 0);
	static bool read(u32 p_src, void* p_dest, u32 p_len, bool p_dummy = false);
	static bool write(void* p_src, u32 p_dest, u32 p_len);
	static bool end();
	
	static void setTerminateOnRemoval(bool p_terminate);
	static bool getTerminateOnRemoval();
	
private:
	CardInterface();
	~CardInterface();
	
	static int cardRemovalCallback();
};

}
}


#endif // INC_SAVEFS_CARDINTERFACE_H
