#pragma once
#include "Grid.h"
#include <ds_base_app.h>

class SpriteBatchBuffer;

class Editor : public ds::SpriteScene {

public:
	Editor(SpriteBatchBuffer* buffer);
	~Editor();
	void render();
	void buttonClicked(int index);
	void showGUI();
	void update(float dt);
private:
	p2i _gridPos;
	Grid* _grid;
	p2i _startPoint;
	p2i _endPoint;
	int _selectedType;
	char _name[16];
};