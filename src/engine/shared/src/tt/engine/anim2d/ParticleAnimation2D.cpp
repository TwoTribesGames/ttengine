#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/ParticleAnimation2D.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/particles/ParticleTrigger.h>
#include <tt/fs/File.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>



namespace tt {
namespace engine {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_versionParticleAnimation2D = MAKE_VERSION(1, 0);
static const size_t s_sizeParticleAnimation2D = 2 + 2; // version, stringlen


ParticleAnimation2D::ParticleAnimation2D()
:
PositionAnimation2D(),
m_particlesFile(),
m_worldObject(0),
m_trigger(0)
{
}


ParticleAnimation2D::~ParticleAnimation2D()
{
	using namespace tt::engine::particles;
	if (m_trigger != 0 && ParticleMgr::hasInstance())
	{
		m_trigger->trigger(ParticleTrigger::TriggerType_Stop);
	}
	
}


int ParticleAnimation2D::getSortWeight() const
{
	return 4;
}


bool ParticleAnimation2D::load(const xml::XmlNode* p_node)
{
	TT_NULL_ASSERT(p_node);
	TT_ERR_CREATE("ParticleAnimation2D load from xml");
	
	m_particlesFile = p_node->getAttribute("file");
	if(m_particlesFile.empty())
	{
		TT_PANIC("No file attribute for Particles");
		return false;
	}
	
	if (errStatus.hasError())
	{
		TT_PANIC("%s", errStatus.getErrorMessage().c_str());
		return false;
	}
	
	if (Animation2D::load(p_node) == false)
	{
		return false;
	}
	
	return true;
}


bool ParticleAnimation2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);

	p_node->setAttribute("file",m_particlesFile);
	
	if (Animation2D::save(p_node) == false)
	{
		return false;
	}
	
	return true;
}


bool ParticleAnimation2D::load(const fs::FilePtr& p_file)
{

	using namespace code::bufferutils;
	//////////////////////////////////////////////////////////////////////////
	// read the version
	//////////////////////////////////////////////////////////////////////////
	{
		size_t size = 2;
		u8* buffer = new u8[size];
		if(p_file->read(buffer, static_cast<fs::size_type>(size)) != static_cast<fs::size_type>(size))
		{
			TT_PANIC("File '%s' too small.", p_file->getPath());
			delete[] buffer;
			return false;
		}

		const u8* scratch = buffer;
		

		u16 version = be_get<u16>(scratch, size);
		if (version != s_versionParticleAnimation2D)
		{
			TT_PANIC("Invalid version, code %d.%d, data %d.%d, Please update your converter",
				GET_MAJOR_VERSION(s_versionParticleAnimation2D), GET_MINOR_VERSION(s_versionParticleAnimation2D),
				GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
			return false;
		}	
		delete[] buffer;
	}

	//////////////////////////////////////////////////////////////////////////
	// read the size of the string
	//////////////////////////////////////////////////////////////////////////
	u16 len;
	{
		size_t size = 2;
		u8* buffer = new u8[size];
		if(p_file->read(buffer, static_cast<fs::size_type>(size)) != static_cast<fs::size_type>(size))
		{
			TT_PANIC("File '%s' too small.", p_file->getPath());
			delete[] buffer;
			return false;
		}
		const u8* scratch = buffer;
		// Read the length of the string from the buffer
		len = be_get<u16>(scratch, size);

		delete[] buffer;
	}
	//////////////////////////////////////////////////////////////////////////
	// read the string
	//////////////////////////////////////////////////////////////////////////

	{
		size_t size = len;
		u8* buffer = new u8[size];
		if(p_file->read(buffer, static_cast<fs::size_type>(size)) != static_cast<fs::size_type>(size))
		{
			TT_PANIC("File '%s' too small.", p_file->getPath());
			delete[] buffer;
			return false;
		}

		const u8* scratch = buffer;

		// Read the characters that make up the string from the buffer
		m_particlesFile.reserve(static_cast<std::string::size_type>(len));
		for (u16 i = 0; i < len; ++i)
		{
			m_particlesFile += static_cast<std::string::value_type>(be_get<u8>(scratch, size));
		}

		delete[] buffer;
	}
	
	return true;
}


bool ParticleAnimation2D::save(const fs::FilePtr& p_file) const
{
	size_t bufsize = getBufferSize();
	u8* buffer = new u8[bufsize];
	u8* scratch = buffer;
	size_t size = bufsize;
	if (save(scratch, size) == false)
	{
		delete[] buffer;
		return false;
	}
	
	if (p_file->write(buffer, static_cast<fs::size_type>(bufsize)) != static_cast<fs::size_type>(bufsize))
	{
		delete[] buffer;
		TT_PANIC("Failed to save %d bytes to '%s'", bufsize, p_file->getPath());
		return false;
	}
	delete[] buffer;
	return true;
}


bool ParticleAnimation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{

	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_versionParticleAnimation2D)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d",
		         GET_MAJOR_VERSION(s_versionParticleAnimation2D), GET_MINOR_VERSION(s_versionParticleAnimation2D),
		         GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}	
	
	// Read the length of the string from the buffer
	u16 len = be_get<u16>(p_bufferOUT, p_sizeOUT);

	// Make sure enough bytes remain in the buffer for the string itself
	if (len > p_sizeOUT)
	{
		TT_PANIC("String length (%u) is longer than the number of remaining bytes in the buffer (%u).",
			len, p_sizeOUT);
		p_bufferOUT        += p_sizeOUT;
		p_sizeOUT = 0;
		return false;
	}

	// Read the characters that make up the string from the buffer
	m_particlesFile.reserve(static_cast<std::string::size_type>(len));
	for (u16 i = 0; i < len; ++i)
	{
		m_particlesFile += static_cast<std::string::value_type>(be_get<u8>(p_bufferOUT, p_sizeOUT));
	}

	return Animation2D::load(p_bufferOUT, p_sizeOUT);
}


bool ParticleAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT) const
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	be_put(s_versionParticleAnimation2D, p_bufferOUT, p_sizeOUT);
	

	// Write out the string length (in bytes, without null terminator)
	u16 stringByteLength = static_cast<u16>(m_particlesFile.length());
	be_put(stringByteLength, p_bufferOUT, p_sizeOUT);

	// Write out the individual characters of the string
	for (std::string::const_iterator it = m_particlesFile.begin(); it != m_particlesFile.end(); ++it)
	{
		be_put(static_cast<u8>(*it), p_bufferOUT, p_sizeOUT);
	}
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT);
}


size_t ParticleAnimation2D::getBufferSize() const
{
	return s_sizeParticleAnimation2D + Animation2D::getBufferSize() + (m_particlesFile.size() + 2);
}


void ParticleAnimation2D::setRanges()
{
	Animation2D::setRanges();

	
	//m_delta = m_end - m_begin;
}


void ParticleAnimation2D::update(real p_delta)
{
	PositionAnimation2D::update(p_delta);

	using namespace tt::engine::particles;

	if(isActive() && Animation2D::getTime()>0 && m_trigger == 0)
	{
		m_trigger = ParticleMgr::getInstance()->addTrigger(m_particlesFile,m_worldObject);
		m_trigger->trigger(ParticleTrigger::TriggerType_Start);
	}
	if(isActive() == false && m_trigger != 0)
	{
		m_trigger->trigger(ParticleTrigger::TriggerType_Stop);
	}
}


ParticleAnimation2D::ParticleAnimation2D(const ParticleAnimation2D& p_rhs)
:
PositionAnimation2D(p_rhs),
m_particlesFile(p_rhs.m_particlesFile),
m_worldObject(p_rhs.m_worldObject),
m_trigger(p_rhs.m_trigger->clone())
{
}

//namespace end
}
}
}
