#include <algorithm>

#include <tt/platform/tt_error.h>
#include <tt/thread/Defer.h>


namespace tt {
namespace thread {

Defer::Defers Defer::ms_defers;
u64           Defer::ms_time = 0;


// ------------------------------------------------------------
// Public functions


Defer::~Defer()
{
	Defers::iterator it = std::find(ms_defers.begin(), ms_defers.end(), this);
	if (it != ms_defers.end())
	{
		ms_defers.erase(it);
	}
}


void Defer::defer(u64 p_delay, function p_function, void* p_data)
{
	Defer* defer = new Defer(ms_time + p_delay, p_function, p_data);
	ms_defers.push_back(defer);
	std::sort(ms_defers.begin(), ms_defers.end(), Defer::predicate);
}


void Defer::update(u64 p_delta)
{
	ms_time += p_delta;
	
	if (ms_defers.empty())
	{
		return;
	}
	
	while (ms_defers.empty() == false)
	{
		Defer* def = ms_defers.front();
		if (def->m_time <= ms_time)
		{
			def->trigger();
			delete def;
		}
		else
		{
			break;
		}
	}
}


// ------------------------------------------------------------
// Private functions

Defer::Defer(u64 p_time, function p_function, void* p_data)
:
m_time(p_time),
m_function(p_function),
m_data(p_data)
{
}


void Defer::trigger()
{
	if (m_function != 0)
	{
		m_function(m_data);
	}
}


bool Defer::predicate(const Defer* p_lhs, const Defer* p_rhs)
{
	return p_lhs->m_time < p_rhs->m_time;
}


// namespace end
}
}
