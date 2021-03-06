#pragma once
#include <diesel.h>
#include <Windows.h>
#include <vector>
#include <stdint.h>

//#define BASE_APP_IMPLEMENTATION

class SpriteBatchBuffer;

namespace ds {

	// ----------------------------------------------------
	// Application settings
	// ----------------------------------------------------
	struct ApplicationSettings {
		bool useIMGUI;
		bool useGPUProfiling;
		int screenWidth;
		int screenHeight;
		const char* windowTitle;
		ds::Color clearColor;
		char guiToggleKey;
	};

	// ----------------------------------------------------
	// event stream
	// ----------------------------------------------------
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

	// ----------------------------------------------------
	// Scene
	// ----------------------------------------------------
	class Scene {

	public:
		Scene() : _active(false), _initialized(false) {}
		virtual ~Scene() {}
		virtual void beforeRendering() {}
		virtual void render() {}
		virtual void afterRendering() {}
		virtual void update(float dt) {}
		virtual void prepare(ds::EventStream* events) {
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
		virtual void OnButtonClicked(int index) {}
	protected:
		RID loadImageFromFile(const char* name);
		ds::EventStream* _events;
		bool _active;
		bool _initialized;
	};

	// ----------------------------------------------------
	// SpriteScene
	// ----------------------------------------------------
	class SpriteScene : public Scene {

	public:
		SpriteScene(SpriteBatchBuffer* buffer) : Scene() , _buffer(buffer) {}
		virtual ~SpriteScene() {}
		void beforeRendering();
		void afterRendering();
	protected:
		SpriteBatchBuffer* _buffer;
	};

	// ----------------------------------------------------
	// BaseScene
	// ----------------------------------------------------
	class BaseScene : public Scene {

	public:
		BaseScene() : Scene() {}
		virtual ~BaseScene() {}
		virtual void prepare(ds::EventStream* events) {
			if (!_initialized) {
				_events = events;
				_initialized = true;
				_camera = ds::buildPerspectiveCamera(ds::vec3(0.0f, 3.0f, -6.0f));

				ds::ViewportInfo vpInfo = { ds::getScreenWidth(), ds::getScreenHeight(), 0.0f, 1.0f };
				_viewPort = ds::createViewport(vpInfo);

				ds::RenderPassInfo rpInfo = { &_camera, _viewPort, ds::DepthBufferState::ENABLED, 0, 0 };
				_basicPass = ds::createRenderPass(rpInfo);

				ds::BlendStateInfo blendInfo = { ds::BlendStates::SRC_ALPHA, ds::BlendStates::SRC_ALPHA, ds::BlendStates::INV_SRC_ALPHA, ds::BlendStates::INV_SRC_ALPHA, true };
				_blendStateID = ds::createBlendState(blendInfo);
			}
		}
	protected:
		RID _viewPort;
		RID _blendStateID;
		RID _basicPass;
		ds::Camera _camera;
	};

	struct ButtonState {
		bool pressed;
		bool clicked;
	};

	// ----------------------------------------------------
	// BaseApp
	// ----------------------------------------------------
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
			scene->setActive(true);
			_scenes.push_back(scene);
		}
		void popScene() {
			if (!_scenes.empty()) {
				Scene* scene = _scenes[_scenes.size() - 1];
				scene->onDeactivation();
				scene->setActive(false);
			}
			_scenes.pop_back();
		}
		void initializeSettings(const char* settingsFileName);
		void loadSettings();
		bool isRunning() const {
			return _running;
		}		
		SpriteBatchBuffer* createSpriteBatchBuffer(RID textureID, int maxSprites);
	protected:
		void stopGame() {
			_running = false;
		}
		RID loadImageFromResource(LPCTSTR name, LPCTSTR type);		
		ApplicationSettings _settings;
		ds::EventStream* _events;
	private:
		void handleButtons();
		SpriteBatchBuffer* _sprites;
		Scenes _scenes;
		float _loadTimer;
		const char* _settingsFileName;
		bool _useTweakables;
		bool _guiKeyPressed;
		bool _guiActive;
		bool _running;
		ButtonState _buttonStates[2];
	};


	
}

extern ds::BaseApp* app;

#ifdef BASE_APP_IMPLEMENTATION
//#include <SpriteBatchBuffer.h>
//#include <stb_image.h>
//#include <ds_tweakable.h>
//#include <ds_imgui.h>

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
		_settings.windowTitle = "BaseApp";
		_settings.clearColor = ds::Color(0.1f, 0.1f, 0.1f, 1.0f);
		_settings.guiToggleKey = 'D';
		_events = new ds::EventStream;
		_loadTimer = 0.0f;
		_useTweakables = false;
		_guiKeyPressed = false;
		_guiActive = true;
		_running = true;
		_sprites = 0;
		_buttonStates[0] = { false, false };
		_buttonStates[1] = { false, false };
	}

	BaseApp::~BaseApp() {
		if (_useTweakables) {
			twk_shutdown();
		}
		if (_settings.useIMGUI) {
			gui::shutdown();
		}
		if (_sprites != 0) {
			delete _sprites;
		}
		delete _events;
	}

	// ---------------------------------------------------------------
	// handle buttons
	// ---------------------------------------------------------------
	void BaseApp::handleButtons() {
		for (int i = 0; i < 2; ++i) {
			if (ds::isMouseButtonPressed(i)) {
				_buttonStates[i].pressed = true;
			}
			else if (_buttonStates[i].pressed) {
				_buttonStates[i].pressed = false;
				if (!_buttonStates[i].clicked) {
					_buttonStates[i].clicked = true;
				}
				else {
					_buttonStates[i].clicked = false;
				}
			}
		}
	}

	// -------------------------------------------------------
	// init
	// -------------------------------------------------------
	void BaseApp::init() {
		SetThreadAffinityMask(GetCurrentThread(), 1);
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

		ds::log(LogLevel::LL_DEBUG, "=> Press '%c' to toggle GUI", _settings.guiToggleKey);
		initialize();
	}

	SpriteBatchBuffer* BaseApp::createSpriteBatchBuffer(RID textureID, int maxSprites) {
		SpriteBatchBufferInfo sbbInfo = { 2048, textureID , ds::TextureFilters::LINEAR };
		_sprites = new SpriteBatchBuffer(sbbInfo);
		return _sprites;
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

		if (ds::isKeyPressed(_settings.guiToggleKey)) {
			if (!_guiKeyPressed) {
				_guiActive = !_guiActive;
				_guiKeyPressed = true;
			}
		}
		else {
			_guiKeyPressed = false;
		}

		handleButtons();

		for (int i = 0; i < 2; ++i) {
			if (_buttonStates[i].clicked) {
				ScenesIterator it = _scenes.begin();
				while (it != _scenes.end()) {
					if ((*it)->isActive()) {
						(*it)->OnButtonClicked(i);
					}
					++it;
				}
				_buttonStates[i].clicked = false;
			}
		}

		_events->reset();

		update(dt);

		render();

		ScenesIterator it = _scenes.begin();
		while (it != _scenes.end()) {
			(*it)->update(dt);
			++it;
		}
		it = _scenes.begin();
		while (it != _scenes.end()) {
			(*it)->beforeRendering();
			(*it)->render();
			(*it)->afterRendering();
			++it;
		}

		if (_settings.useIMGUI && _guiActive) {
			it = _scenes.begin();
			while (it != _scenes.end()) {
				(*it)->showGUI();
				++it;
			}
		}
		
		handleEvents(_events);
	}

	RID Scene::loadImageFromFile(const char* name) {
		int x, y, n;
		unsigned char *data = stbi_load(name, &x, &y, &n, 4);
		ds::TextureInfo info = { x,y,n,data,ds::TextureFormat::R8G8B8A8_UNORM , ds::BindFlag::BF_SHADER_RESOURCE };
		RID textureID = ds::createTexture(info, name);
		stbi_image_free(data);
		return textureID;
	}

	void SpriteScene::beforeRendering() {
		_buffer->begin();
	}

	void SpriteScene::afterRendering() {
		_buffer->flush();
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
