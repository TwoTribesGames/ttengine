#include <tt/args/CmdLine.h>
#include <tt/app/Application.h>
#include <tt/input/KeyboardController.h>

#include <toki/main/loadstate/LoadStateGenerateMetaData.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {
namespace loadstate {



//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateGenerateMetaData::doLoadStep()
{
#if !defined(TT_BUILD_FINAL)
#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
	if (m_waitingForExit)
	{
		if (tt::input::KeyboardController::isAnyKeyDown())
		{
			tt::app::getApplication()->terminate(true);
		}
	}
	else if (tt::app::getCmdLine().exists("generate_meta_data"))
	{
		level::MetaDataGenerator& generator = AppGlobal::getMetaDataGenerator();
		const std::string outputFilepath = tt::app::getCmdLine().getString("generate_meta_data");
		if (outputFilepath.empty())
		{
			TT_PANIC("Commandline argument 'generate_meta_data' should specify relative output filepath.");
		}
		else
		{
			generator.generate("levels/");
			
			generator.saveToBinaryFile(outputFilepath + "/" + "levels.ttmeta");
			generator.saveToTextFile(outputFilepath + "/" + "levels.txt");
		}
		
		m_waitingForExit = true;
	}
#endif
#endif
}


std::string LoadStateGenerateMetaData::getName() const
{
	if (m_waitingForExit == false)
	{
		return "Generating Meta Data";
	}
	
	return "Generation done. Press key to exit.";
}

//--------------------------------------------------------------------------------------------------
// Private member functions

LoadStateGenerateMetaData::LoadStateGenerateMetaData()
:
m_waitingForExit(false)
{
}


// Namespace end
}
}
}
