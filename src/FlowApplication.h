#pragma once
#include <ds_sprite_app.h>

class Editor;
class Battleground;

class FlowApplication : public ds::BaseApp {

public:
	FlowApplication();
	~FlowApplication();
	void initialize();
	void handleEvents(ds::EventStream* events);
private:
	Editor* _editor;
	Battleground* _battleGround;
};