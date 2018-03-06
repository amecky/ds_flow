#pragma once
#include <diesel.h>
#include <Windows.h>
#include <vector>
#include <stdint.h>

class SpriteBatchBuffer;

namespace ds {

	struct ApplicationSettings {
		bool useIMGUI;
		bool useGPUProfiling;
		int screenWidth;
		int screenHeight;
		const char* windowTitle;
		ds::Color clearColor;
	};

	class EventStream {

		struct EventHeader {
			uint32_t id;
			uint32_t type;
			size_t size;
		};

	public:
		EventStream();
		virtual ~EventStream();
		void reset();
		void add(uint32_t type);
		void add(uint32_t type, void* p, size_t size);
		const bool get(uint32_t index, void* p) const;
		const int getType(uint32_t index) const;
		const bool containsType(uint32_t type) const;
		const uint32_t num() const {
			return _mappings.size();
		}
	private:
		void addHeader(uint32_t type, size_t size);
		EventStream(const EventStream& orig) {}
		char* _data;
		std::vector<uint32_t> _mappings;
		uint32_t _index;
	};

	class Scene {

	public:
		Scene() : _active(false), _initialized(false) {}
		virtual ~Scene() {}
		virtual void render(SpriteBatchBuffer* buffer) {}
		virtual void update(float dt) {}
		void prepare(ds::EventStream* events) {
			if (!_initialized) {
				_events = events;
				_initialized = true;
			}
		}
		virtual void initialize() {}
		void setActive(bool active) {
			if (active) {
				onActivation();
			}
			else {
				onDeactivation();
			}
			_active = active;
		}
		virtual void onActivation() {}
		virtual void onDeactivation() {}
		virtual void showGUI() {}
		bool isActive() const {
			return _active;
		}
	protected:
		ds::EventStream* _events;
	private:
		bool _active;
		bool _initialized;
	};

	class BaseApp {

		typedef std::vector<Scene*> Scenes;
		typedef std::vector<Scene*>::iterator ScenesIterator;

	public:
		BaseApp();
		~BaseApp();
		const ApplicationSettings& getSettings() const {
			return _settings;
		}
		void init();
		virtual void initialize() = 0;
		virtual void handleEvents(ds::EventStream* events) {}
		void tick(float dt);
		virtual void render() {}
		virtual void update(float dt) {}
		void pushScene(Scene* scene) {
			scene->prepare(_events);
			scene->initialize();
			scene->onActivation();
			_scenes.push_back(scene);
		}
		void popScene() {
			if (!_scenes.empty()) {
				Scene* scene = _scenes[_scenes.size() - 1];
				scene->onDeactivation();
			}
			_scenes.pop_back();
		}
		void setSpriteBatchBuffer(SpriteBatchBuffer* buffer) {
			_buffer = buffer;
		}
		void initializeSettings(const char* settingsFileName);
		void loadSettings();
		bool isRunning() const {
			return _running;
		}
	protected:
		void stopGame() {
			_running = false;
		}
		RID loadImageFromResource(LPCTSTR name, LPCTSTR type);
		ApplicationSettings _settings;
		ds::EventStream* _events;
		SpriteBatchBuffer* _buffer;
	private:
		Scenes _scenes;
		float _loadTimer;
		const char* _settingsFileName;
		bool _useTweakables;
		bool _guiKeyPressed;
		bool _guiActive;
		bool _running;
	};


	
}

extern ds::BaseApp* app;

#ifdef BASE_APP_IMPLEMENTATION

namespace ds {

	const int EVENT_HEADER_SIZE = 12;

	void debug(const LogLevel& level, const char* message) {
#ifdef DEBUG
		OutputDebugString(message);
		OutputDebugString("\n");
#endif
	}

	// ---------------------------------------------------------------
	// load image using stb_image
	// ---------------------------------------------------------------
	RID BaseApp::loadImageFromResource(LPCTSTR name, LPCTSTR type) {
		int x, y, n;
		HRSRC resourceHandle = ::FindResource(NULL, name, type);
		if (resourceHandle == 0) {
			return NO_RID;
		}
		DWORD imageSize = ::SizeofResource(NULL, resourceHandle);
		if (imageSize == 0) {
			return NO_RID;
		}
		HGLOBAL myResourceData = ::LoadResource(NULL, resourceHandle);
		void* pMyBinaryData = ::LockResource(myResourceData);
		unsigned char *data = stbi_load_from_memory((const unsigned char*)pMyBinaryData, imageSize, &x, &y, &n, 4);
		ds::TextureInfo info = { x,y,n,data,ds::TextureFormat::R8G8B8A8_UNORM , ds::BindFlag::BF_SHADER_RESOURCE };
		RID textureID = ds::createTexture(info, "Texture");
		stbi_image_free(data);
		UnlockResource(myResourceData);
		FreeResource(myResourceData);
		return textureID;
	}

	// ---------------------------------------------------------------
	// load text file
	// ---------------------------------------------------------------
	const char* loadTextFile(LPCTSTR name) {

		HRSRC resourceHandle = ::FindResource(NULL, name, RT_RCDATA);
		if (resourceHandle == 0) {
			return 0;
		}
		DWORD imageSize = ::SizeofResource(NULL, resourceHandle);
		if (imageSize == 0) {
			return 0;
		}
		HGLOBAL myResourceData = ::LoadResource(NULL, resourceHandle);
		char* pMyBinaryData = (char*)::LockResource(myResourceData);
		UnlockResource(myResourceData);
		char* ret = new char[imageSize];
		memcpy(ret, pMyBinaryData, imageSize);
		FreeResource(myResourceData);
		return ret;
	}

	// -------------------------------------------------------
	// BaseApp
	// -------------------------------------------------------
	BaseApp::BaseApp() {
		_settings.useIMGUI = false;
		_settings.useGPUProfiling = false;
		_settings.screenWidth = 1280;
		_settings.screenHeight = 720;
		_settings.windowTitle = "ColorZone";
		_settings.clearColor = ds::Color(0.9f, 0.9f, 0.9f, 1.0f);
		_events = new ds::EventStream;
		_loadTimer = 0.0f;
		_useTweakables = false;
		_guiKeyPressed = false;
		_guiActive = true;
		_running = true;
	}

	BaseApp::~BaseApp() {
		if (_useTweakables) {
			twk_shutdown();
		}
		if (_settings.useIMGUI) {
			gui::shutdown();
		}
		delete _events;
	}

	// -------------------------------------------------------
	// init
	// -------------------------------------------------------
	void BaseApp::init() {
		//
		// prepare application
		//
		ds::RenderSettings rs;
		rs.width = _settings.screenWidth;
		rs.height = _settings.screenHeight;
		rs.title = _settings.windowTitle;
		rs.clearColor = _settings.clearColor;
		rs.multisampling = 4;
		rs.useGPUProfiling = _settings.useGPUProfiling;
#ifdef DEBUG
		rs.logHandler = debug;
		rs.supportDebug = true;
#endif
		ds::init(rs);

		if (_settings.useIMGUI) {
			ds::log(LogLevel::LL_DEBUG, "=> IMGUI is enabled");
			gui::init();
		}

		ds::log(LogLevel::LL_DEBUG, "=> Press 'D' to toggle GUI");
		initialize();
	}

	// -------------------------------------------------------
	// intialize settings
	// -------------------------------------------------------
	void BaseApp::initializeSettings(const char* settingsFileName) {
		_settingsFileName = settingsFileName;
		_useTweakables = true;
		ds::log(LogLevel::LL_DEBUG, "=> Tweakables are enabled");
#ifdef DEBUG
		twk_init(_settingsFileName);
#else
		twk_init();
#endif
	}

	// -------------------------------------------------------
	// load settings
	// -------------------------------------------------------
	void BaseApp::loadSettings() {
		if (_useTweakables) {
#ifdef DEBUG
			twk_load();
#else
			const char* txt = loadTextFile(_settingsFileName);
			if (txt != 0) {
				twk_parse(txt);
				delete[] txt;
			}
#endif
		}
	}

	// -------------------------------------------------------
	// tick
	// -------------------------------------------------------
	void BaseApp::tick(float dt) {

		if (_useTweakables) {
#ifdef DEBUG
			_loadTimer += ds::getElapsedSeconds();
			if (_loadTimer >= 1.0f) {
				_loadTimer -= 1.0f;
				twk_load();
			}
#endif
		}

		if (ds::isKeyPressed('D')) {
			if (!_guiKeyPressed) {
				_guiActive = !_guiActive;
				_guiKeyPressed = true;
			}
		}
		else {
			_guiKeyPressed = false;
		}

		_events->reset();

		_buffer->begin();

		update(dt);

		render();

		ScenesIterator it = _scenes.begin();
		while (it != _scenes.end()) {
			(*it)->update(dt);
			++it;
		}
		it = _scenes.begin();
		while (it != _scenes.end()) {
			(*it)->render(_buffer);
			++it;
		}
		_buffer->flush();

		if (_settings.useIMGUI && _guiActive) {
			it = _scenes.begin();
			while (it != _scenes.end()) {
				(*it)->showGUI();
				++it;
			}
		}
		
		handleEvents(_events);
	}

	// -------------------------------------------------------
	// EventStream
	// -------------------------------------------------------
	EventStream::EventStream() {
		_data = new char[4096];
		reset();
	}

	EventStream::~EventStream() {
		delete[] _data;
	}

	// -------------------------------------------------------
	// reset
	// -------------------------------------------------------
	void EventStream::reset() {
		_mappings.clear();
		_index = 0;
	}

	// -------------------------------------------------------
	// add event
	// -------------------------------------------------------
	void EventStream::add(uint32_t type, void* p, size_t size) {
		addHeader(type, size);
		char* data = _data + _index + EVENT_HEADER_SIZE;
		memcpy(data, p, size);
		_mappings.push_back(_index);
		_index += EVENT_HEADER_SIZE + size;
	}

	// -------------------------------------------------------
	// add event
	// -------------------------------------------------------
	void EventStream::add(uint32_t type) {
		addHeader(type, 0);
		char* data = _data + _index;
		_mappings.push_back(_index);
		_index += EVENT_HEADER_SIZE;
	}

	// -------------------------------------------------------
	// add header
	// -------------------------------------------------------
	void EventStream::addHeader(uint32_t type, size_t size) {
		//LOG << "creating event - type: " << type << " size: " << size;
		EventHeader header;
		header.id = _mappings.size();;
		header.size = size;
		header.type = type;
		char* data = _data + _index;
		memcpy(data, &header, EVENT_HEADER_SIZE);
	}

	// -------------------------------------------------------
	// get
	// -------------------------------------------------------
	const bool EventStream::get(uint32_t index, void* p) const {
		//XASSERT(index < _mappings.size(), "Index out of range");
		int lookup = _mappings[index];
		char* data = _data + lookup;
		EventHeader* header = (EventHeader*)data;
		data += EVENT_HEADER_SIZE;
		memcpy(p, data, header->size);
		return true;
	}

	// -------------------------------------------------------
	// get type
	// -------------------------------------------------------
	const int EventStream::getType(uint32_t index) const {
		//XASSERT(index < _mappings.size(), "Index out of range");
		int lookup = _mappings[index];
		char* data = _data + lookup;
		EventHeader* header = (EventHeader*)data;
		return header->type;
	}

	// -------------------------------------------------------
	// contains type
	// -------------------------------------------------------
	const bool EventStream::containsType(uint32_t type) const {
		for (int i = 0; i < _mappings.size(); ++i) {
			if (getType(i) == type) {
				return true;
			}
		}
		return false;
	}


}

#endif
