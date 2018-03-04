#include "Editor.h"
#include <SpriteBatchBuffer.h>

Editor::Editor() {
	_grid = new Grid(GRID_SIZE_X, GRID_SIZE_Y);
	_startPoint = p2i(1, 1);
	_endPoint = p2i(18, 12);
	_grid->setStart(_startPoint.x, _startPoint.y);
	_grid->setEnd(_endPoint.x, _endPoint.y);
	_selectedType = 0;
	sprintf_s(_name, "Testlevel");
}

Editor::~Editor() {
	delete _grid;
}

void Editor::buttonClicked(int index) {
	if (_gridPos.x != -1 && _gridPos.y != -1) {
		if (index == 0) {
			_grid->set(_gridPos, _selectedType);
		}
		if (index == 1) {
			if (_grid->get(_gridPos) == 0) {
				_grid->set(_gridPos, 1);
			}
			else {
				_grid->set(_gridPos, 0);
			}
		}
	}
}

void Editor::render(SpriteBatchBuffer* buffer) {
	for (int y = 0; y < _grid->height; ++y) {
		for (int x = 0; x < _grid->width; ++x) {
			int type = _grid->get(x, y);
			ds::vec2 p = ds::vec2(START_X + x * 46, START_Y + 46 * y);
			buffer->add(p, GRID_TEXTURES[type]);
		}
	}

	buffer->add(ds::vec2(640,650), GRID_TEXTURES[_selectedType]);
}

void Editor::tick(float dt) {
	ds::vec2 mp = ds::getMousePosition();
	p2i gridPos;
	if (convert(mp.x, mp.y, &gridPos)) {
		_gridPos = gridPos;
	}
	else {
		_gridPos = p2i(-1, -1);
	}
}

// ---------------------------------------------------------------
// show GUI
// ---------------------------------------------------------------
void Editor::showGUI() {
	p2i dp(10, 710);
	int state = 1;
	gui::setAlphaLevel(0.3f);
	gui::start(&dp, 240);
	gui::begin("Grid", 0);
	ds::vec2 mp = ds::getMousePosition();
	gui::Value("Mouse", mp);
	gui::Value("Pos", _gridPos);
	gui::TextBox("Name", _name, 16);
	gui::StepInput("Type", &_selectedType, 0, 20, 1);
	if (gui::Button("Load")) {
		_grid->load(_name);
	}
	if (gui::Button("Save")) {
		_grid->save(_name);		
	}
	gui::end();
}