#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/Wave.h>
#include <tt/audio/xact/WaveBank.h>
#include <tt/code/helpers.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/compression/fastlz.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/xml/XmlNode.h>


//#define WAVE_DEBUG
#ifdef WAVE_DEBUG
	#define Wave_Printf TT_Printf
#else
	#define Wave_Printf(...)
#endif


#define WAVE_WARN
#ifdef WAVE_WARN
	#define Wave_Warn TT_Printf
#else
	#define Wave_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

// NOTE: For the reason for these specific signature bytes, see
// http://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header
// (these bytes are inspired by the PNG file header bytes,
// for the same reasons as listed there: mainly to detect file transmission problems)
const size_t g_waveBankSignatureLength = 9;
const u8     g_waveBankSignature[g_waveBankSignatureLength] =
{
	0x89,        // Has the high bit set to detect transmission systems that do not support 8 bit data
	'T', 'T', 'W', 'B', // The actual signature bytes for a particle trigger
	0x0D, 0x0A,  // A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
	0x1A,        // A byte that stops display of the file under DOS when the command type has been used—the end-of-file character
	0x0A         // A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
};
const u32    g_waveBankCurrentVersion = 3;


WaveBank::WaveBank(int p_waveBankIndex, const std::string& p_name)
:
m_name(p_name),
m_waves(),
m_waveBankIndex(p_waveBankIndex)
{
}


WaveBank::~WaveBank()
{
	code::helpers::freePointerContainer(m_waves);
}


void WaveBank::addWave(Wave* p_wave)
{
	if (p_wave == 0)
	{
		Wave_Warn("WaveBank::addWave: wave must not be 0\n");
		return;
	}
	
	int waveindex = static_cast<int>(m_waves.size());
	m_waves.push_back(p_wave);
	
	// set the wavebankindex/waveindex in the wave itself as well, used for loading and saving
	p_wave->setWaveIndex(waveindex);
	
	TT_ASSERT(m_waveBankIndex >= 0);
	p_wave->setWaveBankIndex(m_waveBankIndex);
}


Wave* WaveBank::getWave(int p_index) const
{
	if (p_index < 0 || p_index >= static_cast<int>(m_waves.size()))
	{
		Wave_Warn("WaveBank::getWave: wave %d not found\n", p_index);
		return 0;
	}
	
	return m_waves[p_index];
}


bool WaveBank::loadWaves()
{
	const std::string filename(AudioTT::getRoot() + m_name + ".ttwb");
	tt::fs::FilePtr file = tt::fs::open(filename, tt::fs::OpenMode_Read);
	if (file == 0)
	{
		TT_PANIC("Failed to open file '%s'", filename.c_str());
		return false;
	}
	
	// Load whole file into memory
	tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(file->getLength()));
	if (file->read(buffer->getData(), file->getLength()) != file->getLength())
	{
		TT_PANIC("Failed to read from file '%s'", filename.c_str());
		return false;
	}
	
	// Safe to close file now
	file.reset();
	
	tt::code::BufferReadContext context(buffer->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Signature
	u8 signature[g_waveBankSignatureLength] = { 0 };
	bu::get(signature, g_waveBankSignatureLength, &context);
	if (std::memcmp(signature, g_waveBankSignature, g_waveBankSignatureLength) != 0)
	{
		TT_PANIC("Wave bank file '%s' doesn't have valid signature.", filename.c_str());
		return false;
	}
	
	// Version
	u32 dataVersion = bu::get<u32>(&context);
	if (dataVersion != g_waveBankCurrentVersion)
	{
		TT_PANIC("Wave bank file '%s' file format is not the expected version. "
		         "Loaded version %u, expected version %u.", m_name.c_str(),
		         dataVersion, g_waveBankCurrentVersion);
		return false;
	}
	
	// Compressed or not?
	bool compressed = bu::get<bool>(&context);
	tt::code::BufferPtrForCreator uncompressedBuffer;
	if (compressed)
	{
		const u32 compressedSize = bu::get<u32>(&context);
		const u32 totalSize      = bu::get<u32>(&context);
		uncompressedBuffer.reset(new tt::code::Buffer(totalSize));
		if (fastlz_decompress(context.cursor, compressedSize, uncompressedBuffer->getData(), totalSize) !=
			static_cast<s32>(totalSize))
		{
			TT_PANIC("Cannot decompress wavebank '%s'.", filename.c_str());
			return false;
		}
		context = uncompressedBuffer->getReadContext();
	}
	
	u32 numberOfWaves = bu::get<u32>(&context);
	for (u32 i = 0; i < numberOfWaves; ++i)
	{
		Wave* wave = Wave::createWave(&context);
		if (wave == 0)
		{
			Wave_Warn("WaveBank::loadWaves: error creating wave with index %d\n", i);
		}
		else
		{
			addWave(wave);
		}
	}
	
	return true;
}


void WaveBank::unloadWaves()
{
	tt::code::helpers::freePointerContainer(m_waves);
	m_waves.clear();
}


s32 WaveBank::getMemorySize() const
{
	s32 totalSize = 0;
	for (Waves::const_iterator it = m_waves.begin(); it != m_waves.end(); ++it)
	{
		totalSize += (*it)->getMemorySize();
	}
	return totalSize;
}


WaveBank* WaveBank::createWaveBank(int p_waveBankIndex, const xml::XmlNode* p_node, bool p_fromConverter)
{
	if (p_node == 0)
	{
		Wave_Warn("WaveBank::createWaveBank: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Wave Bank")
	{
		Wave_Warn("WaveBank::createWaveBank: xml node '%s' is not a wave bank node\n", p_node->getName().c_str());
		return 0;
	}
	
	std::string waveBankName(p_node->getAttribute("Name"));
	
	if (waveBankName.empty())
	{
		Wave_Warn("WaveBank::createWaveBank: xml node '%s' doesn't have a wave bank name\n",p_node->getName().c_str());
		return 0;
	}
	
	WaveBank* bank = new WaveBank(p_waveBankIndex, waveBankName);
	
	if (p_fromConverter == false)
	{
		return bank;
	}
	
	// Called from converter, add 'dummy' waves to this bank
	int count = 0;
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Wave")
		{
			count++;
			Wave* wave = new Wave();
			if (wave == 0)
			{
				Wave_Warn("WaveBank::createWaveBank: error creating Wave\n");
			}
			else
			{
				bank->addWave(wave);
			}
		}
		else
		{
			Wave_Warn("WaveBank::createWaveBank: encountered unknown child node '%s'\n", child->getName().c_str());
		}
	}
	
	return bank;
}


bool WaveBank::load(const fs::FilePtr& p_file)
{
	fs::readNarrowString(p_file, &m_name);
	
	return true;
}


bool WaveBank::save(const fs::FilePtr& p_file) const
{
	fs::writeNarrowString(p_file, m_name);
	
	return true;
}

// Namespace end
}
}
}
