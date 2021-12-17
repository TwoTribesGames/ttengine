#include <squirrel/sqbind.h>

typedef std::vector<bool*> BoolPtrs;

BoolPtrs ms_sqBindRegisteredInits;

void SqBind_RegisterInit(bool* p_init)
{
	ms_sqBindRegisteredInits.push_back(p_init);
}


void SqBind_UnInitAll()
{
	for (BoolPtrs::const_iterator it = ms_sqBindRegisteredInits.begin(); 
		it != ms_sqBindRegisteredInits.end(); ++it)
	{
		*(*it) = false;
	}
	ms_sqBindRegisteredInits.clear();
}
