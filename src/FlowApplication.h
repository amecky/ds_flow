#pragma once
#include <ds_sprite_app.h>

class Editor;
class Battleground;

struct ButtonState {
	bool pressed;
	bool clicked;
};


class FlowApplication : public ds::BaseApp {

public:
	FlowApplication();
	~FlowApplication();
	void initialize();
	void handleEvents(ds::EventStream* events);
	void update(float dt);
private:
	void handleButton(int index, ButtonState* state);
	Editor* _editor;
	Battleground* _battleGround;
	ButtonState _leftButton;
	ButtonState _rightButton;
};