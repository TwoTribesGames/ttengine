#if !defined(INC_TT_MATH_QUATERNIONUTILS_H)
#define INC_TT_MATH_QUATERNIONUTILS_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace math {

class Quaternion;

class QuaternionUtils
{
public:
	/*! \brief Snaps an axis-aligned quaternion to 90-degree angles. */
	static void snapToAxis(Quaternion& p_quaternion);
	
private:
	static void snapComponent(real& p_value);
	
	QuaternionUtils();
	QuaternionUtils(const QuaternionUtils&);
	~QuaternionUtils();
	const QuaternionUtils& operator=(const QuaternionUtils&);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_QUATERNIONUTILS_H)
