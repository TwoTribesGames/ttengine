#ifndef INC_TT_CODE_UNCOPYABLE_H
#define INC_TT_CODE_UNCOPYABLE_H


namespace tt {
namespace code {

/**
 * Basic class for classes which should not be copied.
 * Just inherit privatly from this class and the should prevent copying.
 */
class Uncopyable
{
public:
	// Allow construction and destruction of derived objects
	Uncopyable()          { }
	virtual ~Uncopyable() { }
	
private:
	//... but prevent copying
	Uncopyable(const Uncopyable&);
	Uncopyable& operator=(const Uncopyable&);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_UNCOPYABLE_H)
