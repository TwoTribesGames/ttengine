#include <algorithm>

#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Global accessor for application instance

Application* g_globalApplicationInstance = 0;

Application* getApplication()
{
	TT_ASSERTMSG(g_globalApplicationInstance != 0, "Application instance is not available.");
	return g_globalApplicationInstance;
}


bool hasApplication()
{
	return g_globalApplicationInstance != 0;
}


const args::CmdLine& getCmdLine()
{
	static bool          allArgsInitialized = false;
	static args::CmdLine allArgs("");
	
	if (hasApplication())
	{
		return getApplication()->getCmdLine();
	}
	
	if (allArgsInitialized == false)
	{
		allArgs = args::CmdLine::getApplicationCmdLine();
	}
	return allArgs;
}


void Application::registerPlatformCallbackInterface(PlatformCallbackInterface* p_listener)
{
	TT_ASSERTMSG(std::find(m_appListeners.begin(), m_appListeners.end(), p_listener) == m_appListeners.end(),
	             "Shouldn't register the same PlatformCallbackInterface twice");
	
	m_appListeners.push_back(p_listener);
}



void Application::unregisterPlatformCallbackInterface(PlatformCallbackInterface* p_listener)
{
	PlatformCallbackInterfaces::iterator it = 
			std::find(m_appListeners.begin(), m_appListeners.end(), p_listener);
	
	if (it != m_appListeners.end())
	{
		m_appListeners.erase(it);
	}
	else
	{
		TT_PANIC("Couldn't find PlatformCallbackInterface for unregistration.");
	}
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

Application::Application()
:
m_appListeners()
{
	// NOTE: Do not set g_globalApplicationInstance to 'this' here. Derived classes must
	//       make the instance available explicitly by calling makeApplicationAvailable.
	//       Reason for this is that the constructor of the derived class has not executed
	//       yet at this point, so the virtual functions aren't properly set up either.
	//       If client code somehow calls getApplication during this period, behavior is undefined.
}


Application::~Application()
{
	TT_ASSERT(g_globalApplicationInstance == this);
	g_globalApplicationInstance = 0;
}


void Application::makeApplicationAvailable()
{
	TT_ASSERTMSG(g_globalApplicationInstance == 0,
	             "A global application instance is already available.");
	g_globalApplicationInstance = this;
}

// Namespace end
}
}
