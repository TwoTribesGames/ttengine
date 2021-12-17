#include <string.h>

#include <tt/system/CPUInfo.h>

#if TT_NO_CPUINFO
// No CPUInfo for this platform
#else

#if defined(TT_PLATFORM_WIN)
#include <intrin.h>

//  Windows
#define cpuid    __cpuid

#elif defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX)

//  GCC Inline Assembly
void cpuid(int CPUInfo[4],int InfoType){
    __asm__ __volatile__ (
        "cpuid":
        "=a" (CPUInfo[0]),
        "=b" (CPUInfo[1]),
        "=c" (CPUInfo[2]),
        "=d" (CPUInfo[3]) :
        "a" (InfoType)
    );
}

#else

#error no cpuid for this platform!

#endif


namespace tt {
namespace system {


const char* const getCpuFeatureName(CpuFeature p_feature)
{
	switch (p_feature)
	{
	case CpuFeature_FPU:   return "x87 FPU on Chip";
	case CpuFeature_VME:   return "Virtual-8086 Mode Enhancement";
	case CpuFeature_DE:    return "Debugging Extensions";
	case CpuFeature_PSE_1: return "Page Size Extensions";
	case CpuFeature_TSC:   return "Time Stamp Counter";
	case CpuFeature_MSR:   return "RDMSR and WRMSR Support";
	case CpuFeature_PAE:   return "Physical Address Extensions";
	case CpuFeature_MCE:   return "Machine Check Exception";
	case CpuFeature_CX8:   return "CMPXCHG8B Inst.";
	case CpuFeature_APIC:  return "APIC on Chip";
	case CpuFeature_RSV_1: return "Reserved";
	case CpuFeature_SEP:   return "SYSENTER and SYSEXIT";
	case CpuFeature_MTRR:  return "Memory Type Range Registers";
	case CpuFeature_PGE:   return "PTE Global Bit";
	case CpuFeature_MCA:   return "Machine Check Architecture";
	case CpuFeature_CMOV:  return "Conditional Move/Compare Instruction";
	case CpuFeature_PAT:   return "Page Attribute Table";
	case CpuFeature_PSE_2: return "Page Size Extension";
	case CpuFeature_PSN:   return "Processor Serial Number";
	case CpuFeature_CLFSH: return "CFLUSH Instruction";
	case CpuFeature_RSV_2: return "Reserved";
	case CpuFeature_DS:    return "Debug Store";
	case CpuFeature_ACPI:  return "Thermal Monitor and Clock Ctrl";
	case CpuFeature_MMX:   return "MMX Technology";
	case CpuFeature_FXSR:  return "FXSAVE/FXRSTOR";
	case CpuFeature_SSE:   return "SSE Extensions";
	case CpuFeature_SSE2:  return "SSE2 Extensions";
	case CpuFeature_SS:    return "Self Snoop";
	case CpuFeature_HTT:   return "Hyper-threading technology";
	case CpuFeature_TM:    return "Thermal Monitor";
	case CpuFeature_RSV_3: return "Reserved";
	case CpuFeature_PBE:   return "Pend. Brk. En.";
	default:
		TT_PANIC("Unknown CPU feature: %d", p_feature);
		return "";
	}
}


CPUInfo::CPUInfo()
:
steppingID(0),
model(0),
family(0),
processorType(0),
extendedModel(0),
extendedFamily(0),
brandIndex(0),
cacheLineSizeCLFLUSH(0),
physicalIdAPIC(0),
hasSSE3(false),
monitorMwait(false),
cplQualifiedDebugStore(false),
thermalMonitor2(false),
hasSSSE3(false),
l1ContextId(false),
hasFMA3(false),
hasSSE41(false),
hasSSE42(false),
hasAVX(false),
cpuFeatures(),
cacheLineSize(0),
l2Associativity(0),
cacheSizeIn1KUnits(0),
hasx64(false),
hasSSE4a(false),
hasFMA4(false),
hasXOP(false)
{
	memset(idString   , 0, sizeof(idString   ));
	memset(brandString, 0, sizeof(brandString));
}


CPUInfo getCPUInfo()
{
	CPUInfo info;
	
	int CPUInfoRaw[4] = {0};
	
	// Get info for argument 0
	cpuid(CPUInfoRaw, 0);
	const int idCount = CPUInfoRaw[0];
	
	TT_STATIC_ASSERT(sizeof(info.idString) >= sizeof(int) * 3);
	memset(info.idString, 0, sizeof(info.idString));
	*((int*)(info.idString    )) = CPUInfoRaw[1];
	*((int*)(info.idString + 4)) = CPUInfoRaw[3];
	*((int*)(info.idString + 8)) = CPUInfoRaw[2];
	
	for (int id = 1; id <= idCount; ++id)
	{
		cpuid(CPUInfoRaw, id);
		
		// Get info for argument 1
		if (id == 1)
		{
			info.steppingID             =  (CPUInfoRaw[0] >>  0) & 0xF;
			info.model                  =  (CPUInfoRaw[0] >>  4) & 0xF;
			info.family                 =  (CPUInfoRaw[0] >>  8) & 0xF;
			info.processorType          =  (CPUInfoRaw[0] >> 12) & 0x3;
			info.extendedModel          =  (CPUInfoRaw[0] >> 16) & 0xF;
			info.extendedFamily         =  (CPUInfoRaw[0] >> 20) & 0xFF;
			info.brandIndex             =  (CPUInfoRaw[1] >>  0) & 0xFF;
			info.cacheLineSizeCLFLUSH   = ((CPUInfoRaw[1] >>  8) & 0xF ) * 8;
			info.physicalIdAPIC         =  (CPUInfoRaw[1] >> 24) & 0xFF;
			info.hasSSE3                = ((CPUInfoRaw[2] >>  0) & 0x1 ) != 0;
			info.monitorMwait           = ((CPUInfoRaw[2] >>  3) & 0x1 ) != 0;
			info.cplQualifiedDebugStore = ((CPUInfoRaw[2] >>  4) & 0x1 ) != 0;
			info.thermalMonitor2        = ((CPUInfoRaw[2] >>  8) & 0x1 ) != 0;
			info.hasSSSE3               = ((CPUInfoRaw[2] >>  9) & 0x1 ) != 0;
			info.l1ContextId            = ((CPUInfoRaw[2] >> 10) & 0x1 ) != 0;
			info.hasFMA3                = ((CPUInfoRaw[2] >> 12) & 0x1 ) != 0;
			info.hasSSE41               = ((CPUInfoRaw[2] >> 19) & 0x1 ) != 0;
			info.hasSSE42               = ((CPUInfoRaw[2] >> 20) & 0x1 ) != 0;
			info.hasAVX                 = ((CPUInfoRaw[2] >> 28) & 0x1 ) != 0;
			
			info.cpuFeatures  = CPUInfo::CpuFeatures(CPUInfoRaw[3]);
		}
	}
	
	// Extended information
	// Get info for argument 0x80000000
	cpuid(CPUInfoRaw, 0x80000000);
	const u32 extendedIdCount = static_cast<u32>(CPUInfoRaw[0]);
	
	TT_STATIC_ASSERT(sizeof(info.brandString) >= sizeof(CPUInfoRaw) * 3);
	memset(info.brandString, 0, sizeof(info.brandString));
	
	for (u32 id = 0x80000001; id <= extendedIdCount; ++id)
	{
		cpuid(CPUInfoRaw, id);
		
		if (id == 0x80000001)
		{
			info.hasSSE4a = ((CPUInfoRaw[2] >>  6) & 0x1 ) != 0;
			info.hasXOP   = ((CPUInfoRaw[2] >> 11) & 0x1 ) != 0;
			info.hasFMA4  = ((CPUInfoRaw[2] >> 16) & 0x1 ) != 0;
			info.hasx64   = ((CPUInfoRaw[3] >> 29) & 0x1 ) != 0;
		}
		else if (id == 0x80000002)
		{
			memcpy(info.brandString     , CPUInfoRaw, sizeof(CPUInfoRaw));
		}
		else if (id == 0x80000003)
		{
			memcpy(info.brandString + 16, CPUInfoRaw, sizeof(CPUInfoRaw));
		}
		else if (id == 0x80000004)
		{
			memcpy(info.brandString + 32, CPUInfoRaw, sizeof(CPUInfoRaw));
		}
		else if (id == 0x80000006)
		{
			info.cacheLineSize      = (CPUInfoRaw[2] >>  0) & 0xFF;
			info.l2Associativity    = (CPUInfoRaw[2] >> 12) & 0xF;
			info.cacheSizeIn1KUnits = (CPUInfoRaw[2] >> 16) & 0xFFFF;
		}
	}
	
	return info;
}


// Namespace end
}
}


#endif // #else TT_NO_CPUINFO
