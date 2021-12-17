#if !defined(INC_TOKI_SERIALIZATION_SERIALIZATIONMGR_H)
#define INC_TOKI_SERIALIZATION_SERIALIZATIONMGR_H


#include <string>

#include <tt/code/Buffer.h>

#include <toki/serialization/fwd.h>
#include <toki/serialization/Serializer.h>


namespace toki {
namespace serialization {

enum Section
{
	Section_Game,
	Section_Squirrel,
	Section_EntityMgr,
	Section_TimerMgr,
	Section_Registry,
	Section_FluidMgr,
	Section_LightMgr,
	Section_DarknessMgr,
	Section_SensorMgr,
	Section_TileSensorMgr,
	Section_MovementControllerMgr,
	Section_PowerBeamGraphicMgr,
	Section_PresentationObjectMgr,
	Section_PresentationObjectMgrState,
	Section_GunMgr,
	Section_EffectRectMgr,
	Section_TextLabelMgr,
	Section_DemoMgr,
	
	Section_Count,
	Section_Invalid
};

inline bool isValid(Section p_section) { return p_section >= 0 && p_section < Section_Count; }
u32     getSectionSaveID(Section p_section);
Section getSectionFromSaveID(u32 p_saveID);


class SerializationMgr
{
public:
	static SerializationMgrPtr createEmpty();
	static SerializationMgrPtr createFromFile(const tt::fs::FilePtr& p_file);
	static SerializationMgrPtr createFromCompressedBuffer(const tt::code::BufferPtr& p_buffer);
	SerializationMgr();
	
	void clearAll();
	
	const SerializerPtr& getSection(Section p_section) const;
	
	u32 getTotalSize() const;
	
	/*! \brief Saves the serialization data to file. */
	bool saveToFile(tt::fs::FilePtr& p_file) const;
	
	/*! \brief Loads serialization data from file. */
	bool loadFromFile(const tt::fs::FilePtr& p_file);
	
	SerializationMgrPtr clone() const;
	
	tt::code::BufferPtr compress() const;
	
private:
	SerializationMgr(const SerializationMgr& p_rhs);
	
	bool loadFromCompressedBuffer(const tt::code::BufferPtr& p_buffer);
	
	static const u32 ms_compressionBufferSize = 5 * 1024 * 1024;
	static u8 ms_compressionInputBuffer [ms_compressionBufferSize];
	static u8 ms_compressionOutputBuffer[ms_compressionBufferSize];
	
	SerializerPtr m_sections[Section_Count];
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_SERIALIZATION_SERIALIZATIONMGR_H)
