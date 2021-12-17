#ifndef VISUALBOY_H
#define VISUALBOY_H

#include <tt/code/Buffer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/Semaphore.h>
#include <tt/thread/thread.h>

class VisualBoy
{
public:
	static void initialize();
	static void deinitialize();
	
	static void load(const std::string& p_file);
	static bool isLoaded();
	static void setSoundVolume(real p_volume);
	static void setFilter(const std::string& p_filter);
	static void pause();
	static bool isPaused();
	static void resume();
	
	static void update();
	static void updateForRender();
	static void unload();
	
	static void systemDrawScreen();
	
	inline static const tt::engine::renderer::TexturePtr& getOutputTexture()  { return ms_outputTexture;  }
	
	// matches the "spritestrip/__visualboy" crc
	static inline u64 getVisualBoyTextureCRC() { return 5236507638855379561L; }
	
	//static inline void setInputEnabled(bool p_enabled) { ms_inputEnabled = p_enabled; }
	//static inline bool isInputEnabled() { return ms_inputEnabled; }
	
private:
	enum Action
	{
		Action_Load,
		Action_SetFilter,
		Action_Unload
	};
	
	using QueueEntry = std::pair<Action, std::string>;
	using Queue = std::vector<QueueEntry>;
	static void loadImpl(const std::string& p_file);
	static void unloadImpl();
	static int updateImpl(void* p_arg);
	static bool setFilterImpl(const std::string& p_filter);
	static void initVideo();
	static void initSound();
	static void deinitVideo();
	static void deinitSound();
	
	static tt::thread::handle ms_thread;
	static tt::thread::Semaphore ms_waitForUpdate;
	static tt::thread::Mutex ms_mutex;
	static tt::thread::Mutex ms_drawMutex;
	
	static tt::engine::renderer::TexturePtr ms_outputTexture;
	static tt::code::BufferPtr              ms_buffer;
	static bool ms_inputEnabled;
	static bool ms_threadShouldExit;
	static Queue ms_queue;
	static std::string ms_batteryFilename;
};

#endif // VISUALBOY_H
