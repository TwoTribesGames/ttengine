#ifndef INC_TT_SYSTEM_TIME_H
#define INC_TT_SYSTEM_TIME_H


#include <string>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace system {

/**
 * Time class provides encapsulation around platform dependend time methods,
 * for easy class-wide access to a linear timer.
 */
class Time
{
public:
	inline static Time* getInstance()
	{
		// Create instance
		if (m_instance == 0)
		{
			m_instance = new Time;
		}

		// To be on the safe side ...
		TT_NULL_ASSERT(m_instance);

		// Return object
		return m_instance;
	}

	inline static void  destroyInstance()
	{
		if(m_instance != 0)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

	u64 getMilliSeconds() const;
	u64 getMicroSeconds() const;
	u64 getSeconds() const;

	std::string getNowAsString() const;

private:
	Time();
	~Time();

	Time(const Time&);
	Time& operator=(const Time&);


	static Time* m_instance;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_SYSTEM_TIME_H)

