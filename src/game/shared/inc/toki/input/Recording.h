#if !defined(INC_TOKI_INPUT_RECORDING_H)
#define INC_TOKI_INPUT_RECORDING_H

#include <vector>

#include <tt/app/Platform.h>
#include <tt/fs/types.h>
#include <tt/input/ControllerType.h>
#include <tt/platform/tt_types.h>

#include <toki/constants.h>
#include <toki/game/script/Registry.h>
#include <toki/game/fwd.h>
#include <toki/input/Controller.h>
#include <toki/input/fwd.h>
#include <toki/level/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace input {


struct InputState
{
	tt::input::ControllerType controllerType;
	tt::input::Pointer        pointer;
	tt::input::Button         accept;
	tt::input::Button         cancel;
	tt::input::Button         faceUp;
	tt::input::Button         faceLeft;
	tt::input::Button         left;
	tt::input::Button         right;
	tt::input::Button         up;
	tt::input::Button         down;
	tt::input::Button         virusUpload;
	tt::input::Button         jump;
	tt::input::Button         primaryFire;
	tt::input::Button         secondaryFire;
	tt::input::Button         selectWeapon1;
	tt::input::Button         selectWeapon2;
	tt::input::Button         selectWeapon3;
	tt::input::Button         selectWeapon4;
	tt::input::Button         toggleWeapons;
	tt::input::Button         demoReset;
	tt::input::Button         respawn;
	tt::input::Button         menu;
	tt::input::Button         screenSwitch;
	tt::input::Button         startupFailSafeLevel;
	tt::input::Stick          direction;
	tt::input::Stick          gyroStick;  // 'stick' based on gyroscope input (pitch and roll currently)
	tt::input::Stick          scroll;
	
	// Debug buttons
	tt::input::Button         debugCheat;
	tt::input::Button         debugRestart;
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
};


struct Frame
{
	u64        time;
	real       deltaTime;
	InputState state;
	
	Frame() : time(0), deltaTime(0.0f), state() { }
};
typedef std::vector<Frame> Frames;


struct GameState
{
	tt::math::Vector2 currentPosition;
	s32               activeEntities;
	u64               randomSeedValue;
	
	GameState()
	:
	currentPosition(0,0),
	activeEntities(0),
	randomSeedValue(0)
	{ }
};
typedef std::vector<GameState> GameStates;


enum BuildConfiguration
{
	Build_Dev,
	Build_Test,
	Build_Final
};


struct ProgressSection
{
	game::script::Registry registry;
	game::CheckPointMgrPtr checkPoints;
};


struct RecordingSection
{
	std::string     levelName;
	u64             randomSeed;
	Frames          frames;
	GameStates      gameStates;
	ProgressSection progress[ProgressType_Count];
};
typedef std::vector<RecordingSection> RecordingSections;


struct FileHeader
{
	FileHeader()
	:
	compressed(false),
	compressedSize(0),
	uncompressedSize(0)
	{ }
	
	bool compressed;
	u32  compressedSize;
	u32  uncompressedSize;
	
	bool save(tt::fs::FilePtr& p_file) const;
	bool load(const tt::fs::FilePtr& p_file);
};


// Header information
struct RecordingHeader
{
	RecordingHeader();
	
	tt::app::Platform                  host;
	BuildConfiguration                 config;
	s32                                revision;
	s32                                gameStateFrequency;
	u64                                startTime;
	u64                                stopTime;
	u32                                targetFPS;
	serialization::SerializationMgrPtr startState;
	
	void validate() const;
	
	bool save(tt::fs::FilePtr& p_file) const;
	bool load(const tt::fs::FilePtr& p_file);
};


class Recording
{
public:
	~Recording();
	static RecordingPtr create();
	static RecordingPtr load(const std::string& p_filename);
	
	void stop();
	
	bool addSection  (const RecordingSection& p_newSection);
	bool addFrame    (const Frame& p_newFrame);
	bool addGameState(const GameState& p_newGameState);
	
	bool isGameStateFrame(u32 p_frameIndex) const;
	
	const RecordingSection& getSection(u32 p_index) const;
	
	inline bool isEmpty() const { return m_sections.empty(); }
	inline u32 getTotalSections() const { return static_cast<u32>(m_sections.size()); }
	
	inline const RecordingHeader& getHeader() const   { return m_header;   }
	inline const std::string&     getFilePath() const { return m_filePath; }
	
private:
	enum Mode
	{
		Mode_Created,
		Mode_Loaded
	};
	
	// Constructor for Mode_Created
	Recording(const RecordingHeader& p_header,
	          const tt::fs::FilePtr& p_file, const std::string& p_filePath);
	
	// Constructor for Mode_Loaded
	Recording(const RecordingHeader& p_header, const RecordingSections& p_sections,
	          const std::string& p_filePath);
	
	static bool read         (const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT);
	static bool readSection  (const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT);
	static bool readFrame    (const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT);
	static bool readGameState(const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT);
	
	void compress() const;
	static tt::fs::FilePtr decompress(const std::string& p_filename);
	
	RecordingHeader   m_header;
	RecordingSections m_sections;
	tt::fs::FilePtr   m_file;
	std::string       m_filePath;
	
	Mode              m_mode;
	bool              m_alwaysFlush;
};

// Namespace end
}
}

#endif // INC_TOKI_INPUT_RECORDING_H
