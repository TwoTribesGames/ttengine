#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)

#include <map>
#include <algorithm>

#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_types.h>
#include <tt/str/str.h>
#include <tt/system/Time.h>

#include <squirrel/squirrel.h>
#include <squirrel/sqprofiler/sqprofiler.h>
#include <unordered_map>
#include "memorypool.h"

struct FuncInfo
{
	FuncInfo()
	{
		name = NULL;
		source = NULL;
		id = NULL;
	}
	SQUserPointer id;
	SQInteger line;
	const SQChar* name;
	const SQChar* source;
};

struct CallMetrics {
	CallMetrics()
	{
		ncalls = 0;
		time = 0;
		childrentime = 0;
		ncalled = 0;
		frametime = 0;
		childrenframetime = 0;
		maxframetime = 0;
		maxchildrenframetime = 0;
	}
	FuncInfo *info;
	SQUnsignedInteger ncalls;
	SQUnsignedInteger ncalled;
	u64 time;
	u64 childrentime;
	
	// Per frame information
	u64 frametime;
	u64 childrenframetime;
	u64 maxframetime;
	u64 maxchildrenframetime;
};

struct Call {
	CallMetrics *cm;
	SQUnsignedInteger ncalled;
	u64 starttime;
	u64 childrentime;
};

typedef std::vector<Call> CallStack;
typedef std::unordered_map<SQUserPointer,CallMetrics*> CallMetricsMap;
typedef std::unordered_map<SQUserPointer,FuncInfo*> FuncInfoMap;


struct Thread {
	Thread() {
		cs = NULL;
		cmm = NULL;
	}
	~Thread()
	{
		cs->~CallStack();
		cmm->~CallMetricsMap();
	}
	CallStack *cs;
	CallMetricsMap *cmm;
};

typedef std::unordered_map<SQUserPointer,Thread*> ThreadMap;

ThreadMap *gpThreads = NULL;
FuncInfoMap *gpFuncInfos = NULL;
memorypool<1024*64> *gpMemPool = NULL;
u64 gpFrameCount = 0;
bool gpProfilerActive = false;



void native_hook(HSQUIRRELVM v, SQInteger event_type, const SQChar * sourcename, SQInteger line, const SQChar *funcname)
{
	(void)sourcename;
	(void)line;
	(void)funcname;
	ThreadMap::iterator itr = gpThreads->find(v);
	Thread *threadpp = itr != gpThreads->end() ? itr->second : NULL;
	CallStack *cs = NULL;
	CallMetricsMap* cmm = NULL;
	if(!threadpp) {
		Thread *t = new (gpMemPool->alloc(sizeof(Thread))) Thread();
		t->cs = cs = new (gpMemPool->alloc(sizeof(CallStack))) CallStack();
		t->cmm = cmm = new (gpMemPool->alloc(sizeof(CallMetricsMap))) CallMetricsMap();
		gpThreads->insert(ThreadMap::value_type(v,t));
	}
	else {
		cs = (threadpp)->cs;
		cmm = (threadpp)->cmm;
	}
	//int event_type;
	//sq_getinteger(v,2,&event_type);
	CallMetrics *cm = NULL;
	if(event_type == 'c')
	{
		SQFunctionInfo fi;
		sq_getfunctioninfo(v,0,&fi);
		CallMetricsMap::iterator cmitr = cmm->find(fi.funcid);
		CallMetrics *pp = cmitr != cmm->end() ? cmitr->second : NULL;
		if(!pp) {
			
			
			cm = new (gpMemPool->alloc(sizeof(CallMetrics))) CallMetrics();
			
			FuncInfo *fis = NULL;
			FuncInfoMap::iterator fitr = gpFuncInfos->find(fi.funcid);
			FuncInfo *fipp = fitr != gpFuncInfos->end() ? fitr->second : NULL;
			if(!fipp) {
				fis = new (gpMemPool->alloc(sizeof(FuncInfo))) FuncInfo();
				fis->id = fi.funcid;
				fis->line = fi.line;
				fis->source = gpMemPool->strdup(fi.source);
				fis->name = gpMemPool->strdup(fi.name);
				gpFuncInfos->insert(FuncInfoMap::value_type(fis->id,fis));
			}else {
				fis = fipp;
			}
			cm->info = fis;
			cmm->insert(CallMetricsMap::value_type(fi.funcid,cm));
		}
		else {
			cm = pp;
		}
		cs->push_back(Call());
		Call &c = cs->back();
		c.cm = cm;
		c.starttime = tt::system::Time::getInstance()->getMicroSeconds();
		c.ncalled = 0;
		c.childrentime = 0;
	}
	else if(event_type == 'r'){
		if(cs->empty()) return; //so that if is not started in a top level function it wont break
		Call c = cs->back();
		cs->pop_back();
		u64 totaltime = tt::system::Time::getInstance()->getMicroSeconds() - c.starttime;
		if(!cs->empty()) {
			Call &parent = cs->back();
			parent.childrentime += totaltime;
			parent.ncalled++;
		}
		cm = c.cm;
		cm->ncalls ++;
		cm->ncalled += c.ncalled;
		cm->childrentime += c.childrentime;
		cm->time += totaltime;
		
		// Update frame times
		cm->frametime += totaltime;
		cm->childrenframetime += c.childrentime;
		
		cm->maxframetime         = std::max(cm->maxframetime,         cm->frametime        );
		cm->maxchildrenframetime = std::max(cm->maxchildrenframetime, cm->childrenframetime);
	}
}

SQUIRREL_API SQInteger sq_startprofiler(HSQUIRRELVM v)
{
	TT_ASSERT(gpThreads == NULL);
	gpThreads = new ThreadMap();
	gpFuncInfos = new FuncInfoMap();
	gpMemPool = new memorypool<1024*64>();
	sq_setnativedebughook(v,native_hook);
	gpProfilerActive = true;
	return 0;
}

void _ClearProfiler()
{
	//FOREACH_HASHMAP(ThreadMap,(*gpThreads),itr)
	ThreadMap::iterator itr = gpThreads->begin();
	while(itr != gpThreads->end())
	{
		itr->second->~Thread();
		++itr;
	}
	delete gpThreads;
	delete gpFuncInfos;
	delete gpMemPool;
	gpThreads = NULL;
	gpMemPool = NULL;
	gpFrameCount = 0;
}

SQUIRREL_API SQInteger sq_stopprofiler(HSQUIRRELVM v)
{
	gpProfilerActive = false;
	
	// Dump profile info
	scprintf(_SC("*** sq_stopprofiler total frames: %llu ***\n"), gpFrameCount);

	sq_setnativedebughook(v,NULL);

	for(ThreadMap::iterator thread = gpThreads->begin();thread != gpThreads->end(); ++thread)
	{
		scprintf(_SC("Thread [%p] \n"), thread->first);
		CallMetricsMap *cmm = thread->second->cmm;
		typedef std::multimap<u64, CallMetrics> SortedResults;
		SortedResults results;
		for (CallMetricsMap::iterator itr = cmm->begin(); itr != cmm->end(); ++itr)
		{
			CallMetrics *cm = itr->second;
			// Convert to MS
			//cm->time         /= 1000;
			//cm->childrentime /= 1000;
			results.insert(std::make_pair(cm->maxframetime, *cm));
		}
		
		scprintf(_SC("\tMax T/F\tAvg.T/F\tMax CT/F\tAvg. CT/F\tCALLS\tNCALLED\tSOURCE\n"));
		for (SortedResults::iterator it = results.begin(); it != results.end(); ++it)
		{
			CallMetrics cm = it->second;
			FuncInfoMap::const_iterator funcIt = gpFuncInfos->find(cm.info->id);
			if (funcIt == gpFuncInfos->end())
			{
				TT_PANIC("Cannot find function with id '%p'", cm.info->id);
				continue;
			}
			FuncInfo *func = funcIt->second;
			const std::string name(tt::fs::utils::getFileTitle(func->source) + "(" + tt::str::toStr(func->line) + ")." + std::string(func->name) + "()");
			scprintf(_SC("\t%8llu"), cm.maxframetime);
			scprintf(_SC("\t%8llu"), cm.time / gpFrameCount);
			scprintf(_SC("\t%8llu"), cm.maxchildrenframetime);
			scprintf(_SC("\t%8llu"), cm.childrentime / gpFrameCount);
			scprintf(_SC("\t%8d"), cm.ncalls);
			scprintf(_SC("\t%8d"), cm.ncalled);
			scprintf(_SC("\t%s\n"), name.c_str());
			//scprintf(_SC("\t\ttotal     time %7llu, excl. %7llu, incl. %7llu\n"),cm.time,(cm.time - cm.childrentime), cm.childrentime);
			//scprintf(_SC("\t\tper frame time %7llu, excl. %7llu, incl. %7llu\n"),cm.time / gpFrameCount ,(cm.time - cm.childrentime) / gpFrameCount, cm.childrentime / gpFrameCount);
		}
	}
	_ClearProfiler();
	return 0;
}


SQUIRREL_API SQInteger sq_updateprofiler(HSQUIRRELVM v)
{
	(void)v;
	if (gpProfilerActive)
	{
		++gpFrameCount;
		
		// Reset frame times
		// FIXME: Disable hook here?
		for(ThreadMap::iterator thread = gpThreads->begin();thread != gpThreads->end(); ++thread)
		{
			CallMetricsMap *cmm = thread->second->cmm;
			for (CallMetricsMap::iterator itr = cmm->begin(); itr != cmm->end(); ++itr)
			{
				CallMetrics *cm = itr->second;
				cm->frametime         = 0;
				cm->childrenframetime = 0;
			}
		}
	}
	return 0;
}


void ShutdownScriptProfiler()
{
	if(gpThreads != NULL)
	{
		_ClearProfiler();
	}
}

static SQRegFunction profilerib_funcs[]={
	{_SC("start_profiler"), sq_startprofiler, 0, 0},
	{_SC("stop_profiler"),  sq_stopprofiler,  0, 0},
	{0,0,0,0}
};

void sq_registerscriptprofiler(HSQUIRRELVM v)
{
	SQInteger i=0;
	while(profilerib_funcs[i].name!=0)
	{
		sq_pushstring(v,profilerib_funcs[i].name,-1);
		sq_newclosure(v,profilerib_funcs[i].f,0);
		sq_setparamscheck(v,profilerib_funcs[i].nparamscheck,profilerib_funcs[i].typemask);
		sq_setnativeclosurename(v,-1,profilerib_funcs[i].name);
		sq_newslot(v,-3,SQFalse);
		i++;
	}
}

#endif // #ifndef TT_BUILD_FINAL && defined(TT_PLATFORM_WIN)
