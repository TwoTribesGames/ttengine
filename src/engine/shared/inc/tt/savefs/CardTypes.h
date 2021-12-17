#ifndef INC_SAVEFS_CARDTYPES_H
#define INC_SAVEFS_CARDTYPES_H

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace savefs {

enum CardType
{
	CardType_EEPROM_4Kbit,
	CardType_EEPROM_64Kbit,
	CardType_EEPROM_521Kbit,
	CardType_FLASH_2Mbit,
	CardType_FLASH_4Mbit,
	CardType_FLASH_8Mbit,
	CardType_FLASH_16Mbit,
	CardType_FLASH_64Mbit,
	CardType_FRAM_256Kbit,
	
	CardType_None
};


inline u32 getCardSize(CardType p_type)
{
	switch (p_type)
	{
	case CardType_EEPROM_4Kbit:   return     512;
	case CardType_EEPROM_64Kbit:  return    8192;
	case CardType_EEPROM_521Kbit: return   65536;
	case CardType_FLASH_2Mbit:    return  262144;
	case CardType_FLASH_4Mbit:    return  524288;
	case CardType_FLASH_8Mbit:    return 1048576;
	case CardType_FLASH_16Mbit:   return 2097152;
	case CardType_FLASH_64Mbit:   return 8388608;
	case CardType_FRAM_256Kbit:   return   32768;
	case CardType_None:           return       0;
	default:
		TT_PANIC("Unknown card type %d.", p_type);
		return 0;
	}
}


}
}


#endif // INC_SAVEFS_CARDTYPES_H
