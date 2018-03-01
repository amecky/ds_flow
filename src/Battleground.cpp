#include "Battleground.h"
#include "..\FlowField.h"
#include <SpriteBatchBuffer.h>

const static int START_X = 52;
const static int START_Y = 62;

void readGridData(Grid* grid, p2i* start, p2i* end) {
	FILE* fp = fopen("field.txt", "r");
	char* data = 0;
	int fileSize = -1;
	if (fp) {
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		data = new char[fileSize + 1];
		fread(data, 1, fileSize, fp);
		data[fileSize] = '\0';
		fclose(fp);
	}
	if (data != 0) {
		int idx = 0;
		for (int y = 13; y >= 0; --y) {
			for (int x = 0; x < 20; ++x) {
				if (data[idx] == 'x') {
					grid->set(x, y, 1);
				}
				if (data[idx] == '=') {
					grid->set(x, y, 4);
				}
				if (data[idx] == 's') {
					*start = p2i(x, y);
				}
				if (data[idx] == 'e') {
					*end = p2i(x, y);
				}
				++idx;
			}
			++idx;
		}
	}
	delete[] data;
}

ds::vec2 convert_to_screen(int gx, int gy) {
	return{ START_X + gx * 46, START_Y + gy * 46 };
}

// ---------------------------------------------------------------
// get angle between two ds::vec2 vectors
// ---------------------------------------------------------------
float getAngle(const ds::vec2& u, const ds::vec2& v) {
	double x = v.x - u.x;
	double y = v.y - u.y;
	double ang = atan2(y, x);
	return (float)ang;
}

// ---------------------------------------------------------------
// convert screen coordinates to grid position if possible
// ---------------------------------------------------------------
bool convert(int screenX, int screenY, int startX, int startY, p2i* ret) {
	if (screenX >= (startX - 23) && screenY >= (startY - 23)) {
		ret->x = (screenX - startX + 23) / 46;
		ret->y = (screenY - startY + 23) / 46;
		return true;
	}
	return false;
}

bool convert(int screenX, int screenY, p2i* ret) {
	if (screenX >= (START_X - 23) && screenY >= (START_Y - 23)) {
		ret->x = (screenX - START_X + 23) / 46;
		ret->y = (screenY - START_Y + 23) / 46;
		return true;
	}
	return false;
}

Battleground::Battleground() {
	_grid = new Grid(20, 14);
	_startPoint = p2i(0, 0);
	_endPoint = p2i(0, 0);
	readGridData(_grid, &_startPoint, &_endPoint);
	_grid->setStart(_startPoint.x, _startPoint.y);
	_grid->setEnd(_endPoint.x, _endPoint.y);

	_flowField = new FlowField(_grid);
	_flowField->build(_endPoint);
}

Battleground::~Battleground() {
	delete _flowField;
	delete _grid;
}

void Battleground::render(SpriteBatchBuffer* buffer) {
	for (int y = 0; y < _grid->height; ++y) {
		for (int x = 0; x < _grid->width; ++x) {
			ds::vec2 p = ds::vec2(START_X + x * 46, START_Y + 46 * y);
			ds::vec4 t = ds::vec4(0, 0, 46, 46);
			int type = _grid->items[x + y * _grid->width];
			if (type == 1) {
				t = ds::vec4(46, 0, 46, 46);
			}
			else if (type == 2) {
				t = ds::vec4(138, 0, 46, 46);
			}
			if (type == 3) {
				t = ds::vec4(184, 0, 46, 46);
			}
			if (type != 4) {
				buffer->add(p, t);
				// draw direction
				int d = _flowField->get(x, y);
				if (d >= 0 && d < 9) {
					buffer->add(p, ds::vec4(d * 46, 138, 46, 46));
				}
			}
		}
	}
	//
	// draw towers
	//
	for (size_t i = 0; i < _towers.size(); ++i) {
		const Tower& t = _towers[i];
		buffer->add(t.position, ds::vec4(0, 93, 46, 46), ds::vec2(1.0f), t.direction);
	}
	//
	// draw walkers
	//
	for (uint32_t i = 0; i < _walkers.numObjects;++i) {	
		const Walker& w = _walkers.objects[i];
		buffer->add(w.pos, ds::vec4(276, 0, 24, 24), ds::vec2(1, 1), w.rotation);
		
	}
}

void Battleground::startWalker() {
	ID id = _walkers.add();
	Walker& w = _walkers.get(id);
	w.gridPos = _startPoint;
	w.pos = ds::vec2(START_X + _startPoint.x * 46, START_Y + _startPoint.y * 46);
	w.rotation = 0.0f;
	w.velocity = ds::vec2(0.0f);
	_walker_ids.push_back(id);
}

void Battleground::tick(float dt) {
	moveWalkers(dt);

	for (size_t i = 0; i < _towers.size(); ++i) {
		Tower& t = _towers[i];
		for (uint32_t i = 0; i < _walkers.numObjects; ++i) {
			const Walker& w = _walkers.objects[i];
			float diff = sqr_length(t.position - w.pos);
			if (diff < t.radius * t.radius) {
				t.direction = getAngle(w.pos, t.position);
			}
		}
	}
}

void Battleground::moveWalkers(float dt) {
	for (uint32_t i = 0; i < _walkers.numObjects; ++i) {
		Walker& w = _walkers.objects[i];
		if (_flowField->hasNext(w.gridPos)) {
			p2i n = _flowField->next(w.gridPos);
			p2i nextPos = p2i(START_X + n.x * 46, START_Y + n.y * 46);
			ds::vec2 diff = w.pos - ds::vec2(nextPos.x, nextPos.y);
			if (sqr_length(diff) < 4.0f) {
				convert(w.pos.x, w.pos.y, START_X, START_Y, &w.gridPos);
			}
			ds::vec2 v = normalize(ds::vec2(nextPos.x, nextPos.y) - w.pos) * 100.0f;
			w.pos += v * dt;
			w.rotation = getAngle(w.pos, ds::vec2(nextPos.x, nextPos.y));
		}
	}
}



// ---------------------------------------------------------------
// prepare walker
// ---------------------------------------------------------------
void prepareWalker(Walker* walker, const p2i& start) {
	walker->gridPos = start;
	walker->pos = ds::vec2(START_X + start.x * 46, START_Y + start.y * 46);
	walker->rotation = 0.0f;
	walker->velocity = ds::vec2(0.0f);
}

// ---------------------------------------------------------------
// move walker
// ---------------------------------------------------------------
bool moveWalker(FlowField* flowField, Walker* walker) {
	if (flowField->hasNext(walker->gridPos)) {
		p2i n = flowField->next(walker->gridPos);
		p2i nextPos = p2i(START_X + n.x * 46, START_Y + n.y * 46);
		ds::vec2 diff = walker->pos - ds::vec2(nextPos.x, nextPos.y);
		if (sqr_length(diff) < 4.0f) {
			convert(walker->pos.x, walker->pos.y, START_X, START_Y, &walker->gridPos);
		}
		ds::vec2 v = normalize(ds::vec2(nextPos.x, nextPos.y) - walker->pos) * 100.0f;
		walker->pos += v * static_cast<float>(ds::getElapsedSeconds());
		walker->rotation = getAngle(walker->pos, ds::vec2(nextPos.x, nextPos.y));
		return true;
	}
	return false;
}

void Battleground::addTower(ds::vec2& screenPos) {
	p2i gridPos;
	if (convert(screenPos.x, screenPos.y, &gridPos)) {
		_grid->set(gridPos.x, gridPos.y, 1);
		_flowField->build(_endPoint);
		Tower t;
		t.type = 0;
		t.gx = gridPos.x;
		t.gy = gridPos.y;
		t.position = convert_to_screen(t.gx, t.gy);
		t.radius = 100.0f;
		t.energy = 1;
		t.timer = 0.0f;
		t.bulletTTL = 1.0f;
		t.direction = 0.0f;
		_towers.push_back(t);
	}
}

// ---------------------------------------------------------------
// the texture coordinates for all numbers 0 - 9
// ---------------------------------------------------------------
const ds::vec4 NUMBERS[] = {
	ds::vec4(0,46,17,14),
	ds::vec4(17,46,9,14),
	ds::vec4(26,46,16,14),
	ds::vec4(42,46,16,14),
	ds::vec4(58,46,17,14),
	ds::vec4(75,46,17,14),
	ds::vec4(92,46,17,14),
	ds::vec4(109,46,17,14),
	ds::vec4(126,46,17,14),
	ds::vec4(143,46,16,14),
};

// ---------------------------------------------------------------
// draw number
// ---------------------------------------------------------------
void drawNumber(SpriteBatchBuffer* buffer, int value, ds::vec2 p) {
	if (value < 10) {
		buffer->add(p, NUMBERS[value]);
	}
	else if (value < 100) {
		int f = value / 10;
		int s = value - f * 10;
		ds::vec4 ft = NUMBERS[f];
		ds::vec4 st = NUMBERS[s];
		float w = (ft.z + st.z) * 0.5f;
		p.x -= w;
		buffer->add(p, ft);
		p.x += ft.z + 2;
		buffer->add(p, st);
	}
}


/*
void rotateTowers(Towers& towers, Walkers& walkers) {
	for (size_t i = 0; i < towers.size(); ++i) {
		Tower& t = towers[i];
		for (size_t j = 0; j < walkers.size(); ++j) {
			const Walker& w = walkers[j];
			float diff = sqr_length(t.position - w.pos);

			if (diff < t.radius * t.radius) {
				t.direction = getAngle(w.pos, t.position);
				float d = t.direction * ds::TWO_PI / 360.0f;
				ds::log(LL_DEBUG, "dir %g", d);
			}
		}
	}
}
*/