#include "Battleground.h"
#include "..\FlowField.h"
#include <SpriteBatchBuffer.h>
#include <ds_imgui.h>

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
				else if (data[idx] == '=') {
					grid->set(x, y, 4);
				}
				else if (data[idx] == 's') {
					*start = p2i(x, y);
				}
				else if (data[idx] == 'e') {
					*end = p2i(x, y);
				}
				else if (data[idx] == '.') {
					grid->set(x, y, 0);
				}
				else  {
					ds::log(LL_DEBUG, "FOUND '%c'",data[idx]);
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

// ---------------------------------------------------------------
// ctor
// ---------------------------------------------------------------
Battleground::Battleground() {
	_grid = new Grid(20, 14);
	_startPoint = p2i(0, 0);
	_endPoint = p2i(0, 0);
	readGridData(_grid, &_startPoint, &_endPoint);
	_grid->setStart(_startPoint.x, _startPoint.y);
	_grid->setEnd(_endPoint.x, _endPoint.y);

	_flowField = new FlowField(_grid);
	_flowField->build(_endPoint);

	_pendingWalkers = { 0, 0, 0.0f, 0.0f };
	_dbgTTL = 0.4f;
	_dbgShowOverlay = true;
}

// ---------------------------------------------------------------
// dtor
// ---------------------------------------------------------------
Battleground::~Battleground() {
	delete _flowField;
	delete _grid;
}

// ---------------------------------------------------------------
// render
// ---------------------------------------------------------------
void Battleground::render(SpriteBatchBuffer* buffer) {
	//
	// draw grid
	//
	for (int y = 0; y < _grid->height; ++y) {
		for (int x = 0; x < _grid->width; ++x) {
			ds::vec2 p = ds::vec2(START_X + x * 46, START_Y + 46 * y);
			ds::vec4 t = ds::vec4(0, 0, 46, 46);
			int type = _grid->items[x + y * _grid->width];
			if (type == 0) {
				t = ds::vec4(46, 0, 46, 46);
			}
			else if (type == 1) {
				t = ds::vec4(46, 0, 46, 46);
			}
			else if (type == 2) {
				t = ds::vec4(138, 0, 46, 46);
			}
			else if (type == 3) {
				t = ds::vec4(184, 0, 46, 46);
			}
			else if (type == 4) {
				t = ds::vec4(0, 0, 46, 46);
			}
			//else if (type == 0) {
				buffer->add(p, t);
				if (_dbgShowOverlay) {
					// draw direction
					int d = _flowField->get(x, y);
					if (d >= 0 && d < 9) {
						buffer->add(p, ds::vec4(d * 46, 138, 46, 46));
					}
				}
			//}
		}
	}
	//
	// draw towers
	//
	for (size_t i = 0; i < _towers.size(); ++i) {
		const Tower& t = _towers[i];
		buffer->add(t.position, ds::vec4(46, 92, 46, 46), ds::vec2(1.0f), t.direction);
	}
	//
	// draw walkers
	//
	for (uint32_t i = 0; i < _walkers.numObjects;++i) {	
		const Walker& w = _walkers.objects[i];
		buffer->add(w.pos, ds::vec4(276, 0, 24, 24), ds::vec2(1, 1), w.rotation);
		
	}
	//
	// draw bullets
	//
	for (size_t i = 0; i < _bullets.size(); ++i) {
		const Bullet& b = _bullets[i];
		buffer->add(b.pos, ds::vec4(0, 60, 12, 12));
	}	
}

// ---------------------------------------------------------------
// start walker
// ---------------------------------------------------------------
void Battleground::startWalker() {
	ID id = _walkers.add();
	Walker& w = _walkers.get(id);
	w.gridPos = _startPoint;
	w.pos = ds::vec2(START_X + _startPoint.x * 46, START_Y + _startPoint.y * 46);
	w.rotation = 0.0f;
	w.velocity = ds::vec2(0.0f);
}

// ---------------------------------------------------------------
// start walkers
// ---------------------------------------------------------------
void Battleground::startWalkers(int type, int count, float ttl) {
	_pendingWalkers.timer = 0.0f;
	_pendingWalkers.type = type;
	_pendingWalkers.count = count;
	_pendingWalkers.ttl = ttl;
}

// ---------------------------------------------------------------
// emitt walker
// ---------------------------------------------------------------
void Battleground::emittWalker(float dt) {
	if (_pendingWalkers.count > 0) {
		_pendingWalkers.timer += dt;
		if (_pendingWalkers.timer >= _pendingWalkers.ttl) {
			--_pendingWalkers.count;
			startWalker();
			_pendingWalkers.timer -= _pendingWalkers.ttl;
		}
	}
}

// ---------------------------------------------------------------
// is close (walker still in reach of tower)
// ---------------------------------------------------------------
bool Battleground::isClose(const Tower& tower, const Walker& walker) const {
	float diff = sqr_length(tower.position - walker.pos);
	return (diff < tower.radius * tower.radius);
}

// ---------------------------------------------------------------
// button clicked
// ---------------------------------------------------------------
void Battleground::buttonClicked(int index) {
	if (index == 0) {
		for (size_t i = 0; i < _towers.size(); ++i) {
			startBullet(i, 1);
		}
	}
}

// ---------------------------------------------------------------
// tick
// ---------------------------------------------------------------
void Battleground::tick(float dt) {

	emittWalker(dt);

	moveWalkers(dt);

	rotateTowers();

	moveBullets(dt);

	fireBullets(dt);
}
	
// ---------------------------------------------------------------
// fire bullets
// ---------------------------------------------------------------
void Battleground::fireBullets(float dt) {
	for (size_t i = 0; i < _towers.size(); ++i) {
		Tower& t = _towers[i];
		t.timer += dt;
		if (t.target != INVALID_ID) {			
			if (t.timer >= t.bulletTTL) {
				startBullet(i,1);
				t.timer = 0.0f;
			}
		}
	}
}

// ---------------------------------------------------------------
// start bullet
// ---------------------------------------------------------------
void Battleground::startBullet(int towerIndex, int energy) {
	const Tower& t = _towers[towerIndex];
	Bullet b;
	b.energy = t.energy;
	b.pos = t.position;
	b.timer = 0.0f;
	b.ttl = 1.0f;
	const Walker& w = _walkers.get(t.target);
	ds::vec2 dd = w.pos - t.position;
	float direction = getAngle(ds::vec2(1, 0), dd);
	b.velocity = 400.0f * ds::vec2(cos(direction), sin(direction));
	_bullets.push_back(b);
}

// ---------------------------------------------------------------
// check walker collision
// ---------------------------------------------------------------
bool Battleground::checkWalkerCollision(const ds::vec2& pos, float radius) {
	float sumRadius = radius + 12.0f;
	for (uint32_t i = 0; i < _walkers.numObjects; ++i) {
		const Walker& w = _walkers.objects[i];
		float diff = sqr_length(pos - w.pos);
		if (diff < sumRadius * sumRadius) {
			if (_walkers.contains(w.id)) {
				_walkers.remove(w.id);
			}
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------
// move bullets
// ---------------------------------------------------------------
void Battleground::moveBullets(float dt) {
	std::vector<Bullet>::iterator it = _bullets.begin();
	while (it != _bullets.end()) {
		it->pos += it->velocity * dt;
		if (checkWalkerCollision(it->pos, 6.0f)) {
			it = _bullets.erase(it);
		}
		else if (it->pos.x < 0.0f || it->pos.x > 1020.0f || it->pos.y < 0.0f || it->pos.y > 760.0f) {
			it = _bullets.erase(it);
		}
		else {
			++it;
		}
	}
	
}

// ---------------------------------------------------------------
// rotate towers
// ---------------------------------------------------------------
void Battleground::rotateTowers() {
	for (size_t i = 0; i < _towers.size(); ++i) {
		Tower& t = _towers[i];
		if (t.target != INVALID_ID) {
			if (_walkers.contains(t.target)) {
				const Walker& w = _walkers.get(t.target);
				if (!isClose(t, w)) {
					t.target = INVALID_ID;
					//t.timer = 0.0f;
				}
				else {
					ds::vec2 dd = w.pos - t.position;
					t.direction = getAngle(ds::vec2(1, 0), dd);
				}
			}
			else {
				t.target = INVALID_ID;
				//t.timer = t.bulletTTL;
			}
		}
		if (t.target == INVALID_ID) {
			for (uint32_t i = 0; i < _walkers.numObjects; ++i) {
				const Walker& w = _walkers.objects[i];
				float diff = sqr_length(t.position - w.pos);
				if (diff < t.radius * t.radius) {
					//t.timer = t.bulletTTL;
					ds::vec2 dd = w.pos - t.position;
					t.direction = getAngle(ds::vec2(1, 0),normalize(dd));
					t.target = w.id;

				}
			}
		}
	}
}

// ---------------------------------------------------------------
// move walkers
// ---------------------------------------------------------------
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
		else {
			_walkers.remove(w.id);
		}
	}
}

// ---------------------------------------------------------------
// add tower
// ---------------------------------------------------------------
void Battleground::addTower(ds::vec2& screenPos) {
	p2i gridPos;
	if (convert(screenPos.x, screenPos.y, &gridPos)) {
		if (_grid->get(gridPos) == 0) {
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
			t.bulletTTL = 2.0f;
			t.direction = 0.0f;
			t.target = INVALID_ID;
			_towers.push_back(t);
		}
	}
}

// ---------------------------------------------------------------
// show GUI
// ---------------------------------------------------------------
void Battleground::showGUI() {
	p2i dp(10, 758);
	int state = 1;
	
	gui::start(&dp, 300);
	gui::begin("Walkers", 0);
	gui::Checkbox("Show overlay", &_dbgShowOverlay);
	gui::Input("TTL", &_dbgTTL);
	if (gui::Button("Start")) {
		startWalkers(0, 8, _dbgTTL);
	}
	gui::end();
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
