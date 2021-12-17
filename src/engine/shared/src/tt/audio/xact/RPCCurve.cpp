#include <tt/audio/xact/RPCCurve.h>
#include <tt/code/ErrorStatus.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>


namespace tt {
namespace audio {
namespace xact {

//--------------------------------------------------------------------------------------------------
// Helper functions

const char* getSoundVariableName(SoundVariable p_enum)
{
	switch (p_enum)
	{
	case SoundVariable_AttackTime:         return "AttackTime";
	case SoundVariable_Distance:           return "Distance";
	case SoundVariable_DopplerPitchScalar: return "DopplerPitchScalar";
	case SoundVariable_NumCueInstances:    return "NumCueInstances";
	case SoundVariable_OrientationAngle:   return "OrientationAngle";
	case SoundVariable_ReleaseTime:        return "ReleaseTime";
	case SoundVariable_SpeedOfSound:       return "SpeedOfSound";
	case SoundVariable_Volume:             return "Volume";
		
	default:
		TT_PANIC("Invalid SoundVariable value: %d", p_enum);
		return "";
	}
}


SoundVariable getSoundVariableFromName(const std::string& p_name)
{
	for (s32 i = 0; i < SoundVariable_Count; ++i)
	{
		SoundVariable asEnum = static_cast<SoundVariable>(i);
		if (p_name == getSoundVariableName(asEnum))
		{
			return asEnum;
		}
	}
	
	return SoundVariable_Invalid;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

RPCCurve::RPCCurve()
:
m_id(),
m_appliesToSound(false),
m_variable(SoundVariable_Invalid),
m_parameter()
{
}


RPCCurve::RPCCurve(const std::string& p_id, bool p_appliesToSound, SoundVariable p_variable, RuntimeParameter p_param)
:
m_id(p_id),
m_appliesToSound(p_appliesToSound),
m_variable(p_variable),
m_parameter(p_param)
{
	TT_ASSERTMSG(m_parameter == RuntimeParameter_Volume, "Unsupported Control Parameter");
	TT_ASSERTMSG(m_appliesToSound, "Track specific RPC curves are not supported.");
}


real RPCCurve::getParameterValue(real p_variable) const
{
	// Linear curve
	const RPCPoint& first = m_curve.front();
	const RPCPoint& last  = m_curve.back();

	if(p_variable <= first.x) return first.y;
	if(p_variable >= last.x ) return last.y;

	real slope = (last.y - first.y) / (last.x - first.x);

	return first.y + (p_variable - first.x) * slope;
}


RPCCurve* RPCCurve::createCurve(const xml::XmlNode* p_node)
{
	TT_NULL_ASSERT(p_node);

	TT_ERR_CREATE("Parsing RPC Curve");

	std::string curveID = xml::util::parseStr(p_node, "Name", &errStatus);
	u32 param           = xml::util::parseU32(p_node, "Property", &errStatus);
	bool sound          = (xml::util::parseU32(p_node, "Sound", &errStatus) == 1);

	TT_ERR_ASSERT_ON_ERROR();

	// Check parameter
	if(param >= RuntimeParameter_Count)
	{
		TT_PANIC("Invalid parameter value (%d)", param);
		return 0;
	}

	if(param != RuntimeParameter_Volume)
	{
		TT_WARN("Only distance -> volume control is supported");
		return 0;
	}

	// Check variable to control
	SoundVariable soundVariable(SoundVariable_Volume);
	const xml::XmlNode* varNode = p_node->getFirstChild("Variable Entry");
	if(varNode != 0)
	{
		std::string varName = varNode->getAttribute("Name");
		soundVariable = getSoundVariableFromName(varName);
	}

	// Create RPC Curve
	RPCCurve* curve = new RPCCurve(curveID, sound, soundVariable, static_cast<RuntimeParameter>(param));

	const xml::XmlNode* child = p_node->getChild();
	while(child != 0)
	{
		if(child->getName() == "RPC Point")
		{
			curve->addPoint(child);
		}

		child = child->getSibling();
	}
	
	return curve;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool RPCCurve::load(const fs::FilePtr& p_file)
{
	// read members
	fs::readNarrowString(p_file, &m_id);
	fs::readBool(p_file, &m_appliesToSound);
	fs::readEnum<u8, SoundVariable>(p_file, &m_variable);
	fs::readEnum<u8, RuntimeParameter>(p_file, &m_parameter);
	
	TT_ASSERT(m_curve.empty());
	s32 size = 0;
	fs::readInteger(p_file, &size);
	m_curve.reserve(size);
	for (int i = 0; i < size; ++i)
	{
		RPCPoint point;
		fs::readReal(p_file, &point.x);
		fs::readReal(p_file, &point.y);
		fs::readEnum<u8, CurveType>(p_file, &point.type);
		m_curve.push_back(point);
	}
	
	return true;
}


bool RPCCurve::save(const fs::FilePtr& p_file) const
{
	// write members
	fs::writeNarrowString(p_file, m_id);
	fs::writeBool(p_file, m_appliesToSound);
	fs::writeEnum<u8>(p_file, m_variable);
	fs::writeEnum<u8>(p_file, m_parameter);
	
	fs::writeInteger(p_file, m_curve.size());
	for (RPCCurvePoints::const_iterator it = m_curve.begin(); it != m_curve.end(); ++it)
	{
		fs::writeReal(p_file, (*it).x);
		fs::writeReal(p_file, (*it).y);
		fs::writeEnum<u8>(p_file, (*it).type);
	}
	
	return true;
}


bool RPCCurve::addPoint(const xml::XmlNode* p_node)
{
	TT_NULL_ASSERT(p_node);
	
	TT_ERR_CREATE("Parsing RPC Curve Point");
	
	RPCPoint curvePoint;
	curvePoint.x  = xml::util::parseReal(p_node, "X", &errStatus);
	curvePoint.y  = xml::util::parseReal(p_node, "Y", &errStatus);
	u32 curveType = xml::util::parseU32(p_node, "Curve", &errStatus);
	
	TT_ERR_ASSERT_ON_ERROR();
	
	if(errStatus.hasError()) return false;
	
	// Handle curve type
	TT_ASSERTMSG(static_cast<CurveType>(curveType) == CurveType_Linear, "Only linear curves are supported.");
	curvePoint.type = CurveType_Linear;
	
	m_curve.push_back(curvePoint);
	
	return true;
}


// namespace end
}
}
}
