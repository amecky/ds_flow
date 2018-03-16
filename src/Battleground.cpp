#include "Battleground.h"
#include "..\FlowField.h"
#include <SpriteBatchBuffer.h>
#include <ds_imgui.h>
#include "EventTypes.h"
#include "utils\CSVFile.h"

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
// ctor
// ---------------------------------------------------------------
Battleground::Battleground(SpriteBatchBuffer* buffer) : ds::SpriteScene(buffer) {
	_grid = new Grid(GRID_SIZE_X, GRID_SIZE_Y);
	_startPoint = p2i(0, 0);
	_endPoint = p2i(0, 0);
	_grid->load("TestLevel");
	_startPoint = _grid->getStart();
	_endPoint = _grid->getEnd();
	_selectedTower = -1;
	_flowField = new FlowField(_grid);
	_flowField->build(_endPoint);
	buildPath();
	_pendingWalkers = { WalkerType::SIMPLE_CELL, 0, 0.0f, 0.0f };
	_dbgTTL = 0.4f;
	_dbgShowOverlay = false;
	_dbgShowPath = true;
	_dbgWalkerIndex = 0;
	_dbgTowerType = 0;

	CSVFile csvFile;
	if (csvFile.load("walker_definitions.csv", "resources")) {
		size_t num = csvFile.size();
		for (size_t i = 0; i < num; ++i) {
			const TextLine& tl = csvFile.get(i);
			WalkerDefinition& def = _definitions[i];
			def.texture.x = tl.get_int(0);
			def.texture.y = tl.get_int(1);
			def.texture.z = tl.get_int(2);
			def.texture.w = tl.get_int(3);
			def.energy = tl.get_int(4);
			int colors[4];
			colors[0] = tl.get_int(5);
			colors[1] = tl.get_int(6);
			colors[2] = tl.get_int(7);
			colors[3] = tl.get_int(8);
			def.color = ds::Color(colors[0], colors[1], colors[2], colors[3]);
			def.velocity = tl.get_float(9);
		}
	}
	readTowerDefinitions();
}

// ---------------------------------------------------------------
// dtor
// ---------------------------------------------------------------
Battleground::~Battleground() {
	delete _flowField;
	delete _grid;
}

void Battleground::readTowerDefinitions() {
	CSVFile csvFile;
	if (csvFile.load("tower_definitions.csv", "resources")) {
		size_t num = csvFile.size();
		for (size_t i = 0; i < num; ++i) {
			const TextLine& tl = csvFile.get(i);
			TowerDefinition& def = _towerDefinitions[i];
			def.texture.x = tl.get_int(0);
			def.texture.y = tl.get_int(1);
			def.texture.z = tl.get_int(2);
			def.texture.w = tl.get_int(3);
			def.radius = tl.get_int(4);
			def.energy = tl.get_int(5);
			def.bulletTTL = tl.get_float(6);
		}
	}
}
// ---------------------------------------------------------------
// render
// ---------------------------------------------------------------
void Battleground::render() {
	//
	// draw grid
	//
	for (int y = 0; y < _grid->height; ++y) {
		for (int x = 0; x < _grid->width; ++x) {
			ds::vec2 p = ds::vec2(START_X + x * 46, START_Y + 46 * y);
			int type = _grid->get(x,y);
			_buffer->add(p, GRID_TEXTURES[type]);
			if (_dbgShowOverlay) {
				// draw direction
				int d = _flowField->get(x, y);
				if (d >= 0 && d < 9) {
					_buffer->add(p, ds::vec4(d * 46, 138, 46, 46));
				}
			}
		}
	}

	for (size_t i = 0; i < _path.size(); ++i) {
		p2i p = _path[i];
		int d = _flowField->get(p.x, p.y);
		if (d >= 0 && d < 9) {
			ds::vec2 gp = ds::vec2(START_X + p.x * 46, START_Y + 46 * p.y);
			_buffer->add(gp, ds::vec4(d * 46, 138, 46, 46));
		}
	}
	//
	// draw towers
	//
	for (size_t i = 0; i < _towers.size(); ++i) {
		const Tower& t = _towers[i];
		_buffer->add(t.position, ds::vec4(138 + t.level * 46, 46, 46, 46));
		_buffer->add(t.position, t.texture, ds::vec2(1.0f), t.direction);
	}
	//
	// draw walkers
	//
	for (uint32_t i = 0; i < _walkers.numObjects;++i) {	
		const Walker& w = _walkers.objects[i];
		const WalkerDefinition& def = _definitions[w.definitionIndex];
		_buffer->add(w.pos, def.texture, ds::vec2(1, 1), w.rotation, def.color);
		
	}
	//
	// draw bullets
	//
	for (size_t i = 0; i < _bullets.numObjects; ++i) {
		const Bullet& b = _bullets.objects[i];
		_buffer->add(b.pos, ds::vec4(0, 60, 12, 12));
	}	
}

// ---------------------------------------------------------------
// start walker
// ---------------------------------------------------------------
void Battleground::startWalker(int definitionIndex) {
	const WalkerDefinition& def = _definitions[definitionIndex];
	ID id = _walkers.add();
	Walker& w = _walkers.get(id);
	w.gridPos = _startPoint;
	w.pos = ds::vec2(START_X + _startPoint.x * 46, START_Y + _startPoint.y * 46);
	w.rotation = 0.0f;
	w.velocity = def.velocity;
	w.definitionIndex = definitionIndex;
	w.energy = def.energy;
}

// ---------------------------------------------------------------
// start walkers
// ---------------------------------------------------------------
void Battleground::startWalkers(int definitionIndex, int count, float ttl) {
	_pendingWalkers.timer = 0.0f;
	_pendingWalkers.definitionIndex = definitionIndex;
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
			startWalker(_pendingWalkers.definitionIndex);
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
// tick
// ---------------------------------------------------------------
void Battleground::update(float dt) {

	if (_events->containsType(EventType::RIGHT_BUTTON_CLICKED)) {
		ds::vec2 mp = ds::getMousePosition();
		addTower(mp, _dbgTowerType);
	}
	if (_events->containsType(EventType::LEFT_BUTTON_CLICKED)) {
		ds::vec2 mp = ds::getMousePosition();
		p2i gridPos;
		_selectedTower = -1;
		if (convert(mp.x, mp.y, &gridPos)) {
			for (size_t i = 0; i < _towers.size(); ++i) {
				const Tower& t = _towers[i];
				if (gridPos.x == t.gx && gridPos.y == t.gy) {
					_selectedTower = i;
				}
			}
		}
	}

	emittWalker(dt);

	moveWalkers(dt);

	rotateTowers();

	for (size_t i = 0; i < _towers.size(); ++i) {
		Tower& t = _towers[i];
		if (t.target == INVALID_ID) {
			t.animation.timer += dt;
			if (t.animationState == 0) {
				t.direction += t.animation.angle * dt * static_cast<float>(t.animation.direction);
				if (t.animation.timer >= t.animation.ttl) {
					t.animationState = 1;
					t.animation.timer = 0.0f;
					t.animation.ttl = ds::random(0.5f, 1.5f);
				}
			}
			else {
				if (t.animation.timer >= t.animation.ttl) {
					startAnimation(i);
				}
			}
		}
	}

	moveBullets(dt);

	fireBullets(dt);
}

// -------------------------------------------------------------
// start rotation animation
// -------------------------------------------------------------
void Battleground::startAnimation(int index) {
	Tower& t = _towers[index];
	float min = ds::PI * 0.25f;
	float angle = ds::random(min, min + ds::PI * 0.5f);
	t.animation.angle = angle;
	float dir = ds::random(-5.0f, 5.0f);
	t.animation.direction = 1;
	if (dir < 0.0f) {
		t.animation.direction = -1;
	}
	t.animation.timer = 0.0f;
	t.animation.ttl = angle / min * 2.0f;
	t.animationState = 0;
	//DBG_LOG("angle %3.2f ttl %2.4f direction %d", (angle*360.0f / ds::TWO_PI), t.animation.ttl,t.animation.direction);
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
				startBullet(i,t.energy);
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
	ID id = _bullets.add();
	Bullet& b = _bullets.get(id);
	b.energy = t.energy;
	b.pos = t.position;
	b.timer = 0.0f;
	b.ttl = 1.0f;
	const Walker& w = _walkers.get(t.target);
	ds::vec2 dd = w.pos - t.position;
	float direction = getAngle(ds::vec2(1, 0), dd);
	b.velocity = 400.0f * ds::vec2(cos(direction), sin(direction));
}

// ---------------------------------------------------------------
// check walker collision
// ---------------------------------------------------------------
bool Battleground::checkWalkerCollision(const ds::vec2& pos, float radius, int energy) {
	float sumRadius = radius + 12.0f;
	for (uint32_t i = 0; i < _walkers.numObjects; ++i) {
		Walker& w = _walkers.objects[i];
		float diff = sqr_length(pos - w.pos);
		if (diff < sumRadius * sumRadius) {
			if (_walkers.contains(w.id)) {
				w.energy -= energy;
				if (w.energy <= 0) {
					_walkers.remove(w.id);
				}
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
	for (int i = 0; i < _bullets.numObjects; ++i) {
		Bullet& b = _bullets.objects[i];
		b.pos += b.velocity * dt;
		if (checkWalkerCollision(b.pos, 6.0f, b.energy)) {
			if (_bullets.contains(b.id)) {
				_bullets.remove(b.id);
			}
		}
		else if (b.pos.x < 0.0f || b.pos.x > 1020.0f || b.pos.y < 0.0f || b.pos.y > 760.0f) {
			if (_bullets.contains(b.id)) {
				_bullets.remove(b.id);
			}
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
				}
				else {
					ds::vec2 dd = w.pos - t.position;
					t.direction = getAngle(ds::vec2(1, 0), dd);
				}
			}
			else {
				t.target = INVALID_ID;
			}
		}
		if (t.target == INVALID_ID) {
			for (uint32_t i = 0; i < _walkers.numObjects; ++i) {
				const Walker& w = _walkers.objects[i];
				float diff = sqr_length(t.position - w.pos);
				if (diff < t.radius * t.radius) {
					ds::vec2 dd = w.pos - t.position;
					t.direction = getAngle(ds::vec2(1, 0),normalize(dd));
					t.target = w.id;

				}
			}
		}
	}
}

void Battleground::buildPath() {
	_path.clear();
	p2i current = _startPoint;
	while (_flowField->hasNext(current)) {
		_path.push_back(current);
		current = _flowField->next(current);
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
			ds::vec2 v = normalize(ds::vec2(nextPos.x, nextPos.y) - w.pos) * w.velocity;
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
void Battleground::addTower(ds::vec2& screenPos, int defIndex) {
	p2i gridPos;
	if (convert(screenPos.x, screenPos.y, &gridPos)) {
		if (_grid->get(gridPos) == 0) {
			const TowerDefinition& def = _towerDefinitions[defIndex];
			_grid->set(gridPos.x, gridPos.y, 1);
			_flowField->build(_endPoint);
			buildPath();
			Tower t;
			t.type = 0;
			t.gx = gridPos.x;
			t.gy = gridPos.y;
			t.position = convert_to_screen(t.gx, t.gy);
			t.radius = def.radius;
			t.energy = def.energy;
			t.texture = def.texture;
			t.timer = 0.0f;
			t.bulletTTL = def.bulletTTL;
			t.direction = 0.0f;
			t.target = INVALID_ID;
			t.level = 1;
			t.animationState = 1;
			t.animation.timer = 0.0f;
			t.animation.ttl = ds::random(0.5f, 1.5f);
			_towers.push_back(t);
		}
	}
}

// ---------------------------------------------------------------
// show GUI
// ---------------------------------------------------------------
void Battleground::showGUI() {
	p2i dp(10, 710);
	int state = 1;
	gui::setAlphaLevel(0.3f);
	gui::start(&dp, 300);
	gui::begin("Walkers", 0);
	gui::Checkbox("Show overlay", &_dbgShowOverlay);
	gui::Checkbox("Show path", &_dbgShowPath);
	gui::Input("TTL", &_dbgTTL);
	gui::StepInput("Walker", &_dbgWalkerIndex,0,8,1);
	gui::StepInput("Tower", &_dbgTowerType, 0, 2, 1);
	gui::Value("Bullets", _bullets.numObjects);
	if (gui::Button("Start")) {
		startWalkers(_dbgWalkerIndex , 8, _dbgTTL);
	}
	if (_selectedTower != -1) {
		gui::begin("Tower", 0);
		Tower& t = _towers[_selectedTower];
		gui::Value("Index", _selectedTower);
		gui::Value("Level", t.level);
		if (gui::Button("Upgrade")) {
			++t.level;
			t.energy += 10;
			if (t.level > 3) {
				t.level = 3;
				// upgrade tower properly
			}
		}
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
