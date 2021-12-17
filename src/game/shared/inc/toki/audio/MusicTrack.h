#if !defined(INC_TOKI_AUDIO_MUSICTRACK_H)
#define INC_TOKI_AUDIO_MUSICTRACK_H


#include <string>

#include <tt/audio/player/fwd.h>
#include <tt/code/fwd.h>
#include <tt/math/TimedLinearInterpolation.h>
#include <tt/platform/tt_types.h>

#include <toki/audio/fwd.h>
#include <toki/game/entity/fwd.h>


namespace toki {
namespace audio {

class MusicTrack
{
public:
	static const std::string ms_musicPath;
	static const std::string ms_musicExtension;
	
	struct CreationParams
	{
		inline CreationParams(const std::string& p_musicName)
		:
		musicName(p_musicName)
		{ }
		
		std::string musicName;
	};
	typedef const CreationParams& ConstructorParamType;
	
	
	MusicTrack(const CreationParams& p_creationParams, const MusicTrackHandle& p_ownHandle);
	~MusicTrack();
	
	void play();
	void stop();
	void pause();
	void resume();
	
	bool isPlaying() const;
	
	real getVolume() const;
	void setVolume(real p_normalizedVolume, real p_fadeDuration, bool p_stopTrackAfterFade);
	
	void handleMusicVolumeSettingChanged();  // user changed the music volume
	
	inline void setCallbackEntity(const game::entity::EntityHandle& p_entity)
	{ m_callbackEntity = p_entity; }
	inline const game::entity::EntityHandle& getCallbackEntity() const { return m_callbackEntity; }
	
	void update(real p_deltaTime);
	
	
	inline const MusicTrackHandle& getHandle() const { return m_ownHandle; }
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static MusicTrack* getPointerFromHandle(const MusicTrackHandle& p_handle);
	void invalidateTempCopy() {}
	
private:
	typedef tt_ptr<tt::audio::player::TTIMPlayer>::shared TTIMPlayerPtr;
	typedef tt::math::TimedLinearInterpolation<real> TLI;
	
	
	std::string getTrackFilename() const;
	void applyPlayerVolume(real p_normalizedVolume);
	
	
	MusicTrackHandle                                   m_ownHandle;
	TTIMPlayerPtr                                      m_player;
	tt::audio::player::MusicPlayerCallbackInterfacePtr m_playerCallbackInterface;
	
	CreationParams             m_creationParams;
	TLI                        m_normalizedVolumeFade;
	bool                       m_stopTrackAfterVolumeFade;
	game::entity::EntityHandle m_callbackEntity;
	
	bool m_shouldStartPlaying;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_MUSICTRACK_H)
