#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/PlayWaveEvent.h>
#include <tt/audio/xact/PlayWaveEventInstance.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/utils.h>
#include <tt/audio/xact/Wave.h>
#include <tt/audio/xact/WaveBank.h>
#include <tt/code/ErrorStatus.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>

#include <algorithm>

//#define PLAY_DEBUG
#ifdef PLAY_DEBUG
	#define Play_Printf TT_Printf
#else
	#define Play_Printf(...)
#endif

//#define PLAY_TRACE
#ifdef PLAY_TRACE
	#define Play_Trace TT_Printf
#else
	#define Play_Trace(...)
#endif

#define PLAY_WARN
#ifdef PLAY_WARN
	#define Play_Warn TT_Printf
#else
	#define Play_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

PlayWaveEvent::PlayWaveEvent()
:
m_timeStamp(0.0f),
m_randomOffset(0.0f),
m_loopCount(0),
m_infinite(false),
m_breakLoop(true),
m_newWaveOnLoop(false),
m_volVariationEnabled(false),
m_volOperator(Operator_Replace),
m_volNewOnLoop(false),
m_volRangeMinInDB(0.0f),
m_volRangeMaxInDB(0.0f),
m_volReplace(true),
m_pitchVariationEnabled(false),
m_pitchOperator(Operator_Replace),
m_pitchNewOnLoop(false),
m_pitchRangeMin(0.0f),
m_pitchRangeMax(0.0f),
m_pitchReplace(true),
m_panVariationEnabled(false),
m_panNewOnLoop(false),
m_panAngle(0),
m_panArc(0),
m_useCenterSpeaker(false),
m_playOrder(Variation_Ordered),
m_playList(),
m_playOrderedList(),
m_totalWeight(0),
m_prevWave(0)
{
}


void PlayWaveEvent::setTimeStamp(real p_time)
{
	if (p_time < 0.0f)
	{
		Play_Warn("PlayWaveEvent::setTimeStamp: time stamp %f is less than 0.0; clamping\n", realToFloat(p_time));
		p_time = 0.0f;
	}
	m_timeStamp = p_time;
}


void PlayWaveEvent::setRandomOffset(real p_offset)
{
	if (p_offset < 0.0f)
	{
		Play_Warn("PlayWaveEvent::setRandomOffset: offset %f is less than 0.0; clamping\n", realToFloat(p_offset));
		p_offset = 0.0f;
	}
	m_randomOffset = p_offset;
}


void PlayWaveEvent::addWave(Wave* p_wave, int p_weight)
{
	if (p_wave == 0)
	{
		Play_Warn("PlayWaveEvent::addWave: wave must not be 0\n");
		return;
	}
	
	if (p_weight > 255)
	{
		Play_Warn("PlayWaveEvent::addWave: weight %d is larger than 255; clamping\n", p_weight);
		p_weight = 255;
	}
	
	if (p_weight < 0)
	{
		Play_Warn("PlayWaveEvent::addWave: weight %d is less than 0; clamping\n", p_weight);
		p_weight = 0;
	}
	
	m_playList.push_back(PlayListEntry(p_wave, p_weight));
	m_totalWeight += p_weight;
}


void PlayWaveEvent::setLoopCount(int p_loopCount)
{
	if (p_loopCount > 254)
	{
		Play_Warn("PlayWaveEvent::setLoopCount: loop count %d larger than 254; clamping\n", p_loopCount);
		p_loopCount = 254;
	}
	if (p_loopCount < 0)
	{
		Play_Warn("PlayWaveEvent::setLoopCount: loop count %d less than 0; clamping\n", p_loopCount);
		p_loopCount = 0;
	}
	m_loopCount = p_loopCount;
}


void PlayWaveEvent::setVolumeRangeMin(real p_minInDB)
{
	if (p_minInDB > 6.0f)
	{
		Play_Warn("PlayWaveEvent::setVolumeRangeMin: volume %f larger than 6.0dB; clamping\n", realToFloat(p_minInDB));
		p_minInDB = 6.0f;
	}
	
	if (p_minInDB < -96.0f)
	{
		Play_Warn("PlayWaveEvent::setVolumeRangeMin: volume %f less than -96.0dB; clamping\n", realToFloat(p_minInDB));
		p_minInDB = -96.0f;
	}
	
	m_volRangeMinInDB = p_minInDB;
}


void PlayWaveEvent::setVolumeRangeMax(real p_maxInDB)
{
	if (p_maxInDB > 6.0f)
	{
		Play_Warn("PlayWaveEvent::setVolumeRangeMax: volume %f larger than 6.0dB; clamping\n", realToFloat(p_maxInDB));
		p_maxInDB = 6.0f;
	}
	
	if (p_maxInDB < -96.0f)
	{
		Play_Warn("PlayWaveEvent::setVolumeRangeMax: volume %f less than -96.0dB; clamping\n", realToFloat(p_maxInDB));
		p_maxInDB = -96.0f;
	}
	
	m_volRangeMaxInDB = p_maxInDB;
}


void PlayWaveEvent::setPitchRangeMin(real p_min)
{
	if (p_min > 12.0f)
	{
		Play_Warn("PlayWaveEvent::setPitchRangeMin: pitch %f larger than 12.0; clamping\n", realToFloat(p_min));
		p_min = 12.0f;
	}
	
	if (p_min < -12.0f)
	{
		Play_Warn("PlayWaveEvent::setPitchRangeMin: pitch %f less than -12.0; clamping\n", realToFloat(p_min));
		p_min = -12.0f;
	}
	
	m_pitchRangeMin = p_min;
}


void PlayWaveEvent::setPitchRangeMax(real p_max)
{
	if (p_max > 12.0f)
	{
		Play_Warn("PlayWaveEvent::setPitchRangeMax: pitch %f larger than 12.0; clamping\n", realToFloat(p_max));
		p_max = 12.0f;
	}
	
	if (p_max < -12.0f)
	{
		Play_Warn("PlayWaveEvent::setPitchRangeMax: pitch %f less than -12.0; clamping\n", realToFloat(p_max));
		p_max = -12.0f;
	}
	
	m_pitchRangeMax = p_max;
}


void PlayWaveEvent::setPanAngle(int p_angle)
{
	if (p_angle > 359)
	{
		Play_Warn("PlayWaveEvent::setPanAngle: angle %d larger than 359; clamping\n", p_angle);
		p_angle = 359;
	}
	
	if (p_angle < 0)
	{
		Play_Warn("PlayWaveEvent::setPanAngle: angle %d less than 0; clamping\n", p_angle);
		p_angle = 0;
	}
	m_panAngle = p_angle;
}


void PlayWaveEvent::setPanArc(int p_arc)
{
	if (p_arc > 360)
	{
		Play_Warn("PlayWaveEvent::setPanArc: angle %d larger than 360; clamping\n", p_arc);
		p_arc = 360;
	}
	
	if (p_arc < 0)
	{
		Play_Warn("PlayWaveEvent::setPanArc: angle %d less than 0; clamping\n", p_arc);
		p_arc = 0;
	}
	m_panArc = p_arc;
}


PlayWaveEvent* PlayWaveEvent::createPlayWaveEvent(const xml::XmlNode* p_node)
{
	TT_ERR_CREATE("createPlayWaveEvent");
	
	if (p_node == 0)
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Play Wave Event")
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: xml node '%s' is not a play wave event node\n", p_node->getName().c_str());
		return 0;
	}
	
	PlayWaveEvent* event = new PlayWaveEvent;
	
	const std::string& loopstr(p_node->getAttribute("Loop Count"));
	if (loopstr.empty())
	{
		event->setLoopInfinite(false);
		event->setLoopCount(0);
	}
	else
	{
		const s32 loopcount = str::parseS32(loopstr, &errStatus);
		if (errStatus.hasError())
		{
			Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid loop count '%s'\n", loopstr.c_str());
		}
		else
		{
			if (loopcount == 255)
			{
				event->setLoopInfinite(true);
			}
			else
			{
				event->setLoopInfinite(false);
				event->setLoopCount(loopcount);
			}
		}
	}
	
	const std::string& breakloopstr(p_node->getAttribute("Break Loop"));
	if (breakloopstr.empty())
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing break loop\n");
	}
	else
	{
		const s32 breakloop = str::parseS32(breakloopstr, &errStatus);
		if (errStatus.hasError())
		{
			Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid break loop value '%s'\n", breakloopstr.c_str());
		}
		else
		{
			if (breakloop == 0)
			{
				event->setBreakLoop(true);
			}
			else if (breakloop == 1)
			{
				event->setBreakLoop(false);
			}
			else
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid break loop '%d'\n", breakloop);
			}
		}
	}
	
	const std::string& usespkrposstr(p_node->getAttribute("Use Speaker Position"));
	if (usespkrposstr.empty())
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing use speaker position\n");
	}
	else
	{
		const s32 usespeakerposition = str::parseS32(usespkrposstr, &errStatus);
		if (errStatus.hasError())
		{
			Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid use speaker position value '%s'\n", usespkrposstr.c_str());
		}
		else
		{
			if (usespeakerposition == 0)
			{
				event->setPanVariationEnabled(false);
			}
			else if (usespeakerposition == 1)
			{
				event->setPanVariationEnabled(true);
			}
			else
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid use speaker position value '%d'\n", usespeakerposition);
			}
		}
	}
	
	const std::string& usecntrspkrstr(p_node->getAttribute("Use Center Speaker"));
	if (usecntrspkrstr.empty())
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing use center speaker\n");
	}
	else
	{
		const s32 usecenterspeaker = str::parseS32(usecntrspkrstr, &errStatus);
		if (errStatus.hasError())
		{
			Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid use center speaker value '%s'\n", usecntrspkrstr.c_str());
		}
		else
		{
			if (usecenterspeaker == 0)
			{
				event->setUseCenterSpeaker(false);
			}
			else if (usecenterspeaker == 1)
			{
				event->setUseCenterSpeaker(true);
			}
			else
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid use center speaker value '%d'\n", usecenterspeaker);
			}
		}
	}
	
	const std::string& newpanonloopstr(p_node->getAttribute("New Speaker Position On Loop"));
	if (newpanonloopstr.empty())
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing new speaker position on loop\n");
	}
	else
	{
		const s32 newpanonloop = str::parseS32(newpanonloopstr, &errStatus);
		if (errStatus.hasError())
		{
			Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid new speaker position on loop value '%s'\n", newpanonloopstr.c_str());
		}
		else
		{
			if (newpanonloop == 0)
			{
				event->setPanNewOnLoop(false);
			}
			else if (newpanonloop == 1)
			{
				event->setPanNewOnLoop(true);
			}
			else
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid new speaker position on loop value '%d'\n", newpanonloop);
			}
		}
	}
	
	const std::string& pananglestr(p_node->getAttribute("Speaker Position Angle"));
	if (pananglestr.empty())
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing speaker position angle loop\n");
	}
	else
	{
		event->setPanAngle((int)(xml::fast_atof(pananglestr.c_str())));
	}
	
	std::string panarcstr = p_node->getAttribute("Speaer Position Arc");
	if (panarcstr.empty())
	{
		// in case MS ever gets around fixing that typo
		// FIXME: In the current DirectX SDK (and XACT tool), this typo has indeed been fixed. Drop support for the misspelled name?
		panarcstr = p_node->getAttribute("Speaker Position Arc");
	}
	
	if (panarcstr.empty())
	{
		Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing speaker position arc loop\n");
	}
	else
	{
		event->setPanArc((int)(xml::fast_atof(panarcstr.c_str())));
	}
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Event Header")
		{
			const std::string& timestampstr(child->getAttribute("Timestamp"));
			
			if (timestampstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: no time stamp specified\n");
			}
			else
			{
				event->setTimeStamp(xml::fast_atof(timestampstr.c_str()) / 1000.0f);
			}
			
			const std::string& randoffstr(child->getAttribute("Random Offset"));
			
			if (randoffstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: no random offset specified\n");
			}
			else
			{
				event->setRandomOffset(xml::fast_atof(randoffstr.c_str()) / 1000.0f);
			}
		}
		else if (child->getName() == "Pitch Variation")
		{
			event->setPitchVariationEnabled(true);
			
			const std::string& operatorstr(child->getAttribute("Operator"));
			
			if (operatorstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: no pitch operator specified\n");
			}
			else
			{
				const s32 op = str::parseS32(operatorstr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid pitch operator specified: '%s'\n", operatorstr.c_str());
				}
				else if (op == 0)
				{
					event->setPitchOperator(Operator_Replace);
				}
				else if (op == 1)
				{
					event->setPitchOperator(Operator_Add);
				}
				else
				{
					Play_Warn("PitchEvent::createPitchEvent: unknown pitch operator specified: '%d'\n", op);
				}
			}
			
			const std::string& newpitchonloopstr(child->getAttribute("New Variation On Loop"));
			if (newpitchonloopstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing new pitch on loop\n");
			}
			else
			{
				const s32 newpitchonloop = str::parseS32(newpitchonloopstr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid new pitch on loop value '%s'\n", newpitchonloopstr.c_str());
				}
				else
				{
					if (newpitchonloop == 0)
					{
						event->setPitchNewOnLoop(false);
					}
					else if (newpitchonloop == 1)
					{
						event->setPitchNewOnLoop(true);
					}
					else
					{
						Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid new pitch on loop value '%d'\n", newpitchonloop);
					}
				}
			}
			
			const std::string& minstr(child->getAttribute("Min"));
			if (minstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing pitch min\n");
			}
			else
			{
				event->setPitchRangeMin(xml::fast_atof(minstr.c_str()) / 100.0f);
			}
			
			const std::string& maxstr(child->getAttribute("Max"));
			if (maxstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing pitch max\n");
			}
			else
			{
				event->setPitchRangeMax(xml::fast_atof(maxstr.c_str()) / 100.0f);
			}
		}
		else if (child->getName() == "Volume Variation")
		{
			event->setVolumeVariationEnabled(true);
			
			// For some obscure reason, operator is stored as "Volume"
			const std::string& operatorstr(child->getAttribute("Volume"));
			
			if (operatorstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: no volume operator specified\n");
			}
			else
			{
				const s32 op = str::parseS32(operatorstr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid volume operator specified: '%s'\n", operatorstr.c_str());
				}
				else if (op == 0)
				{
					event->setVolumeOperator(Operator_Replace);
				}
				else if (op == 1)
				{
					event->setVolumeOperator(Operator_Add);
				}
				else
				{
					Play_Warn("PitchEvent::createPitchEvent: unknown volume operator specified: '%d'\n", op);
				}
			}
			
			const std::string& newvolonloopstr(child->getAttribute("New Variation On Loop"));
			if (newvolonloopstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing new volume on loop\n");
			}
			else
			{
				const s32 newvolonloop = str::parseS32(newvolonloopstr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid new volume on loop value '%s'\n", newvolonloopstr.c_str());
				}
				else
				{
					if (newvolonloop == 0)
					{
						event->setVolumeNewOnLoop(false);
					}
					else if (newvolonloop == 1)
					{
						event->setVolumeNewOnLoop(true);
					}
					else
					{
						Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid new volume on loop value '%d'\n", newvolonloop);
					}
				}
			}
			
			const std::string& minstr(child->getAttribute("Min"));
			if (minstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing volume min\n");
			}
			else
			{
				event->setVolumeRangeMin(xml::fast_atof(minstr.c_str()) / 100.0f);
			}
			
			const std::string& maxstr(child->getAttribute("Max"));
			if (maxstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing volume max\n");
			}
			else
			{
				event->setVolumeRangeMax(xml::fast_atof(maxstr.c_str()) / 100.0f);
			}
		}
		else if (child->getName() == "Wave Entry")
		{
			// get bank and entry index and weight
			const std::string& bankindexstr (child->getAttribute("Bank Index"));
			const std::string& entryindexstr(child->getAttribute("Entry Index"));
			const std::string& weightstr    (child->getAttribute("Weight"));
			
			WaveBank* wavebank = 0;
			Wave*     wave     = 0;
			
			if (bankindexstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing bank index\n");
			}
			else
			{
				const s32 bankindex = str::parseS32(bankindexstr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid wave entry bank index value '%s'\n", bankindexstr.c_str());
				}
				else
				{
					wavebank = AudioTT::getWaveBank(bankindex);
					if (wavebank == 0)
					{
						Play_Warn("PlayWaveEvent::createPlayWaveEvent: error retreiving wavebank %d\n", bankindex);
					}
				}
			}
			
			if (entryindexstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing wave entry index\n");
			}
			else
			{
				const s32 entryindex = str::parseS32(entryindexstr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid wave entry bank index value '%s'\n", bankindexstr.c_str());
				}
				else if (wavebank != 0)
				{
					wave = wavebank->getWave(entryindex);
					if (wave == 0)
					{
						Play_Warn("PlayWaveEvent::createPlayWaveEvent: error retreiving wave %d from bank '%s'\n", entryindex, wavebank->getName().c_str());
						Play_Warn("PlayWaveEvent::createPlayWaveEvent: bank '%s' entry '%s'\n", child->getAttribute("Bank Name").c_str(), child->getAttribute("Entry Name").c_str());
					}
				}
			}
			
			if (weightstr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: missing weight\n");
			}
			else
			{
				const s32 weight = str::parseS32(weightstr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid weight '%s'\n", weightstr.c_str());
				}
				else if (wave != 0)
				{
					event->addWave(wave, weight);
				}
			}
		}
		else if (child->getName() == "Variation")
		{
			const std::string& vartypestr(child->getAttribute("Variation Type"));
			
			if (vartypestr.empty())
			{
				Play_Warn("PlayWaveEvent::createPlayWaveEvent: no variation type specified\n");
			}
			else
			{
				const s32 type = str::parseS32(vartypestr, &errStatus);
				if (errStatus.hasError())
				{
					Play_Warn("PlayWaveEvent::createPlayWaveEvent: invalid variation type specified: '%s'\n", vartypestr.c_str());
				}
				else
				{
					switch (type)
					{
					case 0: event->setVariation(Variation_Ordered);           break;
					case 1: event->setVariation(Variation_OrderedFromRandom); break;
					case 2: event->setVariation(Variation_Random);            break;
					case 3: event->setVariation(Variation_RandomNoRepeat);    break;
					case 4: event->setVariation(Variation_Shuffle);           break;
					default:
						Play_Warn("PlayWaveEvent::createPlayWaveEvent: unknown variation type %d\n", type); break;
					}
				}
			}
		}
		else
		{
			Play_Warn("PlayWaveEvent::createPlayWaveEvent: encountered unknown child node '%s'\n", child->getName().c_str());
		}
	}
	
	return event;
}


void PlayWaveEvent::orderPlayList()
{
	Play_Trace("PlayWaveEvent::orderPlayList [%p]\n", this);
	
	m_playOrderedList.clear();
	
	int maxweight = 255;
	
	while (m_playOrderedList.size() != m_playList.size())
	{
		Play_Printf("PlayWaveEventInstance::orderPlayList: %d/%d items ordered\n", m_playOrder.size(), m_playList.size());
		int secondbest = 0;
		
		for (PlayList::iterator it = m_playList.begin(); it != m_playList.end(); ++it)
		{
			if ((*it).second == maxweight)
			{
				m_playOrderedList.push_back((*it).first);
			}
			else if ((*it).second > secondbest && (*it).second < maxweight)
			{
				secondbest = (*it).second;
			}
		}
		maxweight = secondbest;
	}
	Play_Printf("PlayWaveEventInstance::orderPlayList: %d/%d items ordered, done\n", m_playOrderedList.size(), m_playList.size());
}


void PlayWaveEvent::shufflePlayList()
{
	Play_Trace("PlayWaveEventInstance::shufflePlayList [%p]\n", this);
	
	m_playOrderedList.clear();
	
	if (m_playList.empty())
	{
		return;
	}
	
	while (m_playOrderedList.size() != m_playList.size())
	{
		u32 index = getXactRandom().getNext(static_cast<u32>(m_playList.size()));
		Wave* snd = m_playList[index].first;
		
		if (std::find(m_playOrderedList.begin(), m_playOrderedList.end(), snd) == m_playOrderedList.end())
		{
			m_playOrderedList.push_back(snd);
		}
	}
}


Wave* PlayWaveEvent::getNextWave()
{
	Play_Trace("PlayWaveEventInstance::getNextWave [%p]\n", this);
	
	switch (getVariation())
	{
	case PlayWaveEvent::Variation_Ordered:
	case PlayWaveEvent::Variation_OrderedFromRandom:
		{
			if (m_playOrderedList.empty())
			{
				m_prevWave = 0;
				return m_prevWave;
			}
			
			// get next item in playlist
			// loop through playlist
			OrderedList::iterator it = std::find(m_playOrderedList.begin(), m_playOrderedList.end(), m_prevWave);
			
			if (it == m_playOrderedList.end())
			{
				m_prevWave = *(m_playOrderedList.begin());
				return m_prevWave;
			}
			
			++it;
			
			if (it == m_playOrderedList.end())
			{
				m_prevWave = *(m_playOrderedList.begin());
				return m_prevWave;
			}
			
			m_prevWave = *it;
			return m_prevWave;
		}
		
	case PlayWaveEvent::Variation_Random:
		{
			if (m_playList.empty())
			{
				m_prevWave = 0;
				return m_prevWave;
			}
			
			// get random item
			int value = static_cast<int>(getXactRandom().getNext(static_cast<u32>(m_totalWeight)));
			int weight = 0;
			for (PlayList::iterator it = m_playList.begin(); it != m_playList.end(); ++it)
			{
				if ((value >= weight) && (value < (weight + (*it).second)))
				{
					m_prevWave = (*it).first;
					return m_prevWave;
				}
				weight += (*it).second;
			}
			m_prevWave = 0;
			return m_prevWave;
		}
		
	case PlayWaveEvent::Variation_RandomNoRepeat:
		{
			if (m_playList.empty())
			{
				m_prevWave = 0;
				return m_prevWave;
			}
			
			// get random item
			if (m_playList.size() == 1)
			{
				m_prevWave = (*m_playList.begin()).first;
				return m_prevWave;
			}
			
			Wave* ret = m_prevWave;
			while (ret == m_prevWave)
			{
				int value = static_cast<int>(getXactRandom().getNext(static_cast<u32>(m_totalWeight)));
				int weight = 0;
				for (PlayList::iterator it = m_playList.begin(); it != m_playList.end(); ++it)
				{
					if ((value >= weight) && (value < (weight + (*it).second)))
					{
						ret = (*it).first;
					}
					weight += (*it).second;
				}
			}
			
			m_prevWave = ret;
			return m_prevWave;
		}
		
	case PlayWaveEvent::Variation_Shuffle:
		{
			if (m_playOrderedList.empty())
			{
				m_prevWave = 0;
				return m_prevWave;
			}
			
			// get next item in playlist
			// if there are no more items
			// shuffle the playlist and get first item
			
			OrderedList::iterator it = std::find(m_playOrderedList.begin(), m_playOrderedList.end(), m_prevWave);
			
			if (it == m_playOrderedList.end())
			{
				shufflePlayList();
				m_prevWave = *(m_playOrderedList.begin());
				return m_prevWave;
			}
			
			++it;
			
			if (it == m_playOrderedList.end())
			{
				shufflePlayList();
				m_prevWave = *(m_playOrderedList.begin());
				return m_prevWave;
			}
			
			m_prevWave = *it;
			return m_prevWave;
		}
		
	default:
		break;
	}
	
	m_prevWave = 0;
	return m_prevWave;
}


real PlayWaveEvent::getStartDelay() const
{
	Play_Trace("PlayWaveEvent::getTimeStamp [%p]\n", this);
	
	real timestamp = m_timeStamp * 1000.0f;
	Play_Printf("PlayWaveEvent::getTimeStamp: timestamp %f\n", realToFloat(timestamp) / 1000.0f);
	
	real offset = m_randomOffset * 1000.0f;
	Play_Printf("PlayWaveEvent::getTimeStamp: offset %f\n", realToFloat(offset) / 1000.0f);
	
	if (offset != 0)
	{
		real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(offset)));
		Play_Printf("PlayWaveEvent::getTimeStamp: rnd %f\n", realToFloat(rnd) / 1000.0f);
		
		timestamp += rnd;
	}
	
	Play_Printf("PlayWaveEvent::getTimeStamp: result %f\n", realToFloat(timestamp) / 1000.0f);
	
	return timestamp / 1000.0f;
}


PlayWaveEventInstance* PlayWaveEvent::instantiate(TrackInstance* p_track)
{
	PlayWaveEventInstance* instance = new PlayWaveEventInstance(this, p_track);
	
	return instance;
}


bool PlayWaveEvent::load(const fs::FilePtr& p_file)
{
	int size;
	
	// read playlist
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read wavebank index (should be >= 0)
		int waveBankIndex;
		fs::readInteger(p_file, &waveBankIndex);
		TT_ASSERT(waveBankIndex >= 0);
		
		// read wave index (should be >= 0)
		int waveIndex;
		fs::readInteger(p_file, &waveIndex);
		TT_ASSERT(waveIndex >= 0);
		
		// read weight
		int weight;
		fs::readInteger(p_file, &weight);
		
		// now fetch the correct wave from the wavebank
		WaveBank* bank = AudioTT::getWaveBank(waveBankIndex);
		TT_NULL_ASSERT(bank);
		
		Wave* wave = bank->getWave(waveIndex);
		TT_NULL_ASSERT(wave);
		
		addWave(wave, weight);
	}
	
	// read members
	fs::readEnum<u8>(p_file, &m_playOrder);
	
	fs::readReal(p_file, &m_timeStamp);
	fs::readReal(p_file, &m_randomOffset);
	
	fs::readInteger(p_file, &m_loopCount);
	fs::readBool(p_file, &m_infinite);
	fs::readBool(p_file, &m_breakLoop);
	fs::readBool(p_file, &m_newWaveOnLoop);
	
	fs::readBool(p_file, &m_volVariationEnabled);
	fs::readEnum<u8>(p_file, &m_volOperator);
	fs::readBool(p_file, &m_volNewOnLoop);
	fs::readReal(p_file, &m_volRangeMinInDB);
	fs::readReal(p_file, &m_volRangeMaxInDB);
	fs::readBool(p_file, &m_volReplace);
	
	fs::readBool(p_file, &m_pitchVariationEnabled);
	fs::readEnum<u8>(p_file, &m_pitchOperator);
	fs::readBool(p_file, &m_pitchNewOnLoop);
	fs::readReal(p_file, &m_pitchRangeMin);
	fs::readReal(p_file, &m_pitchRangeMax);
	fs::readBool(p_file, &m_pitchReplace);
	
	fs::readBool(p_file, &m_panVariationEnabled);
	fs::readBool(p_file, &m_panNewOnLoop);
	fs::readInteger(p_file, &m_panAngle);
	fs::readInteger(p_file, &m_panArc);
	fs::readBool(p_file, &m_useCenterSpeaker);
	
	initPlayList();
	
	return true;
}


bool PlayWaveEvent::save(const fs::FilePtr& p_file) const
{
	// save playlist
	fs::writeInteger(p_file, m_playList.size());
	for (PlayList::const_iterator it = m_playList.begin(); it != m_playList.end(); ++it)
	{
		// save wavebank index (should be >= 0)
		int waveBankIndex = (*it).first->getWaveBankIndex();
		TT_ASSERT(waveBankIndex >= 0);
		fs::writeInteger(p_file, waveBankIndex);
		
		// save waveindex (should be >= 0)
		int waveIndex = (*it).first->getWaveIndex();
		TT_ASSERT(waveIndex >= 0);
		fs::writeInteger(p_file, waveIndex);
		
		// save weight
		fs::writeInteger(p_file, (*it).second);
	}
	
	// write members
	fs::writeEnum<u8>(p_file, m_playOrder);
	
	fs::writeReal(p_file, m_timeStamp);
	fs::writeReal(p_file, m_randomOffset);
	
	fs::writeInteger(p_file, m_loopCount);
	fs::writeBool(p_file, m_infinite);
	fs::writeBool(p_file, m_breakLoop);
	fs::writeBool(p_file, m_newWaveOnLoop);
	
	fs::writeBool(p_file, m_volVariationEnabled);
	fs::writeEnum<u8>(p_file, m_volOperator);
	fs::writeBool(p_file, m_volNewOnLoop);
	fs::writeReal(p_file, m_volRangeMinInDB);
	fs::writeReal(p_file, m_volRangeMaxInDB);
	fs::writeBool(p_file, m_volReplace);
	
	fs::writeBool(p_file, m_pitchVariationEnabled);
	fs::writeEnum<u8>(p_file, m_pitchOperator);
	fs::writeBool(p_file, m_pitchNewOnLoop);
	fs::writeReal(p_file, m_pitchRangeMin);
	fs::writeReal(p_file, m_pitchRangeMax);
	fs::writeBool(p_file, m_pitchReplace);
	
	fs::writeBool(p_file, m_panVariationEnabled);
	fs::writeBool(p_file, m_panNewOnLoop);
	fs::writeInteger(p_file, m_panAngle);
	fs::writeInteger(p_file, m_panArc);
	fs::writeBool(p_file, m_useCenterSpeaker);
	
	return true;
}


void PlayWaveEvent::initPlayList()
{
	// prepare playlist
	switch (getVariation())
	{
	case PlayWaveEvent::Variation_Ordered:
		orderPlayList();
		break;
		
	case PlayWaveEvent::Variation_OrderedFromRandom:
		orderPlayList();
		m_prevWave = m_playOrderedList[getXactRandom().getNext(static_cast<u32>(m_playList.size()))];
		break;
		
	case PlayWaveEvent::Variation_Random:
	case PlayWaveEvent::Variation_RandomNoRepeat:
		shufflePlayList();
		m_prevWave = m_playOrderedList[getXactRandom().getNext(static_cast<u32>(m_playList.size()))];
		break;
		
	case PlayWaveEvent::Variation_Shuffle:
		shufflePlayList();
		break;
		
	default:
		break;
	}
	
	Play_Printf("PlayWaveEventInstance::play: done\n");
}


// Namespace end
}
}
}
