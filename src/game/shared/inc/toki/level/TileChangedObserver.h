#if !defined(INC_TOKI_LEVEL_TILECHANGEDOBSERVER_H)
#define INC_TOKI_LEVEL_TILECHANGEDOBSERVER_H


#include <tt/math/Point2.h>


namespace toki {
namespace level {

class TileChangedObserver
{
public:
	virtual ~TileChangedObserver() { }
	
	virtual void onTileChange(const tt::math::Point2& p_position) = 0;
	
	virtual void onTileLayerDirty() { }
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_TILECHANGEDOBSERVER_H)
