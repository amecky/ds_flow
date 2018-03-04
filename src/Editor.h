#pragma once
#include "Grid.h"

class SpriteBatchBuffer;

class Editor {

public:
	Editor();
	~Editor();
	void render(SpriteBatchBuffer* buffer);
	void buttonClicked(int index);
	void showGUI();
	void tick(float dt);
private:
	p2i _gridPos;
	Grid* _grid;
	p2i _startPoint;
	p2i _endPoint;
	int _selectedType;
	char _name[16];
};