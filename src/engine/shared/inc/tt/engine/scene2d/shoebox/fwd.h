#if !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_FWD_H)
#define INC_TT_ENGINE_SCENE2D_SHOEBOX_FWD_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {

class Shoebox;
typedef tt_ptr<Shoebox>::shared ShoeboxPtr;
typedef tt_ptr<const Shoebox>::shared ShoeboxConstPtr;

struct ShoeboxData;
typedef tt_ptr<ShoeboxData>::shared ShoeboxDataPtr;

class PlaneFollower;
typedef tt_ptr<PlaneFollower>::shared PlaneFollowerPtr;

class ShoeboxPlane;
class Taggable;
class TagMgr;


// Namespace end
}
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_FWD_H)
