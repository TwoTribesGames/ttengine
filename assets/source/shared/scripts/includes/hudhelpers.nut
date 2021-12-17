enum HudAlignment
{
	Left        = 0,
	CenterLeft  = 1,
	Center      = 2,
	CenterRight = 3,
	Right       = 4,
	Count       = 5
};

enum HudLayer
{
	Permanent            = 0x0,
	Normal               = 0x1,
	VirusUploadMode      = 0x2,
	Death                = 0x4,
	Menu                 = 0x8,
	Ife                  = 0x10,
	
	All                  = 0xFF
};
