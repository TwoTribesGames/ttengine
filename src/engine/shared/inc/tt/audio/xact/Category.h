#if !defined(INC_TT_AUDIO_XACT_CATEGORY_H)
#define INC_TT_AUDIO_XACT_CATEGORY_H


#include <string>

#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class Category
{
public:
	explicit Category(const std::string& p_name);
	~Category();
	
	/*! \brief Sets category volume to a new dB value. */
	void setVolume(real p_volumeInDB);
	
	/*! \return Category volume in dB. */
	inline real getVolume() const { return m_volume; }
	
	/*! \brief Sets category reverb mix volume to a new dB value. */
	void setReverbVolume(real p_volumeInDB);
	
	/*! \return Category reverb mix volume in dB. */
	inline real getReverbVolume() const { return m_reverbVolume; }
	
	inline const std::string& getName() const { return m_name; }
	
	static Category* createCategory(const std::string& p_name, xml::XmlNode* p_node);
	
private:
	Category(const Category&);
	Category& operator=(const Category&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	
	real        m_volume;        // in dB, range -96.0 - 12.0
	real        m_reverbVolume;  // in dB, range -96.0 - 12.0
	std::string m_name;          // required for binary loading (lookup)
	
	friend class AudioTT;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_XACT_CATEGORY_H)
