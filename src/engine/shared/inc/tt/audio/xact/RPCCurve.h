#ifndef INC_TT_AUDIO_XACT_RPCCURVE_H
#define INC_TT_AUDIO_XACT_RPCCURVE_H

#include <string>
#include <vector>

#include <tt/fs/types.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

// Xact parameter curves
enum CurveType
{
	CurveType_Linear,
	CurveType_Fast,
	CurveType_Slow,
	CurveType_SinCos
};


// Variables that can control runtime parameters
enum SoundVariable
{
	SoundVariable_AttackTime,
	SoundVariable_Distance,
	SoundVariable_DopplerPitchScalar,
	SoundVariable_NumCueInstances,
	SoundVariable_OrientationAngle,
	SoundVariable_ReleaseTime,
	SoundVariable_SpeedOfSound,
	SoundVariable_Volume,
	
	SoundVariable_Count,
	SoundVariable_Invalid
};

const char*   getSoundVariableName(SoundVariable p_enum);
SoundVariable getSoundVariableFromName(const std::string& p_name);


// Runtime parameters that can be controlled by sound variables
enum RuntimeParameter
{
	RuntimeParameter_Volume,
	RuntimeParameter_Pitch,
	RuntimeParameter_ReverbSend,
	RuntimeParameter_FilterFrequency,
	RuntimeParameter_FilterQFactor,

	RuntimeParameter_Count
};


struct RPCPoint
{
	real x;
	real y;
	CurveType type;
};
typedef std::vector<RPCPoint> RPCCurvePoints;


class RPCCurve
{
public:
	RPCCurve();
	explicit RPCCurve(const std::string& p_id, bool p_appliesToSound, SoundVariable p_variable, RuntimeParameter p_param);
	
	inline const RPCPoint& getFirstPoint() const { TT_ASSERT(m_curve.empty() == false); return m_curve.front(); }
	inline const RPCPoint& getLastPoint()  const { TT_ASSERT(m_curve.empty() == false); return m_curve.back();  }
	
	/*! \brief Get y value on the curve, at point x */
	real getParameterValue(real p_variable) const;
	
	inline SoundVariable getVariable() { return m_variable; }
	
	static RPCCurve* createCurve(const xml::XmlNode* p_node);
	
private:
	RPCCurve(const RPCCurve&);
	RPCCurve& operator=(const RPCCurve&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	bool addPoint(const xml::XmlNode* p_node);
	
	std::string      m_id;
	bool             m_appliesToSound;
	SoundVariable    m_variable;
	RuntimeParameter m_parameter;
	RPCCurvePoints   m_curve;
	
	friend class RuntimeParameterControl;
};


} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_RPCCURVE_H
