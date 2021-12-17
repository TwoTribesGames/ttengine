
#include "tt/engine/renderer/GLStateCache.h"

namespace tt {
namespace engine {
namespace renderer {

#define CACHE_ENABLED

GLStateCache::GLStateCache()
{
}

void GLStateCache::setClientState(GLenum state, bool enabled)
{
#if !defined(TT_BUILD_FINAL)
	auto it = m_clientStateCache.find(state);
	if (it != m_clientStateCache.end()) {
		if (it->second._changed) {
			TT_Printf("[GLSTATE] Changing already changed client state value 0x%X : %d", state, enabled);
		}
	}
#endif
	m_clientStateCache[state] = enabled;
#ifndef CACHE_ENABLED
	if (enabled) {
		glEnableClientState(state);
	} else {
		glDisableClientState(state);
	}
#endif
}

void GLStateCache::setState(GLenum state, bool enabled)
{
#if !defined(TT_BUILD_FINAL)
	auto it = m_stateCache.find(state);
	if (it != m_stateCache.end()) {
		if (it->second._changed) {
			TT_Printf("[GLSTATE] Changing already changed state value %x : %d", state, enabled);
		}
	}
#endif
	m_stateCache[state] = enabled;
#ifndef CACHE_ENABLED
	if (enabled) {
		glEnable(state);
	} else {
		glDisable(state);
	}
#endif
}

void GLStateCache::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	m_frontColor = tt::math::Vector4(r, g, b, a);
#ifndef CACHE_ENABLED
	glColor4f(r, g, b, a);
#endif
}

void GLStateCache::setBlendFunc(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	m_blendFunc = BlendFunc(srcRGB, dstRGB, srcAlpha, dstAlpha);
#ifndef CACHE_ENABLED
	glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
#endif
}

void GLStateCache::apply()
{
	for (tStateCache::iterator it = m_stateCache.begin(), end = m_stateCache.end();
			it != end; ++it) {

		tStateCache::mapped_type &changer = it->second;

		if (changer._changed) {
#ifdef CACHE_ENABLED
			changer._new ? glEnable(it->first) : glDisable(it->first);
#endif
			changer.synced();
		}
	}

	for (tStateCache::iterator it = m_clientStateCache.begin(), end = m_clientStateCache.end();
			it != end; ++it) {

		tStateCache::mapped_type &changer = it->second;

		if (changer._changed) {
#ifdef CACHE_ENABLED
			changer._new ? glEnableClientState(it->first) : glDisableClientState(it->first);
#endif
			changer.synced();
		}
	}

	if (m_frontColor._changed) {
#ifdef CACHE_ENABLED
		tt::math::Vector4 &v = m_frontColor._new;
		glColor4f(v.x, v.y, v.z, v.w);
#endif
		m_frontColor.synced();
	}

	if (m_blendFunc._changed) {
#ifdef CACHE_ENABLED
		BlendFunc &v = m_blendFunc._new;
		glBlendFuncSeparate(v.srcRGB, v.dstRGB, v.srcAlpha, v.dstAlpha);
#endif
		m_blendFunc.synced();
	}
}

// Namespace end
}
}
}

