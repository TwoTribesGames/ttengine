//#include <tt/engine/scene/SceneObjectCache.h>
//#include <tt/engine/scene/Instance.h>
//#include <tt/engine/file/FileUtils.h>
//#include <tt/platform/tt_printf.h>
//
//
//namespace tt {
//namespace engine {
//namespace scene {
//
//SceneObjectCache::SceneObjectCache()
//{
//}
//
//SceneObjectCache::~SceneObjectCache()
//{
//}
//
//void SceneObjectCache::add(const std::string& p_objectname, 
//	                       const std::string& p_context)
//{
//	TT_Printf("[PRECACHING] %s (context %s)\n", 
//		p_objectname.c_str(), p_context.c_str());
//	
//	InstancePtr instance = Instance::load(p_objectname, p_context);
//	m_cache.push_back(instance);
//}
//
//std::size_t SceneObjectCache::size()
//{
//	return m_cache.size();
//}
//
//void SceneObjectCache::clear()
//{
//	m_cache.clear();
//}
//
//// Namespace end
//}
//}
//}
