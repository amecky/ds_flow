#define DS_IMPLEMENTATION
#include <diesel.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define SPRITE_IMPLEMENTATION
#include <SpriteBatchBuffer.h>
#define DS_IMGUI_IMPLEMENTATION
#include <ds_imgui.h>
#include "src\Battleground.h"
#include <stdio.h>
#include <Windows.h>



void debug(const LogLevel& level, const char* message) {
//#ifdef DEBUG
	OutputDebugString(message);
	OutputDebugString("\n");
//#endif
}

// ---------------------------------------------------------------
// load image using stb_image
// ---------------------------------------------------------------
RID loadImage(const char* name) {
	int x, y, n;
	unsigned char *data = stbi_load(name, &x, &y, &n, 4);
	ds::TextureInfo texInfo = { x, y, n, data, ds::TextureFormat::R8G8B8A8_UNORM , ds::BindFlag::BF_SHADER_RESOURCE};
	RID textureID = ds::createTexture(texInfo);
	stbi_image_free(data);
	return textureID;
}



struct ButtonState {
	bool pressed;
	bool clicked;
};

void handleButton(int index, ButtonState* state) {
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
// main method
// ---------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {
	//
	// prepare application
	//
	ds::RenderSettings rs;
	rs.width = 1024;
	rs.height = 768;
	rs.title = "Flowfield sandbox";
	rs.clearColor = ds::Color(0.1f, 0.1f, 0.1f, 1.0f);
	rs.multisampling = 4;
	rs.useGPUProfiling = false;
	rs.supportDebug = true;
	rs.logHandler = debug;
	ds::init(rs);
	
	RID textureID = loadImage("TextureArray.png");

	SpriteBatchBufferInfo sbbInfo = { 2048,textureID };
	SpriteBatchBuffer spriteBuffer(sbbInfo);

	ButtonState leftButton = { false, false };
	ButtonState rightButton = { false, false };

	Battleground battleGround;
	battleGround.startWalker();

	gui::init();

	//p2i current(0, 0);

	while (ds::isRunning()) {

		handleButton(0, &leftButton);
		handleButton(1, &rightButton);

		ds::begin();
		//
		// start a new walker
		//
		if (leftButton.clicked) {
			//battleGround.buttonClicked(0);
			leftButton.clicked = false;
		}
		if (rightButton.clicked) {
			ds::vec2 mp = ds::getMousePosition();
			battleGround.addTower(mp);
			rightButton.clicked = false;
		}
		
		battleGround.tick(ds::getElapsedSeconds());

		spriteBuffer.begin();

		battleGround.render(&spriteBuffer);

		spriteBuffer.flush();

		ds::end();
	}
	ds::shutdown();
}