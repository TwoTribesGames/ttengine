#include <tt/app/ComHelper.h>
#include <tt/audio/player/Xact3Cue.h>
#include <tt/audio/player/Xact3Helpers.h>
#include <tt/audio/player/Xact3Player.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace audio {
namespace player {

//--------------------------------------------------------------------------------------------------
// Public member functions

Xact3Player::Xact3Player(bool p_mustInitCom)
:
SoundPlayer(),
m_engine(0),
m_3DListener(),
m_default3DEmitter(),
m_3DSettings(),
m_soundBanks(),
m_waveBanks(),
m_listenerPosition(math::Vector3::zero),
m_3DAudioEnabled(false),
m_3DInitialized(false),
m_mustReleaseCom(false)
{
	if (p_mustInitCom)
	{
		// Must be called before creating XACT engine
		app::ComHelper::initCom();
		m_mustReleaseCom = true;
	}
}


Xact3Player::~Xact3Player()
{
	if (m_engine != 0)
	{
		uninit();
	}
	
	if (m_3DInitialized)
	{
		delete[] m_3DSettings.pMatrixCoefficients;
	}
	
	if (m_mustReleaseCom)
	{
		app::ComHelper::uninitCom();
	}
}


bool Xact3Player::init(const std::string& p_globalSettings, bool p_debug)
{
	if (m_engine != 0)
	{
		TT_WARN("Xact3Player already initialized!");
		return false;
	}
	
	// Create XACT engine
	DWORD create_flags = 0;
#if defined(TT_BUILD_FINAL)
	(void)p_debug;
#else
	if (p_debug)
	{
		create_flags |= XACT_FLAG_API_DEBUG_MODE;
	}
#endif
	HRESULT hr = XACT3CreateEngine(create_flags, &m_engine);
	
	if (FAILED(hr) || m_engine == 0)
	{
		TT_PANIC("Creating XACT engine failed! Code: 0x%08X\nError: %s (%s)",
		         hr, getXactErrorName(hr), getXactErrorDesc(hr));
		m_engine = 0;
		return false;
	}
	
	// Load global settings
	XACT_RUNTIME_PARAMETERS params = {0};
	params.lookAheadTime = XACT_ENGINE_LOOKAHEAD_DEFAULT;
	
	if (p_globalSettings.empty() == false)
	{
		if (fs::fileExists(p_globalSettings) == false)
		{
			TT_PANIC("Initialize XACT 3 player: global settings file '%s' does not exist.",
			         p_globalSettings.c_str());
			m_engine->Release();
			m_engine = 0;
			return false;
		}
		
		code::BufferPtr buffer = fs::getFileContent(p_globalSettings);
		if (buffer == 0 || buffer->getSize() == 0 || buffer->getData() == 0)
		{
			TT_PANIC("Initialize XACT 3 player: global settings file '%s' could not be opened or is empty.",
			         p_globalSettings.c_str());
			m_engine->Release();
			m_engine = 0;
			return false;
		}
		
		// global settings data needs to be allocated using CoTaskMemAlloc (XMemAlloc for Xbox 360)
		DWORD size = static_cast<DWORD>(buffer->getSize());
		VOID* data = CoTaskMemAlloc(size);
		if (data == 0)
		{
			TT_PANIC("Initialize XACT 3 player: unable to allocate memory for global settings.");
			m_engine->Release();
			m_engine = 0;
			return false;
		}
		mem::copy8(data, buffer->getData(), buffer->getSize());
		
		params.pGlobalSettingsBuffer    = data;
		params.globalSettingsBufferSize = size;
		params.globalSettingsFlags      = XACT_FLAG_GLOBAL_SETTINGS_MANAGEDATA;
	}
	
	// Initialize engine
	hr = m_engine->Initialize(&params);
	if (FAILED(hr))
	{
		TT_PANIC("Initializing XACT engine failed. Code: 0x%08X\nError: %s (%s)",
		         hr, getXactErrorName(hr), getXactErrorDesc(hr));
		m_engine->ShutDown();
		m_engine->Release();
		m_engine = 0;
		return false;
	}
	
	return true;
}


bool Xact3Player::uninit()
{
	if (m_engine == 0)
	{
		TT_WARN("Xact3Player not initialized.");
		return false;
	}
	
	unloadAllSoundBanks();
	unloadAllWaveBanks();
	
	HRESULT hr = m_engine->ShutDown();
	if (FAILED(hr))
	{
		TT_PANIC("Shutdown of XACT engine failed. Code: 0x%08X\nError: %s (%s)",
		         hr, getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	
	m_engine->Release();
	return true;
}


bool Xact3Player::loadSoundBank(const std::string& p_path)
{
	if (m_engine == 0)
	{
		TT_PANIC("Xact3Player not initialized.");
		return false;
	}
	
	if (m_soundBanks.find(p_path) != m_soundBanks.end())
	{
		TT_PANIC("Soundbank '%s' already loaded.", p_path.c_str());
		return false;
	}
	
	if (p_path.empty())
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (fs::fileExists(p_path) == false)
	{
		TT_PANIC("Soundbank file '%s' does not exist.", p_path.c_str());
		return false;
	}
	
	// load sound bank
	SoundBank bank;
	bank.data = fs::getFileContent(p_path);
	if (bank.data == 0 || bank.data->getData() == 0 || bank.data->getSize() == 0)
	{
		TT_PANIC("Soundbank file '%s' could not be opened or is empty.", p_path.c_str());
		return false;
	}
	
	HRESULT hr = m_engine->CreateSoundBank(bank.data->getData(),
	                                       bank.data->getSize(),
	                                       0, 0, &bank.bank);
	
	if (FAILED(hr))
	{
		TT_PANIC("Creating in-memory wave bank failed. Code: 0x%08X\nError: %s (%s)",
		         hr, getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	m_soundBanks.insert(SoundBanks::value_type(tt::fs::utils::getFileTitle(p_path), bank));
	return true;
}


bool Xact3Player::unloadSoundBank(const std::string& p_path)
{
	SoundBanks::iterator it = m_soundBanks.find(p_path);
	if (it == m_soundBanks.end())
	{
		TT_PANIC("Soundbank '%s' not loaded.", p_path.c_str());
		return false;
	}
	
	HRESULT hr = (*it).second.bank->Destroy();
	if (FAILED(hr))
	{
		TT_PANIC("Destroying sound bank '%s' failed. Code: 0x%08X\nError: %s (%s)",
		         (*it).first.c_str(), hr, getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	
	m_soundBanks.erase(it);
	return true;
}


bool Xact3Player::unloadAllSoundBanks()
{
	for (SoundBanks::iterator it = m_soundBanks.begin(); it != m_soundBanks.end(); )
	{
		HRESULT hr = (*it).second.bank->Destroy();
		if (FAILED(hr))
		{
			TT_PANIC("Destroying sound bank '%s' failed. Code: 0x%08X\nError: %s (%s)",
			         (*it).first.c_str(), hr, getXactErrorName(hr), getXactErrorDesc(hr));
			return false;
		}
		it = m_soundBanks.erase(it);
	}
	
	return true;
}


bool Xact3Player::loadWaveBank(const std::string& p_path, bool p_memoryMap)
{
	if (m_engine == 0)
	{
		TT_PANIC("Xact3Player not initialized.");
		return false;
	}
	
	if (m_waveBanks.find(p_path) != m_waveBanks.end())
	{
		TT_PANIC("Wavebank '%s' already loaded.", p_path.c_str());
		return false;
	}
	
	if (p_path.empty())
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (fs::fileExists(p_path) == false)
	{
		TT_WARN("Wavebank file '%s' does not exist.", p_path.c_str());
		return false;
	}
	
	// load wave bank
	WaveBank bank;
	
	if (p_memoryMap)
	{
		bank.fileHandle = CreateFileA(p_path.c_str(), GENERIC_READ,
		                              FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (bank.fileHandle == INVALID_HANDLE_VALUE)
		{
			char msg[256] = { 0 };
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, msg, 256, 0);
			TT_PANIC("Opening wave bank file '%s' failed:\n%s\n", p_path.c_str(), msg);
			return false;
		}
		
		// Get the size of the file
		DWORD size = GetFileSize(bank.fileHandle, 0);
		if (size == INVALID_FILE_SIZE)
		{
			char msg[256] = { 0 };
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, msg, 256, 0);
			TT_PANIC("Retrieving file size of file '%s' failed:\n%s\n", p_path.c_str(), msg);
			
			CloseHandle(bank.fileHandle);
			return false;
		}
		
		// Create a file mapping
		bank.mappingHandle = CreateFileMappingA(bank.fileHandle, 0, PAGE_READONLY, 0, size, 0);
		if (bank.mappingHandle == 0)
		{
			char msg[256] = { 0 };
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, msg, 256, 0);
			TT_PANIC("Creating file mapping of file '%s' failed:\n%s\n", p_path.c_str(), msg);
			
			CloseHandle(bank.fileHandle);
			return false;
		}
		
		// Map a view of the file
		bank.buffer = MapViewOfFile(bank.mappingHandle, FILE_MAP_READ, 0, 0, 0);
		if (bank.buffer == 0)
		{
			char msg[256] = { 0 };
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, msg, 256, 0);
			TT_PANIC("Mapping a view of file '%s' failed:\n%s\n", p_path.c_str(), msg);
			
			CloseHandle(bank.mappingHandle);
			CloseHandle(bank.fileHandle);
			return false;
		}
		
		HRESULT hr = m_engine->CreateInMemoryWaveBank(bank.buffer, size, 0, 0, &bank.bank);
		
		if (FAILED(hr))
		{
			TT_PANIC("Creating memory mapped in-memory wave bank from file '%s' failed. Code: 0x%08X\nError: %s (%s)",
			         p_path.c_str(), hr, getXactErrorName(hr), getXactErrorDesc(hr));
			return false;
		}
	}
	else // if (p_memoryMap)
	{
		XACT_STREAMING_PARAMETERS params = { 0 };
		params.file       = CreateFileA(p_path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		params.packetSize = 2;
		
		HRESULT hr = m_engine->CreateStreamingWaveBank(&params, &bank.bank);
		
		if (FAILED(hr))
		{
			TT_PANIC("Creating streaming wave bank from file '%s' failed. Code: 0x%08X\nError: %s (%s)",
			         p_path.c_str(), hr, getXactErrorName(hr), getXactErrorDesc(hr));
			return false;
		}
	}
	
	m_waveBanks.insert(WaveBanks::value_type(p_path, bank));
	return true;
}


bool Xact3Player::unloadWaveBank(const std::string& p_path)
{
	WaveBanks::iterator it = m_waveBanks.find(p_path);
	if (it == m_waveBanks.end())
	{
		TT_PANIC("Wavebank '%s' not loaded.", p_path.c_str());
		return false;
	}
	
	HRESULT hr = (*it).second.bank->Destroy();
	if (FAILED(hr))
	{
		TT_PANIC("Destroying wavebank '%s' failed. Code: 0x%08X\nError: %s (%s)",
		         (*it).first.c_str(), hr, getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	
	if ((*it).second.buffer != 0)
	{
		UnmapViewOfFile((*it).second.buffer);
		CloseHandle((*it).second.mappingHandle);
		CloseHandle((*it).second.fileHandle);
	}
	
	m_waveBanks.erase(it);
	return true;
}


bool Xact3Player::unloadAllWaveBanks()
{
	for (WaveBanks::iterator it = m_waveBanks.begin(); it != m_waveBanks.end(); )
	{
		HRESULT hr = (*it).second.bank->Destroy();
		if (FAILED(hr))
		{
			TT_PANIC("Destroying wavebank '%s' failed. Code: 0x%08X\nError: %s (%s)",
			         (*it).first.c_str(), hr, getXactErrorName(hr), getXactErrorDesc(hr));
			return false;
		}
		
		if ((*it).second.buffer != 0)
		{
			UnmapViewOfFile((*it).second.buffer);
			CloseHandle((*it).second.mappingHandle);
			CloseHandle((*it).second.fileHandle);
		}
		
		it = m_waveBanks.erase(it);
	}
	
	return true;
}


void Xact3Player::update(real /*p_deltaTime*/)
{
	if (m_engine != 0)
	{
		m_engine->DoWork();
	}
}


SoundCuePtr Xact3Player::createCue(const std::string& p_soundbank, const std::string& p_name, bool p_positional)
{
	IXACT3SoundBank* bank = getBank(p_soundbank);
	if (bank == 0)
	{
		return SoundCuePtr();
	}
	
	XACTINDEX cueIndex = bank->GetCueIndex(p_name.c_str());
	if (cueIndex == XACTINDEX_INVALID)
	{
		TT_PANIC("Cue '%s' not found in soundbank '%s'.", p_name.c_str(), p_soundbank.c_str());
		return SoundCuePtr();
	}
	
	X3DAUDIO_EMITTER cueEmitter(m_default3DEmitter);
	
	return SoundCuePtr(new Xact3Cue(this, bank, cueIndex, p_positional, cueEmitter, p_name));
}


bool Xact3Player::play(const std::string& p_soundbank, const std::string& p_name)
{
	IXACT3SoundBank* bank = getBank(p_soundbank);
	if (bank == 0)
	{
		return false;
	}
	
	const XACTINDEX cueIndex = bank->GetCueIndex(p_name.c_str());
	if (cueIndex == XACTINDEX_INVALID)
	{
		TT_PANIC("Cue '%s' not found in soundbank '%s'.", p_name.c_str(), p_soundbank.c_str());
		return false;
	}
	
	HRESULT hr = bank->Play(cueIndex, 0, 0, 0);
	if (FAILED(hr))
	{
		// Ignore instance limit warnings
		if (hr != XACTENGINE_E_INSTANCELIMITFAILTOPLAY)
		{
			TT_PANIC("Playing XACT cue %u ('%s' from soundbank '%s') failed.\nError: %s (%s)",
			         cueIndex, p_name.c_str(), p_soundbank.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
			return false;
		}
	}
	
	return true;
}


SoundCuePtr Xact3Player::playCue(const std::string& p_soundbank, const std::string& p_name, bool p_positional)
{
	SoundCuePtr cue = createCue(p_soundbank, p_name, p_positional);
	if (cue == 0 || cue->play() == false)
	{
		return SoundCuePtr();
	}
	
	return cue;
}


bool Xact3Player::stop(const std::string& p_soundbank, const std::string& p_name)
{
	IXACT3SoundBank* bank = getBank(p_soundbank);
	if (bank == 0)
	{
		return false;
	}
	
	const XACTINDEX cueIndex = bank->GetCueIndex(p_name.c_str());
	if (cueIndex == XACTINDEX_INVALID)
	{
		TT_PANIC("Cue '%s' not found in soundbank '%s'.", p_name.c_str(), p_soundbank.c_str());
		return false;
	}
	
	HRESULT hr = bank->Stop(cueIndex, 0);
	if (FAILED(hr))
	{
		TT_PANIC("Stopping XACT cue %u ('%s' from soundbank '%s')  failed.\nError: %s (%s)",
		         cueIndex, p_name.c_str(), p_soundbank.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	return true;
}


bool Xact3Player::hasCategory(const std::string& p_category)
{
	if (m_engine == 0)
	{
		return false;
	}
	
	return m_engine->GetCategory(p_category.c_str()) != XACTCATEGORY_INVALID;
}


bool Xact3Player::stopCategory(const std::string& p_category)
{
	if (m_engine == 0)
	{
		return false;
	}
	
	XACTCATEGORY cat = m_engine->GetCategory(p_category.c_str());
	if (cat == XACTCATEGORY_INVALID)
	{
		TT_WARN("Category '%s' is not known to XACT.", p_category.c_str());
		return false;
	}
	
	HRESULT hr = m_engine->Stop(cat, 0);
	if (FAILED(hr))
	{
		TT_PANIC("Stopping XACT category %u ('%s') failed.\nError: %s (%s)",
		         cat, p_category.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	return true;
}


bool Xact3Player::pauseCategory(const std::string& p_category)
{
	if (m_engine == 0)
	{
		return false;
	}
	
	XACTCATEGORY cat = m_engine->GetCategory(p_category.c_str());
	if (cat == XACTCATEGORY_INVALID)
	{
		TT_WARN("Category '%s' is not known to XACT.", p_category.c_str());
		return false;
	}
	
	HRESULT hr = m_engine->Pause(cat, TRUE);
	if (FAILED(hr))
	{
		TT_PANIC("Pausing XACT category %u ('%s') failed.\nError: %s (%s)",
		         cat, p_category.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	return true;
}


bool Xact3Player::resumeCategory(const std::string& p_category)
{
	if (m_engine == 0)
	{
		return false;
	}
	
	XACTCATEGORY cat = m_engine->GetCategory(p_category.c_str());
	if (cat == XACTCATEGORY_INVALID)
	{
		TT_WARN("Category '%s' is not known to XACT.", p_category.c_str());
		return false;
	}
	
	HRESULT hr = m_engine->Pause(cat, FALSE);
	if (FAILED(hr))
	{
		TT_PANIC("Resuming XACT category %u ('%s') failed.\nError: %s (%s)",
		         cat, p_category.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	return true;
}


bool Xact3Player::setCategoryVolume(const std::string& p_category, real p_volume)
{
	if (m_engine == 0)
	{
		return false;
	}
	
	XACTCATEGORY cat = m_engine->GetCategory(p_category.c_str());
	if (cat == XACTCATEGORY_INVALID)
	{
		TT_WARN("Category '%s' is not known to XACT.", p_category.c_str());
		return false;
	}
	
	HRESULT hr = m_engine->SetVolume(cat, static_cast<XACTVOLUME>(p_volume));
	if (FAILED(hr))
	{
		TT_PANIC("Setting volume for XACT category %u ('%s') failed.\nError: %s (%s)",
		         cat, p_category.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	return true;
}


bool Xact3Player::set3DAudioEnabled(bool p_enabled)
{
	if(m_3DAudioEnabled == p_enabled) return true;

	m_3DAudioEnabled = p_enabled;

	if(m_3DAudioEnabled && m_3DInitialized == false)
	{
		if(init3DAudio() == false)
		{
			m_3DAudioEnabled = false;
			return false;
		}
	}

	return true;
}


bool Xact3Player::setListenerPosition(const math::Vector3& p_position)
{
	if (m_3DAudioEnabled == false)
	{
		TT_PANIC("3D Audio is not enabled! Call SoundPlayer::set3DAudioEnabled()");
		return false;
	}
	
	m_listenerPosition = p_position;
	m_3DListener.Position.x =  p_position.x;
	m_3DListener.Position.y =  p_position.y;
	m_3DListener.Position.z = -p_position.z; // Flip because XACT uses left-handed coord system
	return true;
}


bool Xact3Player::setGlobalVariable(const std::string& p_name, real p_value)
{
	const XACTVARIABLEINDEX index = m_engine->GetGlobalVariableIndex(p_name.c_str());
	if (index == XACTVARIABLEINDEX_INVALID)
	{
		TT_WARN("Global variable '%s' does not exist: cannot set value to %f", p_name.c_str(), p_value);
		return false;
	}
	
	return m_engine->SetGlobalVariable(index, static_cast<XACTVARIABLEVALUE>(p_value)) == S_OK;
}


bool Xact3Player::getGlobalVariable(const std::string& p_name, real* p_value_OUT) const
{
	TT_NULL_ASSERT(p_value_OUT);
	if (p_value_OUT == 0)
	{
		return false;
	}
	
	const XACTVARIABLEINDEX index = m_engine->GetGlobalVariableIndex(p_name.c_str());
	if (index == XACTVARIABLEINDEX_INVALID)
	{
		TT_WARN("Global variable '%s' does not exist: cannot get its value.", p_name.c_str());
		return false;
	}
	
	XACTVARIABLEVALUE value = 0.0f;
	if (m_engine->GetGlobalVariable(index, &value) != S_OK)
	{
		return false;
	}
	
	*p_value_OUT = static_cast<real>(value);
	return true;
}


bool Xact3Player::init3DAudio()
{
	if (m_engine == 0) return false;
	
	HRESULT hr = XACT3DInitialize(m_engine, m_3DHandle);
	
	if (FAILED(hr))
	{
		TT_PANIC("Initializing XACT 3D Audio failed.\nError: %s (%s)", getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	
	// FIXME: How do we know the correct value for this?
	UINT32 srcChannelCount = 1;
	
	// Initialize listener structure
	mem::zero8(&m_3DListener, sizeof(X3DAUDIO_LISTENER));
	m_3DListener.OrientFront.z = 1.0f;
	m_3DListener.OrientTop.y   = 1.0f;
	
	// Initialize the default emitter (used as initial values for each positional sound effect)
	mem::zero8(&m_default3DEmitter, sizeof(X3DAUDIO_EMITTER));
	m_default3DEmitter.OrientFront.z       = 1.0f;
	m_default3DEmitter.OrientTop.y         = 1.0f;
	m_default3DEmitter.ChannelCount        = srcChannelCount;
	m_default3DEmitter.ChannelRadius       = 1.0f;
	m_default3DEmitter.InnerRadius         = 10.0f;  // "inner radius" of emitter
	m_default3DEmitter.CurveDistanceScaler = 25.0f;  // "outer radius" of emitter; defines size of audio 'world'
	m_default3DEmitter.DopplerScaler       = 1.0f;
	m_default3DEmitter.pVolumeCurve        = (X3DAUDIO_DISTANCE_CURVE*)&X3DAudioDefault_LinearCurve;
	
	// Initialize DSP settings
	mem::zero8(&m_3DSettings, sizeof(X3DAUDIO_DSP_SETTINGS));
	
	WAVEFORMATEXTENSIBLE format;
	hr = m_engine->GetFinalMixFormat(&format);
	
	if (FAILED(hr))
	{
		TT_PANIC("Get mixed format failed.\nError: %s (%s)", getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	
	m_3DSettings.SrcChannelCount     = srcChannelCount;
	m_3DSettings.DstChannelCount     = format.Format.nChannels;
	m_3DSettings.pMatrixCoefficients = new FLOAT32[m_3DSettings.SrcChannelCount * m_3DSettings.DstChannelCount];
	
	m_3DInitialized = true;
	
	return true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

IXACT3SoundBank* Xact3Player::getBank(const std::string& p_soundbank) const
{
	SoundBanks::const_iterator it = m_soundBanks.find(p_soundbank);
	if (it == m_soundBanks.end())
	{
		TT_PANIC("Soundbank '%s' not found.", p_soundbank.c_str());
		return 0;
	}
	
	IXACT3SoundBank* bank = it->second.bank;
	if (bank == 0)
	{
		TT_PANIC("Invalid soundbank '%s'.", p_soundbank.c_str());
		return 0;
	}
	
	return bank;
}


IXACT3Cue* Xact3Player::prepareCue(IXACT3SoundBank* p_soundbank, XACTINDEX p_cueIndex)
{
	IXACT3Cue* result = 0;
	HRESULT hr = p_soundbank->Prepare(p_cueIndex, 0, 0, &result);
	
	if (FAILED(hr))
	{
		TT_PANIC("Preparing XACT cue %u failed.\nError: %s (%s)",
		         p_cueIndex, getXactErrorName(hr), getXactErrorDesc(hr));
		
		if (result != 0)
		{
			result->Destroy();
		}
		return 0;
	}
	
	return result;
}


bool Xact3Player::setEmitterPosition(IXACT3Cue*           p_cue,
                                     X3DAUDIO_EMITTER*    p_emitter_OUT,
                                     const math::Vector3& p_position)
{
	TT_NULL_ASSERT(p_cue);
	TT_NULL_ASSERT(p_emitter_OUT);
	
	// Set emitter position parameters
	p_emitter_OUT->Position.x =  p_position.x;
	p_emitter_OUT->Position.y =  p_position.y;
	p_emitter_OUT->Position.z = -p_position.z; // Flip because XACT uses left-handed coordinate system
	
	return applyEmitterSettings(p_cue, p_emitter_OUT);
}


bool Xact3Player::setEmitterRadius(IXACT3Cue*        p_cue,
                                   X3DAUDIO_EMITTER* p_emitter_OUT,
                                   real              p_inner,
                                   real              p_outer)
{
	TT_NULL_ASSERT(p_cue);
	TT_NULL_ASSERT(p_emitter_OUT);
	
	// Sanity check the values
	if (math::realLessThan(p_inner, 0.0f))
	{
		TT_PANIC("Invalid cue inner radius: %f. Must be a positive number. "
		         "Value has been corrected to 0.0.", p_inner);
		p_inner = 0.0f;
	}
	
	if (math::realLessThan(p_outer, 0.0f))
	{
		TT_PANIC("Invalid cue outer radius: %f. Must be a positive number. "
		         "Value has been corrected to 0.0.", p_outer);
		p_outer = 0.0f;
	}
	
	if (math::realGreaterThan(p_inner, p_outer))
	{
		TT_PANIC("Cue inner radius (%f) is larger than outer radius (%f). Values have been swapped.",
		         p_inner, p_outer);
		std::swap(p_inner, p_outer);
	}
	
	// Set and apply new emitter radii
	p_emitter_OUT->InnerRadius         = p_inner;
	p_emitter_OUT->CurveDistanceScaler = p_outer;
	
	return applyEmitterSettings(p_cue, p_emitter_OUT);
}


bool Xact3Player::applyEmitterSettings(IXACT3Cue* p_cue, X3DAUDIO_EMITTER* p_emitter)
{
	if (m_3DAudioEnabled == false)
	{
		TT_PANIC("3D Audio is not enabled! Call SoundPlayer::set3DAudioEnabled()");
		return false;
	}
	
	if (m_3DInitialized == false)
	{
		TT_PANIC("3D Audio has not been initialized!");
		return false;
	}
	
	if (p_cue == 0)
	{
		TT_PANIC("Cue pointer cannot be 0.");
		return false;
	}
	
	TT_NULL_ASSERT(p_emitter);
	
	// Compute audio parameters based on current settings
	HRESULT hr = XACT3DCalculate(m_3DHandle, &m_3DListener, p_emitter, &m_3DSettings);
	
	if (FAILED(hr))
	{
		TT_PANIC("XACT3DCalculate() failed.\nError: %s (%s)", getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	
	// Apply calculated settings
	hr = XACT3DApply(&m_3DSettings, p_cue);
	
	if (FAILED(hr))
	{
		TT_PANIC("XACT3DApply() failed.\nError: %s (%s)", getXactErrorName(hr), getXactErrorDesc(hr));
		return false;
	}
	
	return true;
}

// Namespace end
}
}
}
