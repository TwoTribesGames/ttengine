class Colors
{
	SpawnSection          = [255, 155, 225, 30];
	Area                  = [ 20, 155, 225,  64];
	ButterflyArea         = [225, 155, 128,  64];
	Trigger               = [225, 125,  20,  64];
	ForceField            = [125, 255, 125,  64];
	ZeroGravityField      = [  0, 200, 240,  90];
	Checkpoint            = [  0, 125, 255, 128];
	Lava                  = [225, 140,   0, 130];
	TriggerKill           = [225,   0,   0,  96];
	TriggerHealthDecrease = [225, 100,   0,  96];
	Train                 = [255, 255, 255,  64];
	AudioTrigger          = [180,  30, 180,  64];
	Exit                  = [255, 255, 255,  96];
	Darkness              = [  0,   0,   0,  64];
	CrushBox              = [ 62,  74,  83, 200];
	Effect                = [ 20, 125, 255,  64];
	GUI                   = [200, 200, 220,  64];
	PipeGate              = [20,  179, 149,  96];
	BatFlock              = [120, 179, 149,  96];
	CollisionSize         = [125, 255, 125,  64];
	Light                 = [255, 255,   0,  32];
	Sound                 = [255, 100, 100,  64];
}

class ReferenceColors
{
	Waypoint              = [255, 180,  48, 255];
	Area                  = [ 64,  64, 255, 255];
	List                  = [ 64, 180, 180, 255];
	Enable                = [  0, 180,   0, 255];
	Disable               = [255,   0,   0, 255];
	Sensing               = [255, 255,   0, 255];
	Parent                = [255, 255, 255, 255];
	Kill                  = [255, 128, 128, 255];
	Stick                 = [255, 128, 255, 255];
}

class TextColors
{
	White  = ::ColorRGBA(255, 255, 255, 255);
	Light  = ::ColorRGBA( 43, 225, 162, 255);
	Dark   = ::ColorRGBA( 43, 225, 162, 155);
	Black  = ::ColorRGBA(  0,   0,   0, 255);
	Button = ::ColorRGBA(255, 255, 255, 255);
}

function ColorRGB::_tostring()
{
	return "[" + r + ", " + b + ", " + g + "]";
}
