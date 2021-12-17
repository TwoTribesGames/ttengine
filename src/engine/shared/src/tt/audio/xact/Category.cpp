#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/Category.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/xml/XmlNode.h>


//#define CAT_DEBUG
#ifdef CAT_DEBUG
	#define Cat_Printf TT_Printf
#else
	#define Cat_Printf(...)
#endif


#define CAT_WARN
#ifdef CAT_WARN
	#define Cat_Warn TT_Printf
#else
	#define Cat_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

Category::Category(const std::string& p_name)
:
m_volume(0.0f),
m_reverbVolume(-96.0f),
m_name(p_name)
{
}


Category::~Category()
{
}


void Category::setVolume(real p_volumeInDB)
{
	if (p_volumeInDB > 6.0f)
	{
		Cat_Warn("Category::setVolume: volume %f larger than 6.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = 6.0f;
	}
	
	if (p_volumeInDB < -96.0f)
	{
		Cat_Warn("Category::setVolume: volume %f less than -96.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = -96.0f;
	}
	
	m_volume = p_volumeInDB;
}


void Category::setReverbVolume(real p_volumeInDB)
{
	if (p_volumeInDB > 6.0f)
	{
		/*
		Cat_Warn("Category::setReverbVolume: Volume %f larger than 6.0dB; clamping. Category '%s'.\n",
		         realToFloat(p_volumeInDB), m_name.c_str());
		// */
		p_volumeInDB = 6.0f;
	}
	
	if (p_volumeInDB < -96.0f)
	{
		/*
		Cat_Warn("Category::setReverbVolume: Volume %f less than -96.0dB; clamping. Category '%s'.\n",
		         realToFloat(p_volumeInDB), m_name.c_str());
		// */
		p_volumeInDB = -96.0f;
	}
	
	m_reverbVolume = p_volumeInDB;
}


Category* Category::createCategory(const std::string& p_name, xml::XmlNode* p_node)
{
	TT_ASSERT(p_name.empty() == false);
	TT_ASSERTMSG(AudioTT::hasCategory(p_name) == false,
	             "Category name '%s' already exists", p_name.c_str());
	
	if (p_node == 0)
	{
		Cat_Warn("Category::createCategory: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Category")
	{
		Cat_Warn("Category::createCategory: xml node '%s' is not a category node\n", p_node->getName().c_str());
		return 0;
	}
	
	Category* cat = new Category(p_name);
	
	const std::string& volstr(p_node->getAttribute("Volume"));
	if (volstr.empty())
	{
		Cat_Warn("Category::createCategory: no volume specified\n");
	}
	else
	{
		cat->setVolume(xml::fast_atof(volstr.c_str()) / 100.0f);
	}
	
	return cat;
}


bool Category::load(const fs::FilePtr& p_file)
{
	// read members
	fs::readReal(p_file, &m_volume);
	fs::readNarrowString(p_file, &m_name);
	
	return true;
}


bool Category::save(const fs::FilePtr& p_file) const
{
	// write members
	fs::writeReal(p_file, m_volume);
	fs::writeNarrowString(p_file, m_name);
	
	return true;
}

// Namespace end
}
}
}
