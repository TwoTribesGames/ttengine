#if !defined(INC_TT_CODE_IDENTIFIER_H)
#define INC_TT_CODE_IDENTIFIER_H


#include <tt/platform/tt_error.h>


namespace tt {
namespace code {

/*! \brief Base class for type-safe identifiers. */
template<typename Type>
class Identifier
{
public:
	/*! \brief Registers a new identifier. */
	Identifier()
	:
	m_valid(true),
	m_id(ms_staticCount)
	{
		++ms_staticCount;
		TT_ASSERTMSG(ms_lock == false,
		             "Can't define new identifiers after the "
		             "identifier type has been locked.");
	}
	
	inline Identifier& operator++()
	{
		TT_ASSERTMSG(m_valid, "This identifier is invalid.");
		if (m_valid && m_id < ms_staticCount)
		{
			++m_id;
		}
		else
		{
			TT_PANIC("Cannot increment this identifier further, "
			         "because it has reached the maximum value (%d).",
			         ms_staticCount);
		}
		return *this;
	}
	
	inline Identifier operator++(int)
	{
		Identifier old(*this);
		operator++();
		return old;
	}
	
	inline Identifier& operator--()
	{
		TT_ASSERTMSG(m_valid, "This identifier is invalid.");
		if (m_valid && m_id > 0)
		{
			--m_id;
		}
		else
		{
			TT_PANIC("Cannot decrement this identifier further, "
			         "because it has reached the minimum value (0).");
		}
		return *this;
	}
	
	inline Identifier operator--(int)
	{
		Identifier old(*this);
		operator--();
		return old;
	}
	
	inline int getValue() const { return m_id; }
	
	inline bool isValid() const { return m_valid; }
	
	inline bool isFirst() const
	{ return m_valid && m_id == 0; }
	
	inline bool isLast() const
	{ return m_valid && m_id == ms_staticCount - 1; }
	
	inline bool operator==(const Identifier& p_rhs) const
	{ return m_id == p_rhs.m_id; }
	
	inline bool operator!=(const Identifier& p_rhs) const
	{ return m_id != p_rhs.m_id; }
	
	inline bool operator>(const Identifier& p_rhs) const
	{ return m_id > p_rhs.m_id; }
	
	inline bool operator>=(const Identifier& p_rhs) const
	{ return m_id >= p_rhs.m_id; }
	
	inline bool operator<(const Identifier& p_rhs) const
	{ return m_id < p_rhs.m_id; }
	
	inline bool operator<=(const Identifier& p_rhs) const
	{ return m_id <= p_rhs.m_id; }
	
	inline static int getCount() { return ms_staticCount; }
	
	/*! \brief Locks identifier registration, disallowing
	           any further registrations. */
	inline static void lock() { ms_lock = true; }
	
	/*! \brief Returns the first identifier in the range. */
	inline static Identifier first() { return Identifier(0); }
	
	/*! \brief Returns the last identifier in the range. */
	inline static Identifier last() { return Identifier(ms_staticCount - 1); }
	
	/*! \brief Creates an Identifier instance based on the numeric value of an identifier.
	    \return A valid Identifier if the numeric value is valid; Identifier::invalid if not. */
	inline static Identifier createFromValue(int p_value)
	{ return (p_value >= 0 && p_value < ms_staticCount) ? Identifier(p_value) : invalid; }
	
	
	static Identifier invalid;
	
private:
	Identifier(bool /* p_invalidDummy */) : m_valid(false), m_id(-1) { }
	Identifier(int p_id) : m_valid(true), m_id(p_id) { }
	
	
	static int  ms_staticCount;
	static bool ms_lock;
	
	bool m_valid;
	int  m_id;
};


template<typename Type>
Identifier<Type> Identifier<Type>::invalid(false);

template<typename Type>
int Identifier<Type>::ms_staticCount = 0;

template<typename Type>
bool Identifier<Type>::ms_lock = false;

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_IDENTIFIER_H)
