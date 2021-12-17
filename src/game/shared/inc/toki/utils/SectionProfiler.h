#if !defined(INC_TOKI_UTILS_SECTIONPROFILER_H)
#define INC_TOKI_UTILS_SECTIONPROFILER_H

#include <limits>

#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/platform/tt_types.h>
#include <tt/str/toStr.h>
#include <tt/system/Time.h>


#if !defined(TT_BUILD_FINAL)
#define TT_USE_SECTION_PROFILER 1
#else
#define TT_USE_SECTION_PROFILER 0 // Turn off (0) for final builds.
#endif

namespace toki {
namespace utils {

// ------------------------------------------------------------------------------------------------
// Game::update

enum FrameUpdateSection
{
	FrameUpdateSection_ScriptMgr,
	FrameUpdateSection_Misc,
	FrameUpdateSection_EditorToggle,
	FrameUpdateSection_EntityMgr,
	FrameUpdateSection_EntityScriptMgr,
	FrameUpdateSection_FluidMgr,
	FrameUpdateSection_LightMgr,
	FrameUpdateSection_PathMgr,
	FrameUpdateSection_Shoebox,
	FrameUpdateSection_Editor,
	//FrameUpdateSection_EventMgr,
	//FrameUpdateSection_SoundGraphicsMgr,
	FrameUpdateSection_PresentationMgr,
	FrameUpdateSection_ParticleMgr,
	
	FrameUpdateSection_Count,
	FrameUpdateSection_Invalid
};

inline bool isValid(FrameUpdateSection p_section)
{
	return p_section >= 0 && p_section < FrameUpdateSection_Count;
}

inline const char* const getName(FrameUpdateSection p_section)
{
	switch (p_section)
	{
	case FrameUpdateSection_ScriptMgr:          return "ScriptMgr";
	case FrameUpdateSection_Misc:               return "Misc";
	case FrameUpdateSection_EditorToggle:       return "EditorToggle";
	case FrameUpdateSection_EntityMgr:          return "EntityMgr";
	case FrameUpdateSection_EntityScriptMgr:    return "EntityScriptMgr";
	case FrameUpdateSection_FluidMgr:           return "FluidMgr";
	case FrameUpdateSection_LightMgr:           return "LightMgr";
	case FrameUpdateSection_PathMgr:            return "PathMgr";
	case FrameUpdateSection_Shoebox:            return "Shoebox";
	case FrameUpdateSection_Editor:             return "Editor";
	//case FrameUpdateSection_EventMgr:           return "EventMgr";
	//case FrameUpdateSection_SoundGraphicsMgr:   return "SoundGraphicsMgr";
	case FrameUpdateSection_PresentationMgr:    return "PresentationMgr";
	case FrameUpdateSection_ParticleMgr:        return "ParticleMgr";
	default:
		TT_PANIC("Unknown FrameUpdateSection: %d", p_section);
		return "";
	}
}

// ------------------------------------------------------------------------------------------------
// Game::updateForRender


enum FrameUpdateForRenderSection
{
	FrameUpdateForRenderSection_PresentationMgr,
	FrameUpdateForRenderSection_SoundGraphicsMgr,
	FrameUpdateForRenderSection_ParticleMgr,
	FrameUpdateForRenderSection_FluidMgr,
	FrameUpdateForRenderSection_LightMgr,
	FrameUpdateForRenderSection_Shoebox,
	FrameUpdateForRenderSection_Fog,
	
	FrameUpdateForRenderSection_Count,
	FrameUpdateForRenderSection_Invalid
};

inline bool isValid(FrameUpdateForRenderSection p_section)
{
	return p_section >= 0 && p_section < FrameUpdateForRenderSection_Count;
}

inline const char* const getName(FrameUpdateForRenderSection p_section)
{
	switch (p_section)
	{
	case FrameUpdateForRenderSection_PresentationMgr:    return "PresentationMgr";
	case FrameUpdateForRenderSection_SoundGraphicsMgr:   return "SoundGraphicsMgr";
	case FrameUpdateForRenderSection_ParticleMgr:        return "ParticleMgr";
	case FrameUpdateForRenderSection_FluidMgr:           return "FluidMgr";
	case FrameUpdateForRenderSection_LightMgr:           return "LightMgr";
	case FrameUpdateForRenderSection_Shoebox:            return "Shoebox";
	case FrameUpdateForRenderSection_Fog:                return "Fog";
		
	default:
		TT_PANIC("Unknown FrameUpdateForRenderSection: %d", p_section);
		return "";
	}
}


// ------------------------------------------------------------------------------------------------
// Game::render

enum FrameRenderSection
{
	FrameRenderSection_Misc,
	FrameRenderSection_Shoebox,
	FrameRenderSection_Particles,
	FrameRenderSection_Presentation,
	FrameRenderSection_SoundGraphics,
	FrameRenderSection_AttributeDebugView,
	FrameRenderSection_LightMgr,
	FrameRenderSection_FluidMgr,
	FrameRenderSection_EntityMgr,
	
	FrameRenderSection_Count,
	FrameRenderSection_Invalid
};

inline bool isValid(FrameRenderSection p_section)
{
	return p_section >= 0 && p_section < FrameRenderSection_Count;
}

inline const char* const getName(FrameRenderSection p_section)
{
	switch (p_section)
	{
	case FrameRenderSection_Misc:               return "Misc";
	case FrameRenderSection_Shoebox:            return "Shoebox";
	case FrameRenderSection_Particles:          return "Particles";
	case FrameRenderSection_Presentation:       return "Presentation";
	case FrameRenderSection_SoundGraphics:      return "SoundGraphics";
	case FrameRenderSection_AttributeDebugView: return "AttributeDebugView";
	case FrameRenderSection_LightMgr:           return "LightMgr";
	case FrameRenderSection_FluidMgr:           return "FluidMgr";
	case FrameRenderSection_EntityMgr:          return "EntityMgr";
	default:
		TT_PANIC("Unknown FrameRenderSection: %d", p_section);
		return "";
	}
}


// ------------------------------------------------------------------------------------------------
// LightMgr::update

enum LightMgrSection
{
	LightMgrSection_Sensors,
	LightMgrSection_StaticOccluders,
	LightMgrSection_DynamicOccluders,
	LightMgrSection_MergeOccluders,
	
	LightMgrSection_Count,
	LightMgrSection_Invalid
};


inline bool isValid(LightMgrSection p_section)
{
	return p_section >= 0 && p_section < LightMgrSection_Count;
}


inline const char* const getName(LightMgrSection p_section)
{
	switch (p_section)
	{
	case LightMgrSection_Sensors:          return "Sensors";
	case LightMgrSection_StaticOccluders:  return "StaticOccluders";
	case LightMgrSection_DynamicOccluders: return "DynamicOccluders";
	case LightMgrSection_MergeOccluders:   return "MergeOccluders";
	default:
		TT_PANIC("Unknown LightMgrSection: %d", p_section);
		return "";
	}
}


// ------------------------------------------------------------------------------------------------
// FluidMgr::update



enum FluidMgrSection
{
	FluidMgrSection_CheckChanges,
	FluidMgrSection_Simulate,
	FluidMgrSection_Grow,
	FluidMgrSection_FluidGraphicsMgr,
	FluidMgrSection_FluidParticlesMgr,
	FluidMgrSection_NotifyTileChange,
	
	FluidMgrSection_Count,
	FluidMgrSection_Invalid
};


inline bool isValid(FluidMgrSection p_section)
{
	return p_section >= 0 && p_section < FluidMgrSection_Count;
}


inline const char* const getName(FluidMgrSection p_section)
{
	switch (p_section)
	{
	case FluidMgrSection_CheckChanges:      return "CheckChanges";
	case FluidMgrSection_Simulate:          return "Simulate";
	case FluidMgrSection_Grow:              return "Grow";
	case FluidMgrSection_FluidGraphicsMgr:  return "Gfx";
	case FluidMgrSection_FluidParticlesMgr: return "ParticleMgr";
	case FluidMgrSection_NotifyTileChange:  return "NotifyTileChange";
	default:
		TT_PANIC("Unknown FluidMgrSection: %d", p_section);
		return "";
	}
}


// ------------------------------------------------------------------------------------------------
// EntityMgr::update


enum EntityMgrSection
{
	EntityMgrSection_MovementControllers,
	EntityMgrSection_MovementControllersChanges,
	EntityMgrSection_TileRegistrationMgr,
	EntityMgrSection_Sensors,
	EntityMgrSection_TileSensors,
	EntityMgrSection_PowerBeamGraphicMgr,
	EntityMgrSection_TextLabelMgr,
	EntityMgrSection_EffectRectMgr,
	EntityMgrSection_DeathRow,
	EntityMgrSection_Culling,
	EntityMgrSection_ScreenEnterExit,
	
	EntityMgrSection_Count,
	EntityMgrSection_Invalid
};


inline bool isValid(EntityMgrSection p_section)
{
	return p_section >= 0 && p_section < EntityMgrSection_Count;
}


inline const char* const getName(EntityMgrSection p_section)
{
	switch (p_section)
	{
	case EntityMgrSection_MovementControllers:        return "MovementControllers";
	case EntityMgrSection_MovementControllersChanges: return "MovementControllersChanges";
	case EntityMgrSection_TileRegistrationMgr:        return "TileRegistrationMgr";
	case EntityMgrSection_Sensors:                    return "Sensors";
	case EntityMgrSection_TileSensors:                return "TileSensors";
	case EntityMgrSection_PowerBeamGraphicMgr:        return "PowerBeamGraphicMgr";
	case EntityMgrSection_TextLabelMgr:               return "TextLabelMgr";
	case EntityMgrSection_EffectRectMgr:              return "EffectRectMgr";
	case EntityMgrSection_DeathRow:                   return "DeathRow";
	case EntityMgrSection_Culling:                    return "Culling";
	case EntityMgrSection_ScreenEnterExit:            return "Screen Enter/Exit";
	
	default:
		TT_PANIC("Unknown EntityMgrSection: %d", p_section);
		return "";
	}
}


// ------------------------------------------------------------------------------------------------
// SectionProfiler

static const s32 SectionProfiler_renderWidth       = 450;
static const s32 SectionProfiler_renderBorderSize  = 5;

template <typename Type, Type typeCount>
class SectionProfiler
{
public:
	SectionProfiler(const std::string& p_name)
#if TT_USE_SECTION_PROFILER != 0
	:
	m_sampleIndex(0),
	m_currentSection(static_cast<Type>(0)),
	m_sectionStartTime(0),
	m_outsideTime(0),
	m_totalInside(0),
	m_name(p_name)
	{
		for (s32 i = 0; i < typeCount; ++i)
		{
			for (s32 j = 0; j < Constants_SampleCount; ++j)
			{
				m_sectionTimings[i][j] = 0;
			}
		}
	}
#else
	{(void) p_name;}
#endif
	
	inline void startFrameUpdate()
	{
#if TT_USE_SECTION_PROFILER != 0
		m_currentSection   = static_cast<Type>(-1);
		u64 now            = tt::system::Time::getInstance()->getMicroSeconds();
		m_outsideTime      = now - m_sectionStartTime;
		m_sectionStartTime = now;
		++m_sampleIndex;
		if (m_sampleIndex >= Constants_SampleCount)
		{
			m_sampleIndex = 0;
		}
		
		for (s32 i = 0; i < typeCount; ++i)
		{
			m_sectionTimings[i][m_sampleIndex] = 0;
		}
#endif
	}
	
	inline void startFrameUpdateSection(Type p_section)
	{
#if TT_USE_SECTION_PROFILER == 0
		(void)p_section;
#else
		if (m_currentSection != p_section)
		{
			u64 now = tt::system::Time::getInstance()->getMicroSeconds();
			if (isValid(m_currentSection))
			{
				m_sectionTimings[m_currentSection][m_sampleIndex] += now - m_sectionStartTime;
			}
			m_currentSection   = p_section;
			m_sectionStartTime = now;
		}
#endif
	}
	
	inline void stopFrameUpdate()
	{
#if TT_USE_SECTION_PROFILER != 0
		startFrameUpdateSection(static_cast<Type>(-1));
		m_totalInside = 0;
		for (s32 i = 0; i < typeCount; ++i)
		{
			m_totalInside += m_sectionTimings[i][m_sampleIndex];
		}
#endif
	}
	
	s32 render(s32 p_x, s32 p_y, bool p_renderOutsideTime) const
	{
		s32 yPos = p_y;
#if TT_USE_SECTION_PROFILER == 0
		(void) p_x;
		(void) p_renderOutsideTime;
#else
		const u64 maxFrameTime = (1000000/60);
		
		tt::engine::debug::DebugRendererPtr debug(
				tt::engine::renderer::Renderer::getInstance()->getDebug());
		
		// Render quad behind text for easier reading.
		{
			const s32 height = (typeCount * 20) + ((p_renderOutsideTime) ? 80 : 40);
			const s32 width  = SectionProfiler_renderWidth;
			const s32 extra  = SectionProfiler_renderBorderSize;
			
			debug->renderSolidRect(tt::engine::renderer::ColorRGBA(0, 0, 0, 128), 
			                       tt::math::PointRect(tt::math::Point2(p_x - extra, p_y - extra),
			                                           width + (extra * 2), height + (extra * 2)),
			                       true);
			debug->flush();
		}
		
		// Profiler Title / Name
		debug->renderText(m_name.c_str(), p_x, yPos, tt::engine::renderer::ColorRGB::green);
		yPos += 20;
		
		// Sections
		for (s32 i = 0; i < typeCount; ++i)
		{
			Type section = static_cast<Type>(i);
			TT_ASSERT(isValid(section));
			std::string percentTotalStr(getPercentStr(m_sectionTimings[i][m_sampleIndex], m_totalInside));
			std::string percentFrameStr(getPercentStr(m_sectionTimings[i][m_sampleIndex], maxFrameTime));
			
			u64 min = std::numeric_limits<u64>::max();
			u64 max = 0;
			
			for (s32 j = 0; j < Constants_SampleCount; ++j)
			{
				min = std::min(min, m_sectionTimings[i][j]);
				max = std::max(max, m_sectionTimings[i][j]);
			}
			
			const std::string minStr = getSizedStr(tt::str::toStr(min), 3, "  ");
			const std::string maxStr = getSizedStr(tt::str::toStr(max), 3, "  ");
			
			debug->renderText(
					std::string(percentFrameStr + "% (" + percentTotalStr + "%) " + 
					"(min: " + minStr + " (" + getPercentStr(min, maxFrameTime) + "%)" + 
					 " max: " + maxStr + " (" + getPercentStr(max, maxFrameTime) + "%)) " +
					getName(section)) + ": "+ tt::str::toStr(m_sectionTimings[i][m_sampleIndex]),
					p_x, yPos, tt::engine::renderer::ColorRGB::green);
			yPos += 20;
		}
		
		debug->renderText("Total inside time: "+ tt::str::toStr(m_totalInside) + 
		                  " (" + getPercentStr(m_totalInside, maxFrameTime) + "%)",
		                  p_x, yPos, tt::engine::renderer::ColorRGB::green);
		yPos += 20;
		
		if (p_renderOutsideTime)
		{
			debug->renderText(std::string("Outside Time: ") + tt::str::toStr(m_outsideTime) +
			                  " (" + getPercentStr(m_outsideTime, maxFrameTime) + "%)",
			                  p_x, yPos, tt::engine::renderer::ColorRGB::green);
			yPos += 20;
			debug->renderText(std::string("Total Time: ") + tt::str::toStr(m_outsideTime + m_totalInside) +
			                  " (" + getPercentStr(m_outsideTime + m_totalInside, maxFrameTime) + "%)",
			                  p_x, yPos, tt::engine::renderer::ColorRGB::green);
			yPos += 20;
		}
#endif
		return yPos;
	}
	
private:
	static inline std::string getPercentStr(u64 p_part, u64 p_total)
	{
		const u64 percent = (p_total == 0) ? 0 : ((p_part * 100) / p_total);
		std::string str = tt::str::toStr(percent);
		return getSizedStr(str, 2, "0");
	}
	static inline std::string getSizedStr(const std::string& p_str, u32 p_length, const std::string& p_filler)
	{
		if (p_str.length() >= p_length)
		{
			return p_str;
		}
		
		const std::string::size_type addCount = p_length - p_str.length();
		std::string fillerStr;
		fillerStr.reserve(addCount * p_filler.length());
		for (std::string::size_type i = 0; i < addCount; ++i)
		{
			fillerStr += p_filler;
		}
		return p_filler + p_str;
	}
	
	SectionProfiler(const SectionProfiler&);                  // Disable copy
	const SectionProfiler& operator=(const SectionProfiler&); // Disable assigment.
	
#if TT_USE_SECTION_PROFILER != 0
	enum Constants
	{
		Constants_SampleCount = 60
	};
	s32  m_sampleIndex;
	u64  m_sectionTimings[typeCount][Constants_SampleCount];
	Type m_currentSection;
	u64  m_sectionStartTime;
	u64  m_outsideTime;
	u64  m_totalInside;
	const std::string m_name;
#endif
};


typedef SectionProfiler<utils::FluidMgrSection, utils::FluidMgrSection_Count> FluidMgrSectionProfiler;


// Namespace end
}
}

#endif // !defined(INC_TOKI_UTILS_SECTIONPROFILER_H)
