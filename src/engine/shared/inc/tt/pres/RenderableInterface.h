#if !defined(INC_TT_PRES_RENDERABLEINTERFACE_H)
#define INC_TT_PRES_RENDERABLEINTERFACE_H
#include <tt/math/Matrix44.h>


namespace tt {
namespace pres {


class RenderableInterface
{
public:
	virtual ~RenderableInterface() {}
	
	/*! \brief renders this Renderable */ 
	virtual void render() const = 0;
private:
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_RENDERABLEINTERFACE_H)
