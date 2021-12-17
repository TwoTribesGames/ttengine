#if !defined(INC_TT_PRES_CUETOTAG_H)
#define INC_TT_PRES_CUETOTAG_H
#include <map>
#include <string>
#include <vector>

#include <tt/code/ErrorStatus.h>
#include <tt/math/Random.h>
#include <tt/platform/tt_types.h>
#include <tt/pres/fwd.h>



namespace tt {
namespace pres {


class CueToTag
{
public:
	CueToTag();
	
	/*! \brief loads the cue to tag from file
	    \param p_file path + extension to xml cuetotag file
	    \param p_requiredCues the cues that have to present be in the cueToTag file
	    \param p_usedTagsOUT Pointer to tags that will be filled in with the found tags in the cuetotag file
	                             NOTE: it will clear the container before starting.
	    \param p_errStatus an ErrorStatus object
	    \return returns true on succes */ 
	bool load(const std::string& p_file, const Cues& p_requiredCues, 
	          Tags* p_usedTagsOUT, code::ErrorStatus* p_errStatus);
	
	/*! \brief Converts a cue to a set of tags
	    \param p_cue cue that should be converter
	    \param p_random optional Random object
	    \return converted tags */ 
	const Tags& cueToTag(const Cue& p_cue, math::Random& p_random = math::Random::getEffects()) const;
	
private:
	struct TagSet
	{
		Tags tags;
		s32  weight;
		
		inline TagSet()
		:
		tags(),
		weight(0)
		{ }
	};
	typedef std::vector<TagSet> TagSets;
	typedef std::map<Cue, TagSets> CueTagMapping;
	
	static const Tags ms_emptyTags;
	CueTagMapping m_cueMapping;
	bool m_loaded;
	
	// Disable Copy/assignment
	CueToTag(const CueToTag&);
	CueToTag& operator=(const CueToTag&);	
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_CUETOTAG_H)
