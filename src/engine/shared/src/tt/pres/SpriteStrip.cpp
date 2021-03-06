#include <algorithm>

#include <tt/code/bufferutils.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/fs.h>
#include <tt/pres/SpriteStrip.h>
#include <tt/str/format.h>
#include <tt/str/c_str.h>
#include <tt/str/str_types.h>
#include <tt/str/manip.h>



namespace tt {
namespace pres {


static const s32 s_version = 2;

static const fs::size_type g_spriteStripSignatureSize                         = 5;
static const char          g_spriteStripSignature[g_spriteStripSignatureSize] = "SPRT";  // size includes null terminator


bool SpriteStrip::load(const std::string& p_directory, SpriteStripData& p_data,
	                   code::ErrorStatus* p_errStatus, bool p_makeFolder)
{
	TT_ERR_CHAIN(bool, false, "Loading SpriteStrip directory " << p_directory);
	TT_ERR_ASSERT(p_directory.empty() == false);
	
	if (p_directory.find("\\") != std::string::npos)
	{
		TT_ERR_AND_RETURN("frameAnim strip directory '" << p_directory << "' should not contain \\! Use / instead.");
	}
	
	if(*(p_directory.end() - 1) == '/')
	{
		TT_ERR_AND_RETURN("frameAnim strip Directory should not end with /");
	}
	
	p_data.spriteStripNamespace = p_directory;
	str::replace(p_data.spriteStripNamespace, "/", ".");
	
	str::Strings assetStrList = str::explode(p_data.spriteStripNamespace, ".", true);
	assetStrList.pop_back();
	p_data.spriteStripNamespace = str::implode(assetStrList , ".");
	
	p_data.spriteStripNamespace = "textures.autogeneratedspritestrips." + p_data.spriteStripNamespace;
	
	p_data.spriteStripName = p_directory.substr(p_directory.find_last_of("/") + 1);
	
	std::string spriteStripFolder;
	if(p_makeFolder)
	{
		spriteStripFolder = "textures/autogeneratedspritestrips/" + p_directory;
	}
	else
	{
		spriteStripFolder = p_directory;
	}
	
	if(fs::fileExists(spriteStripFolder + ".sprt") == false)
	{
		TT_WARNING("Using directory '%s'. Meta data not created yet this is normal when converting", 
		           p_directory.c_str());
		return false;
	}
	else
	{
		
		fs::FilePtr file = fs::open(spriteStripFolder + ".sprt", fs::OpenMode_Read);
		
		// reading SPRT ID
		char idLoad[g_spriteStripSignatureSize] = { 0 };
		if(fs::read(file, idLoad, g_spriteStripSignatureSize) != g_spriteStripSignatureSize)
		{
			TT_ERR_AND_RETURN("Reading of SPRT ID failed. Meta file to small");
		}
		else if(str::equal(g_spriteStripSignature, idLoad, g_spriteStripSignatureSize) == false)
		{
			TT_ERR_AND_RETURN("Reading of SPRT ID failed. Got '" << idLoad << "' expected '" << g_spriteStripSignature << "'");
		}
		
		// get version
		s32 version = 0;
		if(fs::readInteger(file, &version) == false)
		{
			TT_ERR_AND_RETURN("Reading of version failed.");
		}
		else if (version != s_version)
		{
			// check version
			TT_ERR_AND_RETURN("Invalid version, code "
				<< s_version << ", data " << version <<
				", Please update your converter");
		}
		
		// get framesize
		if(fs::readReal(file, &p_data.frameSize.x) == false)
		{
			TT_ERR_AND_RETURN("Reading of framesize.x failed.");
		}
		
		if(fs::readReal(file, &p_data.frameSize.y) == false)
		{
			TT_ERR_AND_RETURN("Reading of framesize.y failed.");
		}
		
		// get framecount
		if(fs::readInteger<s32>(file, &p_data.totalFrameCount) == false)
		{
			TT_ERR_AND_RETURN("Reading of framecount failed.");
		}
	}
	
	
	
	return true;
}


bool SpriteStrip::save(const std::string& p_outFileMeta, real p_frameHeight, real p_frameWidth, s32 p_frameCount )
{
	TT_MINMAX_ASSERT(p_frameWidth, 0.0f, 1.0f);
	TT_MINMAX_ASSERT(p_frameHeight, 0.0f, 1.0f);

	fs::FilePtr file = fs::open(p_outFileMeta, fs::OpenMode_Write);
	//ID
	if(fs::write(file, g_spriteStripSignature, g_spriteStripSignatureSize) == false)
	{
		fail("Could not create sprite strip meta data ID", p_outFileMeta);
		return false;
	}
	//Version
	if(fs::writeInteger(file, s_version) == false)
	{
		fail("Could not create sprite strip meta data version", p_outFileMeta);
		return false;
	}
	//Width
	if(fs::writeReal(file, p_frameWidth) == false)
	{
		fail("Could not create sprite strip meta data frameheight", p_outFileMeta);
		return false;
	}
	//Height
	if(fs::writeReal(file, p_frameHeight) == false)
	{
		fail("Could not create sprite strip meta data framewidth", p_outFileMeta);
		return false;
	}
	//Count
	if(fs::writeInteger(file, p_frameCount) == false)
	{
		fail("Could not create sprite strip meta data frameCount", p_outFileMeta);
		return false;
	}
	return true;
}


void SpriteStrip::fail( const std::string& p_reason, const std::string& p_outFileMeta )
{
	TT_PANIC("Sprite strip conversion failed for '%s'. Reason: %s",
	         p_outFileMeta.c_str(), p_reason.c_str());
}


//namespace end
}
}
