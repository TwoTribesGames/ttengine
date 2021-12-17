#ifndef INC_TT_CODE_FLOODFILLER_H
#define INC_TT_CODE_FLOODFILLER_H

#include <algorithm>
#include <vector>


namespace tt {
namespace code {

template <typename FillCondition, typename Filler, typename PositionType>
class FloodFiller
{
public:
	typedef std::vector<PositionType> TodoList;
	
	FloodFiller(const FillCondition& p_condition,
	            const Filler&        p_filler)
	:
	m_condition(p_condition),
	m_filler(p_filler)
	{}
	
	~FloodFiller()
	{}
	
	inline void instantFill(const PositionType& p_location)
	{
		startFill(p_location);
		while (isDone() == false)
		{
			update();
		}
	}
	
	inline void startFill(const PositionType& p_location)
	{
		m_todo.clear();
		m_todo.push_back(p_location);
	}
	
	inline void addFillPoint(const PositionType& p_location)
	{
		bool ignored = false;
		if (m_condition(p_location, &ignored))
		{
			m_todo.push_back(p_location);
		}
	}
	
	inline void stopFill()
	{
		m_todo.clear();
	}
	
	inline void update()
	{
		TodoList todo;
		std::swap(m_todo, todo);
		
		for (typename TodoList::iterator it = todo.begin(); it != todo.end(); ++it)
		{
			fill(*it);
		}
	}
	
	inline bool isDone() const
	{
		return m_todo.empty();
	}
	
private:
	typedef typename PositionType::ValueType PositionValueType;
	
	inline void fill(const PositionType& p_location)
	{
		bool doNotContinueButStillFill = false;
		if (m_condition(p_location, &doNotContinueButStillFill))
		{
			m_filler(p_location);
			m_todo.push_back(PositionType(p_location.x, p_location.y - PositionValueType(1)));
			m_todo.push_back(PositionType(p_location.x + PositionValueType(1), p_location.y));
			m_todo.push_back(PositionType(p_location.x, p_location.y + PositionValueType(1)));
			m_todo.push_back(PositionType(p_location.x - PositionValueType(1), p_location.y));
		}
		else if (doNotContinueButStillFill)
		{
			m_filler(p_location);
		}
	}
	
	
	FillCondition m_condition;
	Filler        m_filler;
	TodoList      m_todo;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_FLOODFILLER_H)
