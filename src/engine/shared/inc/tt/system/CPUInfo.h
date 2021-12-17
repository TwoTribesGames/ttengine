#if !defined(INC_TT_SYSTEM_CPU_H)
#define INC_TT_SYSTEM_CPU_H

// Check if this platform supports CPUInfo.
#define TT_NO_CPUINFO 0

#include <tt/code/BitMask.h>

namespace tt {
namespace system {

enum CpuFeature
{
	CpuFeature_FPU,
	CpuFeature_VME,
	CpuFeature_DE,
	CpuFeature_PSE_1,
	CpuFeature_TSC,
	CpuFeature_MSR,
	CpuFeature_PAE,
	CpuFeature_MCE,
	CpuFeature_CX8,
	CpuFeature_APIC,
	CpuFeature_RSV_1,
	CpuFeature_SEP,
	CpuFeature_MTRR,
	CpuFeature_PGE,
	CpuFeature_MCA,
	CpuFeature_CMOV,
	CpuFeature_PAT,
	CpuFeature_PSE_2,
	CpuFeature_PSN,
	CpuFeature_CLFSH,
	CpuFeature_RSV_2,
	CpuFeature_DS,
	CpuFeature_ACPI,
	CpuFeature_MMX,
	CpuFeature_FXSR,
	CpuFeature_SSE,
	CpuFeature_SSE2,
	CpuFeature_SS,
	CpuFeature_HTT,
	CpuFeature_TM,
	CpuFeature_RSV_3,
	CpuFeature_PBE,
	
	CpuFeature_Count
};


inline bool isValidCpuFeature(CpuFeature p_feature)
{
	return p_feature >= 0 && p_feature < CpuFeature_Count;
}

const char* const getCpuFeatureName(CpuFeature p_feature);

// ------------------------------------------------------------------------------------------------
// CPUInfo

struct CPUInfo
{
public:
	typedef tt::code::BitMask<CpuFeature, CpuFeature_Count> CpuFeatures;
	
	char idString[0x20]; // Identification String 
	s32 steppingID;
	s32 model;
	s32 family;
	s32 processorType;
	s32 extendedModel;
	s32 extendedFamily;
	s32 brandIndex;
	s32 cacheLineSizeCLFLUSH;
	s32 physicalIdAPIC;
	bool hasSSE3;
	bool monitorMwait;
	bool cplQualifiedDebugStore;
	bool thermalMonitor2;
	bool hasSSSE3;
	bool l1ContextId;
	bool hasFMA3;
	bool hasSSE41;
	bool hasSSE42;
	bool hasAVX;
	CpuFeatures cpuFeatures;
	
	// Extended CPUID information
	char brandString[0x40]; // Processor Brand String
	s32 cacheLineSize;
	s32 l2Associativity;
	s32 cacheSizeIn1KUnits;
	bool hasx64;
	bool hasSSE4a;
	bool hasFMA4;
	bool hasXOP;
	
	CPUInfo();
};


CPUInfo getCPUInfo();


// Namespace end
}
}

#endif  // !defined(INC_TT_SYSTEM_CPU_H)
