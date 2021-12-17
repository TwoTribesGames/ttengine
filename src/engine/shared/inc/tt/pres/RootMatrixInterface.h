#if !defined(INC_TT_PRES_ROOTMATRIXINTERFACE_H)
#define INC_TT_PRES_ROOTMATRIXINTERFACE_H
#include <tt/math/Matrix44.h>


namespace tt {
namespace pres {


class RootMatrixInterface
{
public:
	virtual ~RootMatrixInterface() {}
	
	/*! \brief Gets the root Transform Matrix */ 
	virtual math::Matrix44 getTransform() const = 0;
private:
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_ROOTMATRIXINTERFACE_H)
