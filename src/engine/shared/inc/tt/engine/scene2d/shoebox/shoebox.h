#if !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOX_H)
#define INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOX_H

#include <string>
#include <vector>

#include <tt/code/BitMask.h>
#include <tt/engine/particles/fwd.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/engine/scene2d/shoebox/shoebox_types.h>
#include <tt/engine/scene2d/shoebox/Taggable.h>
#include <tt/engine/scene2d/BinaryPlanePartition.h>
#include <tt/engine/scene2d/WorldScene.h>
#include <tt/engine/scene2d/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/pres/fwd.h>


namespace tt {
namespace engine{
namespace scene2d {
namespace shoebox {

class Shoebox
{
public:
#if !defined(TT_BUILD_FINAL)
	enum LayerVisibleFlag
	{
		LayerVisibleFlag_Background,
		LayerVisibleFlag_BackgroundZero,
		LayerVisibleFlag_ForegroundZeroBack,
		LayerVisibleFlag_ForegroundZeroFront,
		LayerVisibleFlag_Foreground,
		
		LayerVisibleFlag_Count
	};
	typedef code::BitMask<LayerVisibleFlag, LayerVisibleFlag_Count> LayerVisibleFlags;
#endif
	explicit Shoebox(const pres::PresentationMgrPtr& p_presMgr       = pres::PresentationMgrPtr(),
	                 s32                             p_splitPriority = 0);
	~Shoebox();
	
	/*! \brief Convenience function to load a shoebox from file:
	           loads ShoeboxData from file and optionally creates render objects for it.
	    \param p_filename  Full path (filename + extension) to the shoebox file.
	    \param p_levelWidth  Height of the level.
	    \param p_levelHeight  Width of the level.
	    \param p_scale  Scale of the shoebox.
	    \param p_positionOffset By how many world units to offset all shoebox positions.
	    \param p_instantCreate  if this is true create will be called when the loading is done.
	    \return True on succes */
	bool load(const std::string&   p_filename,
	          s32                  p_levelWidth,
	          s32                  p_levelHeight,
	          real                 p_scale,
	          const math::Vector3& p_positionOffset = math::Vector3::zero,
	          s32                  p_priority       = 0,
	          bool                 p_instantCreate  = true);
	
	/*! \brief Initializes all assets of the shoebox.
	           Can only be called if shoebox data was previously loaded using the load() convenience function.
	    \param p_discardLoadedDataAfterCreate Whether to delete the definition data loaded by load()
	                                          after creating the render objects. */
	void create(s32                  p_levelWidth,
	            s32                  p_levelHeight,
	            real                 p_scale                        = 1.0f,
	            const math::Vector3& p_positionOffset               = math::Vector3::zero,
	            s32                  p_priority                     = 0,
	            bool                 p_discardLoadedDataAfterCreate = true);
	
	/*! \brief Initializes all assets of the shoebox based on the shoebox data passed. */
	void create(const ShoeboxData&   p_data,
	            s32                  p_levelWidth,
	            s32                  p_levelHeight,
	            real                 p_scale          = 1.0f,
	            const math::Vector3& p_positionOffset = math::Vector3::zero,
	            s32                  p_priority       = 0);
	
	/*! \brief Copy the texture pointers from p_textures into own texture list
	           if they are needed by the loaded planes. */
	void copyNeededTextures(const renderer::EngineIDToTextures& p_textures);
	
	/*! \brief Returns all the texture pointers that are used by the planes. */
	renderer::EngineIDToTextures getAllUsedTextures(bool p_loadIncludeFiles) const;
	
	void update(real p_deltaTime);
	
	void renderBackground() const;
	void renderBackgroundZero(const tt::math::VectorRect& p_visibilityRect) const;
	void renderForegroundZeroBack(const tt::math::VectorRect& p_visibilityRect) const;
	void renderForegroundZeroFront(const tt::math::VectorRect& p_visibilityRect) const;
	void renderForeground() const;
	
	/*! \brief Renders all the parts of the shoebox in one go.
	           Use this if no other rendering is needed between the separate shoebox sections
	           (background, foreground, etc). */
	void renderAll(const tt::math::VectorRect& p_visibilityRect) const;
	
	/*! \brief Get weither the shoebox is set up for a high end device or a low end device
	    \return true if it is a high end device false if it is a low end device */
	inline bool isHighEndDevice() const { return m_isHighEndDevice; }
	/*! \brief Set weither the shoebox is set up for a high end device or a low end device
	    \param val  true if it is a high end device false if it is a low end device  */
	inline void setHighEndDevice(bool p_val) { m_isHighEndDevice = p_val; }
	
#if !defined(TT_BUILD_FINAL)
	inline const LayerVisibleFlags& getLayerVisibleFlags() const { return m_layerVisibleFlags; }
	inline       LayerVisibleFlags& modifyLayerVisibleFlags()    { return m_layerVisibleFlags; }
#endif
	
	static void changePositionToRendererSpace(math::Vector3* p_position_OUT,
	                                          s32            p_levelWidth,
	                                          s32            p_levelHeight);
	
	static void changePositionToGameSpace(math::Point2* p_position_OUT,
	                                      s32           p_levelWidth,
	                                      s32           p_levelHeight);
	
	//FIXME: Unused? can we remove? inline const renderer::EngineIDToTextures& getTextureContainer() const { return m_textures; }
	
	/*! \brief Returns the shoebox definition data that was loaded from file.
	           Only available if load() was called for this shoebox
	           and create() did not destroy the loaded data (returns null pointer otherwise). */
	inline const ShoeboxDataPtr& getLoadedData() const { return m_loadedData; }
	
	renderer::TexturePtr addToTextureCache(const std::string& p_filename);
	
	static inline void setParticlesPath(const std::string& p_path) { ms_particlesPath = p_path; }
	static inline void setTexturesPath (const std::string& p_path) { ms_texturesPath  = p_path; }
	static const std::string& setTexturesPath()                    { return ms_texturesPath;    }
	static inline void setShoeboxesPath(const std::string& p_path) { ms_shoeboxesPath = p_path; }
	
	// Shoebox dynamic blur
	inline void setBackBlurLayers(const BlurLayers& p_layers) { m_backBlurLayers = p_layers; }
	inline void setForeBlurLayers(const BlurLayers& p_layers) { m_foreBlurLayers = p_layers; }
	
	inline const BlurLayers& getBackBlurLayers() const { return m_backBlurLayers; }
	inline const BlurLayers& getForeBlurLayers() const { return m_foreBlurLayers; }
	
	inline void setBackBlurLayerQuality (BlurQuality p_quality) { m_backBlurQuality  = p_quality; }
	inline void setFrontBlurLayerQuality(BlurQuality p_quality) { m_frontBlurQuality = p_quality; }
	void        setBlurQuality          (BlurQuality p_quality);
	
	inline BlurQuality getBackBlurLayerQuality () const { return m_backBlurQuality;  }
	inline BlurQuality getFrontBlurLayerQuality() const { return m_frontBlurQuality; }
	
	inline s32  getSplitPriority() const         { return m_splitPriority;       }
	inline void setSplitPriority(s32 p_priority) { m_splitPriority = p_priority; }
	
	void invalidateBatches();
	
	inline bool hasParticleCache() const { return m_particleCache.empty() == false; }
	inline void clearParticleCache() { m_particleCache.clear(); }
	
private:
	struct CreationContext
	{
		s32           levelWidth;
		s32           levelHeight;
		real          scale;
		math::Vector3 positionOffset;
		s32           priorityOffset;
		
		
		inline CreationContext()
		:
		levelWidth(0),
		levelHeight(0),
		scale(1.0f),
		positionOffset(math::Vector3::zero),
		priorityOffset(0)
		{ }
	};
	
	struct ParticleDataCache : public Taggable
	{
		ParticleDataCache(const std::string&       p_filename,
		                  const tt::math::Vector3& p_position,
		                  real                     p_scale,
		                  const std::string&       p_parentID,
		                  bool                     p_hidden,
		                  Shoebox*                 p_parentShoebox)
		:
		filename(   p_filename),
		position(   p_position),
		scale(      p_scale),
		parentID(   p_parentID),
		startHidden(p_hidden),
		parentShoebox(p_parentShoebox),
		effect()
		{
			TT_NULL_ASSERT(parentShoebox);
		}
		virtual ~ParticleDataCache();
		
		void spawn();
		void stop();
		void kill();
		virtual bool handleEvent(const std::string& p_event, const std::string& p_param);
		
		// Settings
		std::string       filename;
		tt::math::Vector3 position;
		real              scale;
		std::string       parentID;
		bool              startHidden;
		
	private:
		// Runtime info
		Shoebox*                                 parentShoebox;
		tt::engine::particles::ParticleEffectPtr effect;
	};
	
	typedef std::vector<ParticleDataCache>           ParticleCache;
	typedef std::vector<pres::PresentationObjectPtr> PresentationObjects;
	typedef std::map   <std::string, PlaneScene*>    PlaneSceneIDMapping;
	typedef std::vector<PlaneFollowerPtr>            PlaneFollowers;
	
	
	std::string makeShoeboxFilename         (const std::string& p_filename) const;
	bool        isAllowedToLoadForDeviceType(const std::string& p_filename) const;
	
	void loadAndCreateIncludes(const ShoeboxData& p_data, const CreationContext& p_context);
	void createFromData       (const ShoeboxData& p_data, const CreationContext& p_context);
	
	void createPlane   (const PlaneData&    p_plane,    const CreationContext& p_context);
	void createParticle(const ParticleData& p_particle, const CreationContext& p_context);
	tt::engine::particles::ParticleEffectPtr spawnParticle(const ParticleDataCache& p_particle);
	
	void getAllUsedTextures(const ShoeboxDataPtr& p_data, renderer::EngineIDToTextures& p_usedTextures,
	                        bool p_loadIncludeFiles) const;

	void restoreRenderSettings() const;
	
	// No copying
	Shoebox(const Shoebox&);
	Shoebox& operator=(const Shoebox&);
	
	
	renderer::EngineIDToTextures     m_textures;
	ParticleCache                    m_particleCache; // Cache particle data so we can recreate them later.
	                                                  // It also stored the effect ptr and is a wrapper for taggable.
	ShoeboxData::PresentationObjects m_queuedPresentationObjects;
	PresentationObjects              m_activePresObjects;
	
	WorldScene*          m_background;
	WorldScene*          m_backgroundZero;
	WorldScene*          m_foregroundZeroBack;
	WorldScene*          m_foregroundZeroFront;
	WorldScene*          m_foreground;
	BinaryPlanePartition m_backPartition;
	BinaryPlanePartition m_forePartitionBack;
	BinaryPlanePartition m_forePartitionFront;
	
	ShoeboxDataPtr       m_loadedData;
	PlaneSceneIDMapping  m_planeScenesWithID;
	// Plane Followers currently only used for particles
	PlaneFollowers       m_planeFollowingParticles;
	
	bool m_isHighEndDevice;
	
	pres::PresentationMgrPtr m_presMgr;
	real                     m_currentTime;
	
	s32 m_splitPriority;
	
	BlurLayers  m_backBlurLayers;
	BlurLayers  m_foreBlurLayers;
	BlurQuality m_backBlurQuality;
	BlurQuality m_frontBlurQuality;
	
#if !defined(TT_BUILD_FINAL)
	LayerVisibleFlags m_layerVisibleFlags;
#endif
	
	static std::string ms_particlesPath;
	static std::string ms_texturesPath;
	static std::string ms_shoeboxesPath;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOX_H)
