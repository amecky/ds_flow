#include "FlowApplication.h"
#include <SpriteBatchBuffer.h>
#include <ds_game_ui.h>
#include <Windows.h>
#include <stb_image.h>
#include "Editor.h"
#include "Battleground.h"
#include <ds_imgui.h>
#include "EventTypes.h"

ds::BaseApp *app = new FlowApplication();
// ---------------------------------------------------------------
// bitmap font definitions
// ---------------------------------------------------------------
static const ds::vec2 FONT_DEF[] = {
	ds::vec2(1,24),   // A
	ds::vec2(24,21),  // B
	ds::vec2(45,20),  // C
	ds::vec2(66,22),  // D
	ds::vec2(88,19),  // E
	ds::vec2(108,19), // F
	ds::vec2(127,21), // G
	ds::vec2(149,21), // H
	ds::vec2(170, 9), // I
	ds::vec2(179,13), // J
	ds::vec2(192,21), // K
	ds::vec2(213,19), // L
	ds::vec2(232,29), // M
	ds::vec2(261,21), // N
	ds::vec2(282,23), // O
	ds::vec2(305,21), // P
	ds::vec2(327,21), // Q
	ds::vec2(348,21), // R
	ds::vec2(369,19), // S 
	ds::vec2(388,19), // T
	ds::vec2(407,21), // U
	ds::vec2(428,24), // V
	ds::vec2(452,30), // W
	ds::vec2(482,23), // X
	ds::vec2(505,22), // Y
	ds::vec2(527,19)  // Z
};

// ---------------------------------------------------------------
// build the font info needed by the game ui
// ---------------------------------------------------------------
void prepareFontInfo(dialog::FontInfo* info) {
	// default for every character just empty space
	for (int i = 0; i < 255; ++i) {
		info->texture_rects[i] = ds::vec4(114, 4, 20, 19);
	}
	// numbers
	for (int c = 48; c <= 57; ++c) {
		int idx = (int)c - 48;
		info->texture_rects[c] = ds::vec4(idx * 22, 490, 22, 19);
	}
	// :
	info->texture_rects[58] = ds::vec4(220, 490, 18, 19);
	// %
	info->texture_rects[37] = ds::vec4(241, 490, 36, 19);
	// characters
	for (int c = 65; c <= 90; ++c) {
		ds::vec2 fd = FONT_DEF[(int)c - 65];
		info->texture_rects[c] = ds::vec4(0.0f + fd.x, 440.0f, fd.y, 19.0f);
	}
}

// ---------------------------------------------------------------
// load image using stb_image
// ---------------------------------------------------------------
RID loadImage(const char* name) {
	int x, y, n;
	unsigned char *data = stbi_load(name, &x, &y, &n, 4);
	ds::TextureInfo texInfo = { x, y, n, data, ds::TextureFormat::R8G8B8A8_UNORM , ds::BindFlag::BF_SHADER_RESOURCE };
	RID textureID = ds::createTexture(texInfo);
	stbi_image_free(data);
	return textureID;
}



FlowApplication::FlowApplication() : ds::BaseApp() {
	_settings.screenWidth = 1280;
	_settings.screenHeight = 720;
	_settings.windowTitle = "Flow";
	_settings.useIMGUI = true;
	_settings.clearColor = ds::Color(16, 16, 16, 255);
	_leftButton = { false, false };
	_rightButton = { false, false };
}


FlowApplication::~FlowApplication() {
	delete _editor;
	delete _battleGround;
}

// ---------------------------------------------------------------
// initialize
// ---------------------------------------------------------------
void FlowApplication::initialize() {
	//
	// load the one and only texture
	RID textureID = loadImage("TextureArray.png");

	SpriteBatchBufferInfo sbbInfo = { 2048, textureID , ds::TextureFilters::LINEAR };
	_buffer = new SpriteBatchBuffer(sbbInfo);
	
	_editor = new Editor();
	_battleGround = new Battleground();
	pushScene(_battleGround);
}

// ---------------------------------------------------------------
// handle events
// ---------------------------------------------------------------
void FlowApplication::handleEvents(ds::EventStream* events) {
	if (events->num() > 0) {
		for (uint32_t i = 0; i < events->num(); ++i) {
			int type = events->getType(i);
			if (type == 100) {
				//popScene();
				//pushScene(_mainGameScene);
			}
			else if (type == 101) {
				//popScene();
				//pushScene(_mapSelectionScene);
			}
			else if (type == 102) {
				stopGame();
			}
		}
	}
}

// ---------------------------------------------------------------
// handle buttons
// ---------------------------------------------------------------
void FlowApplication::handleButton(int index, ButtonState* state) {
	if (ds::isMouseButtonPressed(index)) {
		state->pressed = true;
	}
	else if (state->pressed) {
		state->pressed = false;
		if (!state->clicked) {
			state->clicked = true;
		}
		else {
			state->clicked = false;
		}
	}
}

// ---------------------------------------------------------------
// update
// ---------------------------------------------------------------
void FlowApplication::update(float dt) {
	handleButton(0, &_leftButton);
	if (_leftButton.clicked) {
		_events->add(EventType::LEFT_BUTTON_CLICKED);
		_leftButton.clicked = false;
	}
	handleButton(1, &_rightButton);
	if (_rightButton.clicked) {
		_events->add(EventType::RIGHT_BUTTON_CLICKED);
		_rightButton.clicked = false;
	}
}
