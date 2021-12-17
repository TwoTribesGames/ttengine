#include <tt/engine/scene/Fog.h>
#include <tt/engine/animation/HermiteFloatController.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


Fog::Fog()
:
m_type(Type_None),
m_reference(0),
m_red(),
m_green(),
m_blue(),
m_start(),
m_end()
{
}


bool Fog::load(const fs::FilePtr& p_file)
{
	s32 type(0);
	if(p_file->read(&type, sizeof(type)) != sizeof(type))
	{
		return false;
	}
	m_type = static_cast<Fog::Type>(type);
	
	if(p_file->read(&m_reference, sizeof(m_reference)) != sizeof(m_reference))
	{
		return false;
	}
	
	{
		TT_ERR_CREATE("Loading Fog Controllers");
		
		m_red.reset(new animation::HermiteFloatController);
		m_red->load(p_file, &errStatus);
		
		m_green.reset(new animation::HermiteFloatController);
		m_green->load(p_file, &errStatus);
		
		m_blue.reset(new animation::HermiteFloatController);
		m_blue->load(p_file, &errStatus);
		
		m_start.reset(new animation::HermiteFloatController);
		m_start->load(p_file, &errStatus);
		
		m_end.reset(new animation::HermiteFloatController);
		m_end->load(p_file, &errStatus);
		
		TT_ERR_ASSERT_ON_ERROR();
	}
	
	return true;
}


renderer::ColorRGBA Fog::getColor() const
{
	renderer::ColorRGBA result(255,255,255,255);

	real value(0);

	value = m_red->getValue(0);
	result.r = static_cast<u8>(value);

	value = m_green->getValue(0);
	result.g = static_cast<u8>(value);

	value = m_blue->getValue(0);
	result.b = static_cast<u8>(value);

	return result;
}


real Fog::getStart() const
{
	return m_start->getValue(0);
}


real Fog::getEnd() const
{
	return m_end->getValue(0);
}

// Namespace end
}
} 
}
