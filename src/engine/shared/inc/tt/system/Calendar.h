#ifndef INC_TT_SYSTEM_CALENDAR_H
#define INC_TT_SYSTEM_CALENDAR_H

#include <ctime>

#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace system {

class Calendar
{
public:
	inline Calendar()
	{
		mem::zero8(&m_internal, static_cast<mem::size_type>(sizeof(std::tm)));
	}
	
	
	inline Calendar(const Calendar& p_rhs)
	{
		// copy content from rhs
		mem::copy8(&m_internal, &p_rhs.m_internal, static_cast<mem::size_type>(sizeof(std::tm)));
	}
	
	
	inline ~Calendar()
	{
	}
	
	
	inline Calendar& operator=(const Calendar& p_rhs)
	{
		mem::copy8(&m_internal, &p_rhs.m_internal, static_cast<mem::size_type>(sizeof(std::tm)));
		return *this;
	}
	
	
	inline bool operator==(const Calendar& p_rhs) const
	{
		return  m_internal.tm_year == p_rhs.m_internal.tm_year &&
		        m_internal.tm_mon  == p_rhs.m_internal.tm_mon  &&
		        m_internal.tm_mday == p_rhs.m_internal.tm_mday &&
		        m_internal.tm_hour == p_rhs.m_internal.tm_hour &&
		        m_internal.tm_min  == p_rhs.m_internal.tm_min  &&
		        m_internal.tm_sec  == p_rhs.m_internal.tm_sec;
	}
	
	
	inline bool operator>(const Calendar& p_rhs)  const
	{
		if      (m_internal.tm_year < p_rhs.m_internal.tm_year) return false;
		else if (m_internal.tm_year > p_rhs.m_internal.tm_year) return true;
		
		if      (m_internal.tm_mon  < p_rhs.m_internal.tm_mon ) return false;
		else if (m_internal.tm_mon  > p_rhs.m_internal.tm_mon ) return true;
		
		if      (m_internal.tm_mday < p_rhs.m_internal.tm_mday) return false;
		else if (m_internal.tm_mday > p_rhs.m_internal.tm_mday) return true;
		
		if      (m_internal.tm_hour < p_rhs.m_internal.tm_hour) return false;
		else if (m_internal.tm_hour > p_rhs.m_internal.tm_hour) return true;
		
		if      (m_internal.tm_min  < p_rhs.m_internal.tm_min ) return false;
		else if (m_internal.tm_min  > p_rhs.m_internal.tm_min ) return true;
		
		if      (m_internal.tm_sec <= p_rhs.m_internal.tm_sec ) return false;
		
		return true;
	}
	
	
	inline bool operator<(const Calendar& p_rhs)  const
	{
		if      (m_internal.tm_year > p_rhs.m_internal.tm_year) return false;
		else if (m_internal.tm_year < p_rhs.m_internal.tm_year) return true;

		if      (m_internal.tm_mon  > p_rhs.m_internal.tm_mon ) return false;
		else if (m_internal.tm_mon  < p_rhs.m_internal.tm_mon ) return true;

		if      (m_internal.tm_mday > p_rhs.m_internal.tm_mday) return false;
		else if (m_internal.tm_mday < p_rhs.m_internal.tm_mday) return true;

		if      (m_internal.tm_hour > p_rhs.m_internal.tm_hour) return false;
		else if (m_internal.tm_hour < p_rhs.m_internal.tm_hour) return true;

		if      (m_internal.tm_min  > p_rhs.m_internal.tm_min ) return false;
		else if (m_internal.tm_min  < p_rhs.m_internal.tm_min ) return true;

		if      (m_internal.tm_sec >= p_rhs.m_internal.tm_sec ) return false;

		return true;
	}
	
	
	inline bool operator!=(const Calendar& p_rhs) const { return operator==(p_rhs) == false; }
	inline bool operator>=(const Calendar& p_rhs) const { return operator<(p_rhs) == false; }
	inline bool operator<=(const Calendar& p_rhs) const { return operator>(p_rhs) == false; }
	
	static Calendar getCurrentDate();
	
	inline int getYear()  const { return m_internal.tm_year + 1900; }
	inline int getMonth() const { return m_internal.tm_mon + 1; }
	inline int getDay()   const { return m_internal.tm_mday; }
	
	inline int getHour()   const { return m_internal.tm_hour; }
	inline int getMinute() const { return m_internal.tm_min; }
	inline int getSecond() const { return m_internal.tm_sec; }
	
/*! \brief This replaces the old getDayOfWeek which returned weird data
	\return The day of the week. 0 = sunday, 1 = monday, etc */
	inline int getWeekDay() const
	{
		// create a local copy so the method can stay const
		std::tm local(m_internal);
		mktime(&local);
		return local.tm_wday;
	}
	
	inline void setYear (int p_year)  { m_internal.tm_year = p_year - 1900; }
	inline void setMonth(int p_month) { m_internal.tm_mon  = p_month - 1; }
	inline void setDay  (int p_day)   { m_internal.tm_mday = p_day; }
	
	inline void setHour  (int p_hour)   { m_internal.tm_hour = p_hour; }
	inline void setMinute(int p_minute) { m_internal.tm_min  = p_minute; }
	inline void setSecond(int p_second) { m_internal.tm_sec  = p_second; }
	
private:
	std::tm m_internal;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_SYSTEM_CALENDAR_H)
