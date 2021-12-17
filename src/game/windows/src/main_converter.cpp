#include <tt/app/Application.h>
#include <tt/app/ComHelper.h>
#include <tt/app/WinApp.h>
#include <tt/snd/OpenALSoundSystem.h>
#include <tt/snd/XAudio2SoundSystem.h>

#include <toki/__revision_autogen.h>
#include <toki/unittest/unittest.h>
#include <toki/AppGlobal.h>
#include <toki/AppMain.h>

#include "audio/constants_win.h"

namespace toki {
namespace input {

void Controller::setPointerVisible(bool)
{
}


void Controller::startRumble(RumbleStrength /*p_strength*/, real /*p_durationInSeconds*/, real /*p_panning*/)
{
	// Nothing to do here: regular update processing takes care of starting rumble
}


void Controller::stopRumbleImpl(bool)
{
}

// Namespace end
}
}

/*

// App systems instantiator
class GameAppSystems :
		public tt::app::AppSystems
{
public:
	GameAppSystems()
	:
	tt::app::AppSystems()
	{ }
	
	virtual tt::snd::SoundSystemPtr instantiateSoundSystem()
	{
	}
};
*/

// Provide linkage for the ConsoleAppender in the log framework
#include <tt/log/Log.h>
tt::log::LogLevel tt::log::ConsoleAppender::m_logLevel = tt::log::LogLevel_TEST;

#include <common/converter_mainFuncTemplate.h>
#include "SourceItemLevel.h"


int main(int p_argc, char* p_argv[])
{
	using namespace assettool::conversion;
	return common::converterMainFunc<level::SourceItemLevel>(p_argc, p_argv);
}
