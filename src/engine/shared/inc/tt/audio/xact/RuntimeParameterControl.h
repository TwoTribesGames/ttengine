#ifndef INC_TT_AUDIO_XACT_RUNTIMEPARAMETERCONTROL_H
#define INC_TT_AUDIO_XACT_RUNTIMEPARAMETERCONTROL_H

#include <vector>
#include <string>

#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/xml/fwd.h>

namespace tt {
namespace audio {
namespace xact {


typedef std::vector<RPCCurve*> RPCCurves;


class RuntimeParameterControl
{
public:
	explicit RuntimeParameterControl(const std::string& p_name);
	~RuntimeParameterControl();
	
	void addCurve(RPCCurve* p_curve) { m_curves.push_back(p_curve); }
	
	const std::string& getName()   const { return m_name;   }
	const RPCCurves&   getCurves() const { return m_curves; }
	
	static RuntimeParameterControl* createRPC(const std::string& p_name, const xml::XmlNode* p_node);
	
private:
	RuntimeParameterControl(const RuntimeParameterControl&);
	RuntimeParameterControl& operator=(const RuntimeParameterControl&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	std::string m_name;
	RPCCurves   m_curves;
	
	friend class AudioTT;
};


} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_RUNTIMEPARAMETERCONTROL_H
