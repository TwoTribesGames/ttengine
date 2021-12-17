#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#include <ifaddrs.h>

#if !defined(IFT_ETHER)
#define IFT_ETHER 0x06 /* Ethernet CSMACD */
#endif

//#include <tt/mem/util.h>
#include <tt/settings/mac.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


//#define TT_MAC_DEBUG

#if defined(TT_MAC_DEBUG)
#define MAC_Printf(...) TT_Printf(__VA_ARGS__)
#else
#define MAC_Printf(...) 
#endif


namespace tt {
namespace settings {

bool getMacAddress(u8* p_addressOUT)
{
	if (p_addressOUT == 0)
	{
		TT_PANIC("Invalid address pointer passed.");
		return false;
	}
	
	ifaddrs* addrs = 0;
	bool success = getifaddrs(&addrs) == 0;
	if (success == false)
	{
		return false;
	}
	
	for (const ifaddrs* cursor = addrs; cursor != 0; cursor = cursor->ifa_next)
	{
		MAC_Printf("getMacAddress: Interface '%s':\n", cursor->ifa_name);
		if ((cursor->ifa_addr->sa_family == AF_LINK) &&
		    (((const sockaddr_dl*)cursor->ifa_addr)->sdl_type == IFT_ETHER))
		{
			const sockaddr_dl* dlAddr = (const sockaddr_dl*)cursor->ifa_addr;
			MAC_Printf("getMacAddress: - sdl_nlen = %d\n", dlAddr->sdl_nlen);
			MAC_Printf("getMacAddress: - sdl_alen = %d\n", dlAddr->sdl_alen);
			
			TT_ASSERTMSG(dlAddr->sdl_alen == 6, "Expected MAC address to be 6 bytes, but it is %d bytes.",
			             dlAddr->sdl_alen);
			MAC_Printf("getMacAddress: - MAC: ");
			const uint8_t* base = (const uint8_t*)&dlAddr->sdl_data[dlAddr->sdl_nlen];
			for (int i = 0; i < dlAddr->sdl_alen; ++i)
			{
				if (i != 0)
				{
					MAC_Printf(":");
				}
				MAC_Printf("%02x", base[i]);
				if (i < 6)
				{
					p_addressOUT[i] = base[i];
				}
			}
			MAC_Printf("\n");
			break; // stop after the first match (crude, but hey)
		}
		else
		{
			MAC_Printf("getMacAddress: -- not of the desired type (AF_LINK and IFT_ETHER)\n");
		}
	}
	
	freeifaddrs(addrs);
	
	return true;
}

// Namespace end
}
}
