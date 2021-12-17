#if !defined(INC_TT_PRES_PRESENTATIONMGR_H)
#define INC_TT_PRES_PRESENTATIONMGR_H


#include <map>
#include <vector>

#include <tt/engine/particles/ParticleMgr.h>
#include <tt/pres/anim2d/TranslationAnimation2D.h>
#include <tt/pres/fwd.h>
#include <tt/pres/GroupFactoryInterface.h>
#include <tt/pres/PresentationGroupInterface.h>
#include <tt/pres/PresentationLoader.h>
#include <tt/pres/PresentationObject.h>
#include <tt/str/str_types.h>


namespace tt {
namespace pres {


class PresentationMgr
{
public:
	/*! \brief converts strings to tags and checks for duplicate hashes
	    \param p_strings a container of strings
	    \return converted tags */ 
	static Tags stringsToTags(const str::Strings& p_strings);
	
	/*! \brief converts strings to cues and checks for duplicate hashes
	    \param p_strings a container of strings
	    \return converted cues */ 
	static Cues stringsToCues(const str::Strings& p_strings);
	
	/*! \brief checks whether there are any unsupported tags or whether there are any missing tags
	    \param p_requiredTags The tags that are required
	    \param p_objectTags The tags that are used and need to be checked
	    \param p_file Name of the file where it went wrong for the assert message
	    \return converted cues */ 
	static void checkRequiredTags(const Tags &p_requiredTags, Tags p_objectTags, 
	                              const std::string& p_file);
	
	static GroupFactoryInterfacePtr getDefaultGroupFactory();
	
	/*! \brief Creates a Presentation Mgr
	    \param p_acceptedTags The accepted tags that will be used if empty tags are given in
	                          createPresentationObject.
	    \param p_triggerFactory Implementation of the triggerFactoryInterface. Triggers will be created with this
	    \param p_particleCategory The particleCategory the created particles should be assigned to.
	    \param p_groupFactory Optional. Can be a self implemented GroupFactoryInterface. 
	                          Or a GroupFactory template class with a PresentationObject sorting predicate.
	                          see the default value and PresentationObjectLess for an example predicate
	    \return converted tags */ 
	static PresentationMgrPtr create(const Tags& p_acceptedTags, 
	                                 const TriggerFactoryInterfacePtr& p_triggerFactory,
	                                 u32 p_particleCategory = engine::particles::Category_All,
	                                 const GroupFactoryInterfacePtr& p_groupFactory = getDefaultGroupFactory());
	
	/*! \brief Adds a custom Render layer to render between the presentationObjects
	    \param p_renderable Pointer to a RenderableInterface to render the layer.
	    \param p_depth Depth at which the layer should be rendered
	    \param p_priority Which layer that should be renderd first if they have the same depth.
	    \return converted tags */ 
	void addCustomRenderLayer(const RenderableInterfacePtr& p_renderable, 
	                          real p_depth, s32 p_priority);
	
	/*! \brief Adds a particle layer where particles can be renderd on.
	    \param p_renderGroup Rendergroup of this particleLayer.
	    \param p_layerName Name of this particle layer as used in the presentationData
	    \param p_depth z depth at which this layer should be renderd.
	    \param p_priority Which layer that should be renderd first if they have the same depth. */
	void createParticleLayer(s32 p_renderGroup, const std::string& p_layerName, 
	                         real p_depth, s32 p_priority);
	
	/*! \brief Adds a mapping from a layer name (which can be specified in presentation files)
	           to a ParticleMgr render group. This function does not do anything else:
	           rendering this particle group is up to client code.
	    \param p_layerName Name of this particle layer as used in the presentationData
	    \param p_renderGroup Which ParticleMgr RenderGroup to set particle effects with this layer name to. */
	void addParticleLayerMapping(const std::string& p_layerName, s32 p_renderGroup);
	
	void update(real p_deltaTime);
	void updateForRender(real p_deltaTime);
	void render() const;
	void renderPass(const std::string& p_passName) const;
	
	/*! \brief Loads a presentation object from file and automaticly updates and renders it 
	           when this presnetationMgr is rendered and updated
	    \param p_file path and filename without extension to the presentation object. Will check for 
	                  .pres or .xml version and load binary or xml if available
	    \param p_requiredTags The required tags the loaded pres has to have. 
	                          Will assert if there are unaccepted or missing tags but will not fail. 
	                          If an empty set is given the presentationmanagers acceptedTags will be used.
	    \param p_autoUpdate Whether the created presentation Object should be 
	                        updated autmaticaly with the presentationMgr
	    \return Shared Ptr to the Presentation Object. */ 
	PresentationObjectPtr createPresentationObject(const std::string& p_file, 
	                                               const Tags& p_requiredTags = Tags());
	
	void registerPresentationObject(PresentationObject* p_object);
	void unregisterPresentationObject(PresentationObject* p_object);
	void reinsertPresentationObject(PresentationObject* p_object);
	inline bool hasRegisteredPresentationObjects() const { return m_updateObjects.empty() == false; }
	
	s32 particleRenderGroupFromName(const std::string& p_name) const;
	
	inline void setUsedForPrecache(bool p_usedForPrecache) { m_usedForPrecache = p_usedForPrecache; }
	inline bool isUsedForPrecache() const { return m_usedForPrecache; }
	
private:
	struct CustomLayer
	{
		inline CustomLayer(const RenderableInterfacePtr& p_renderable, real p_depth, s32 p_priority)
		:
		renderable(p_renderable),
		depth(p_depth),
		priority(p_priority)
		{
		}
		
		inline bool operator<(const CustomLayer& p_rhs) const
		{
			if (depth == p_rhs.depth)
			{
				return priority < p_rhs.priority;
			}
			return depth < p_rhs.depth;
		}
		
		RenderableInterfaceWeakPtr renderable;
		real depth;
		s32 priority;
	};
	
	typedef std::vector<PresentationObject*> PresentationObjects;
	typedef std::set<GroupInterfacePtr, GroupLess> Groups;
	typedef std::set<CustomLayer> CustomLayers;
	
	typedef std::map<std::string, s32> ParticleLayerGroupName;
	typedef std::vector<RenderableInterfacePtr> ParticleLayerRenderables;
	
	
	PresentationMgr(const Tags& p_acceptedTags, const TriggerFactoryInterfacePtr& p_triggerFactory,
	                u32 p_particleCategory, const GroupFactoryInterfacePtr& p_groupFactory);
	
	/*! \brief For giving information in assert */
	std::string getPossibleParticleLayerNames()const;
	
	/*! \brief group Management */
	void addToGroup(PresentationObject*      p_object);
	void removeFromGroup(PresentationObject* p_object);
	/*! \brief splits a group up if there is one that covers this depth */
	void manageGroupsForLayerAdd(real p_depth);
	
	inline TriggerFactoryInterfacePtr& getTriggerFactory() { return m_triggerFactory; }
	inline PresentationLoader& getPresentationLoader() { return m_loader; }
	
	PresentationObjects m_updateObjects;
	Groups m_presentationGroups;
	CustomLayers m_customLayers;
	
	ParticleLayerGroupName m_particleLayers;
	ParticleLayerRenderables m_particleRenderables;
	
	Tags m_acceptedTags;
	PresentationLoader m_loader;
	TriggerFactoryInterfacePtr m_triggerFactory;
	
	GroupFactoryInterfacePtr m_groupFactory;
	
	bool m_usedForPrecache;
	
	mutable bool m_updateForRenderWasCalled; // Used to panic when render is called without an updateForRender first.
	
	// Disable Copy/assignment
	PresentationMgr(const PresentationMgr&);
	PresentationMgr& operator=(const PresentationMgr&);
	
	friend class PresentationCache;
	friend class PresentationLoader;
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_PRESENTATIONMGR_H)
