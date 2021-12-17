#if !defined(INC_TT_ENGINE_ANIM2D_ANIMATION2D_H)
#define INC_TT_ENGINE_ANIM2D_ANIMATION2D_H
#include <set>
#include <string>

#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/TweenTypes.h>
#include <tt/fs/types.h>
#include <tt/math/hash/Hash.h>
#include <tt/math/Range.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>
#include <tt/str/str_types.h>


namespace tt {
namespace engine {
namespace anim2d {


typedef tt::math::hash::Hash<32> Tag;
typedef std::set<Tag> Tags;


class Animation2D
{
public:
	enum Preset
	{
		Preset_OneShot,    //!< Play once. (linear/forward/noloop)
		Preset_Looping,    //!< Play looping, when end is reached, start at begin. (linear/forward/loop)
		Preset_PingPong,   //!< When end is reached, reverse animation. (linear/pingpong/loop)
		Preset_Oscillating //!< Same as pingpong but as a sine instead of triangular wave. (easeends/pingpong/loop)
	}; //!< Presets

	enum TimeType
	{
		TimeType_Linear,     //!< Linear
		TimeType_EaseIn,     //!< Start slow, end fast
		TimeType_EaseOut,    //!< Start fast, end slow
		TimeType_EaseEnds,   //!< Slow at endpoints, fast at center (ease in + ease out)
		TimeType_EaseCenter  //!< Fast at endpoints, slow at center (ease out + ease in)
	};
	static TimeType timeTypeFromString(const std::string& p_name);
	static std::string timeTypeToString(const TimeType& p_preset);



	enum DirectionType
	{
		DirectionType_Forward,        //!< 0 -> 1
		DirectionType_Backward,       //!< 1 -> 0
		DirectionType_PingPong,       //!< 0 -> 1 -> 0
		DirectionType_ReversePingPong //!< 1 -> 0 -> 1
	};
	static DirectionType directionTypeFromString(const std::string& p_name);
	static std::string directionTypeToString(const DirectionType& p_preset);

	static Tag loadTag(const xml::XmlNode* p_node, const Tags& p_acceptedTags, 
	                    code::ErrorStatus* p_errStatus);

	Animation2D();
	explicit Animation2D(const Tags& p_tags);
	virtual ~Animation2D() {}
	
	/*! \brief Update logic of this animation.
	    \param p_delta Time passed in seconds.*/
	virtual void update(real p_delta);
	
	/*! \brief Returns the sorting weight, used by autosorting.
	    \return The sorting weight.*/
	virtual int getSortWeight() const = 0;
	
	/*! \brief Starts the animation.*/
	void start();

		/*! \brief Starts the animation.*/
	void start(const Tags& p_tags);
	
	/*! \brief Stops the animation.*/
	inline void stop() { m_active = false; }
	
	/*! \brief Pauses the animation.*/
	inline void pause() { m_paused = true; }
	
	/*! \brief Resumes the animation.*/
	inline void resume() { m_paused = false; }
	
	/*! \brief Unpauses and stops the animation, sets time to 0.*/
	void reset();
	
	/*! \brief Returns whether the animation is active.
	    \return Whether the animation is active.*/
	inline bool isActive() const { return m_active; }

	/*! \brief Returns whether the animation is started with tag filtering.
	    \return Whether the animation is tagged.*/
	inline bool isTagged() const { return m_tagged; }

	/*! \brief Returns whether the animation is paused.
	    \return Whether the animation is paused.*/
	inline bool isPaused() const { return m_paused; }
	
	/*! \brief Sets the playback parameters of the animation via a preset.
	    \param p_type The preset.*/
	void setPreset(Preset p_preset);
	
	/*! \brief Sets the time interpolation type of the animation.
	    \param p_type The time interpolation type.*/
	void setTimeType(TimeType p_type);
	
	/*! \brief Gets the time interpolation type of the animation.
	    \return The time interpolation type of the animation.*/
	inline TimeType getTimeType() const { return m_timeType; }

	/*! \brief Sets the tween interpolation type of the animation.
	    \param p_type The tween interpolation type.*/
	void setTweenType(TweenType p_type);
	
	/*! \brief Gets the tween interpolation type of the animation.
	    \return The tween interpolation type of the animation.*/
	inline TweenType getTweenType() const { return m_tweenType; }

	/*! \brief Sets the playback direction of the animation.
	    \param p_type The playback direction.*/
	void setDirection(DirectionType p_type);
	
	/*! \brief Gets the playback direction  of the animation.
	    \return The playback direction of the animation.*/
	inline DirectionType getDirection() const { return m_directionType; }
	
	/*! \brief Sets whether the animation loops or not.
	    \param p_type Whether the animation should loop or not.*/
	void setLooping(bool p_loop);
	
	/*! \brief Gets whether the animation loops or not.
	    \return Whether the animation loops or not.*/
	inline bool isLooping() const { return m_looping; }
	
	/*! \brief Sets the duration of the animation.
	    \param p_duration The duration type of the animation.*/
	void setDurationRange(math::Range p_duration);
	
	/*! \brief Gets the duration of the animation.
	    \return The duration of the animation.*/
	inline math::Range getDurationRange() const { return m_durationRange; }
	
	/*! \brief Sets the duration of the animation.
	    \param p_duration The duration type of the animation.*/
	void setDuration(real p_duration);
	
	/*! \brief Gets the duration of the animation.
	    \return The duration of the animation.*/
	inline real getDuration() const { return m_duration; }
	
	/*! \brief Sets the duration of the delay.
	    \param p_delay The duration type of the delay.*/
	void setDelayRange(math::Range p_delay);
	
	/*! \brief Gets the duration of the delay.
	    \return The duration of the delay.*/
	inline math::Range getDelayRange() const { return m_delayRange; }
	
	/*! \brief Sets the duration of the delay.
	    \param p_delay The duration type of the delay.*/
	void setDelay(real p_delay);
	
	/*! \brief Gets the duration of the delay.
	    \return The duration of the delay.*/
	inline real getDelay() const { return m_delay; }
	
	/*! \brief Add a tag to the existing set of tags that this Animation will respond to.
	    \param p_tag Tag to add */ 
	inline void addTag(const Tag& p_tag) { m_tags.insert(p_tag); }
	
	/*! \brief Add a set of tags to the existing tags that this Animation will respond to.
	    \param p_tags Tags to add */ 
	inline void addTags(const Tags& p_tags) { m_tags.insert(p_tags.begin(), p_tags.end()); }
	
	/*! \brief Add a tag to the existing set of tags that this Animation will respond to.
	    \param p_tag Tag to add */ 
	inline void addTag(const std::string& p_tag) { m_strTags.push_back(p_tag); }
	
	/*! \brief Add a set of tags to the existing tags that this Animation will respond to.
	    \param p_tags Tags to add */ 
	inline void addTags(const str::Strings& p_tags) 
		{ m_strTags.insert(m_strTags.end(), p_tags.begin(), p_tags.end()); }
	
	/*! \brief Returns the tags of this Animation. */
	inline const Tags& getTags() const { return m_tags; }
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node);
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this animation.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node, 
	                  const Tags& p_applyTags, 
	                  const Tags& p_acceptedTags );
	
	/*! \brief Stores the animation to an xml node.
	    \param p_node The xml node to save to.
	    \return Whether storing succeeded or not.*/
	virtual bool save(xml::XmlNode* p_node) const;
	
	/*! \brief Loads the animation from a file.
	    \param p_file The file to load from.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const fs::FilePtr& p_file);
	
	/*! \brief Stores the animation to a file.
	    \param p_file The file to save to.
	    \return Whether storing succeeded or not.*/
	virtual bool save(const fs::FilePtr& p_file) const;
	
	/*! \brief Loads the animation from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether loading succeeded or not.*/
	virtual bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT);
	
	/*! \brief Loads the animation from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this animation.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	                  const Tags& p_applyTags, 
	                  const Tags& p_acceptedTags );
	
	/*! \brief Save the animation to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT) const;
	
	/*! \brief Returns the size of the buffer needed to load or save this animation.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief Returns a clone of the animation 2D object.*/
	virtual Animation2D* clone() const = 0;
	
protected:
	inline real getTime() const { return m_time; }
	
	/*! \brief Sets internal variables based on ranges.*/
	virtual void setRanges();
	
	//Enable copy / disable assignment
	Animation2D(const Animation2D& p_rhs);
	Animation2D& operator=(const Animation2D&);
	
private:
	
	
	bool m_active;
	bool m_paused;
	
	real m_duration;
	real m_time;
	real m_internalTime;
	real m_delta;
	real m_delay;
	real m_startTime;
	
	bool m_instantAtEnd;
	
	math::Range m_durationRange;
	math::Range m_delayRange;
	
	TimeType      m_timeType;
	TweenType     m_tweenType;
	DirectionType m_directionType;
	bool          m_looping;

	Tags m_tags;
	str::Strings m_strTags;
	bool m_tagged;
};


template <typename T>
class AnimationPredicate2D
{
public:
	inline bool operator()(const T& p_lhs, const T& p_rhs) const
	{
		return p_lhs->getSortWeight() < p_rhs->getSortWeight();
	}
};

//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_ANIMATION2D_H)
