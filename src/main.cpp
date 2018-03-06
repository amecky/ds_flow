#define DS_IMPLEMENTATION
#include <diesel.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define SPRITE_IMPLEMENTATION
#include <SpriteBatchBuffer.h>
#define GAMESETTINGS_IMPLEMENTATION
#include <ds_tweakable.h>
#define DS_GAME_UI_IMPLEMENTATION
#include <ds_game_ui.h>
#define DS_TWEENING_IMPLEMENTATION
#include <ds_tweening.h>
#define DS_IMGUI_IMPLEMENTATION
#include <ds_imgui.h>
#define BASE_APP_IMPLEMENTATION
#include <ds_sprite_app.h>

extern ds::BaseApp* app;


// ---------------------------------------------------------------
// main method
// ---------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	app->init();

	while (ds::isRunning() && app->isRunning()) {

		ds::begin();

		float dt = static_cast<float>(ds::getElapsedSeconds());

		app->tick(dt);
		
		ds::dbgPrint(0, 34, "FPS: %d", ds::getFramesPerSecond());

		ds::end();
	}	
	ds::shutdown();
	
	delete app;
	
}