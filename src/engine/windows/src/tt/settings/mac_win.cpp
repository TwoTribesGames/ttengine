#include <winsock2.h>
#include <Iphlpapi.h>

#include <tt/mem/util.h>
#include <tt/settings/mac.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace settings {

bool getMacAddress(u8* p_addressOUT)
{
	PIP_ADAPTER_ADDRESSES addresses = 0;
	ULONG size = 0;
	ULONG ret = 0;
	
	// Get required memory for adapter addresses and get adapter address info
	for (int i = 0; i < 5; ++i)
	{
		ret = GetAdaptersAddresses(AF_INET, 0, 0, addresses, &size);
		
		if (ret != ERROR_BUFFER_OVERFLOW)
		{
			break;
		}
		
		if (addresses != 0)
		{
			::operator delete(addresses);
		}
		
		addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(::operator new(size));
		if (addresses == 0)
		{
			ret = GetLastError();
			break;
		}
	}
	
	if (ret == ERROR_SUCCESS)
	{
		for (PIP_ADAPTER_ADDRESSES curr = addresses; curr != 0; curr = curr->Next)
		{
			// Ignore PPP and tunnel (VPN) interfaces
			if (curr->IfType == IF_TYPE_TUNNEL ||
			    curr->IfType == IF_TYPE_PPP)
			{
				continue;
			}
			
			mem::copy8(p_addressOUT, curr->PhysicalAddress, 6);
			if (addresses != 0)
			{
				::operator delete(addresses);
			}
			return true;
		}
	}
	
	switch (ret)
	{
	case ERROR_ADDRESS_NOT_ASSOCIATED: TT_Printf("ERROR_ADDRESS_NOT_ASSOCIATED\n"); break;
	case ERROR_BUFFER_OVERFLOW:        TT_Printf("ERROR_BUFFER_OVERFLOW\n"); break;
	case ERROR_INVALID_PARAMETER:      TT_Printf("ERROR_INVALID_PARAMETER\n"); break;
	case ERROR_NOT_ENOUGH_MEMORY:      TT_Printf("ERROR_NOT_ENOUGH_MEMORY\n"); break;
	case ERROR_NO_DATA:                TT_Printf("ERROR_NO_DATA\n"); break;
	default:                           TT_Printf("Unknown GetAdaptersAddresses return value: %u\n", ret); break;
	}
	
	if (addresses != 0)
	{
		::operator delete(addresses);
	}
	
	return false;
}

// Namespace end
}
}
