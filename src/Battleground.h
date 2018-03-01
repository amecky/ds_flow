#pragma once
#include <diesel.h>
#include <vector>
#include "..\Grid.h"
#include "lib\DataArray.h"

class FlowField;
class SpriteBatchBuffer;

struct Walker {
	ID id;
	p2i gridPos;
	ds::vec2 velocity;
	ds::vec2 pos;
	float rotation;
};

struct Tower {
	int type;
	int gx;
	int gy;
	ds::vec2 position;
	float radius;
	int energy;
	float timer;
	float bulletTTL;
	float direction;
};

typedef std::vector<Tower> Towers;

class Battleground {

public:
	Battleground();
	~Battleground();
	void render(SpriteBatchBuffer* buffer);
	void startWalker();
	void addTower(ds::vec2& screenPos);
	void tick(float dt);
private:
	void moveWalkers(float dt);
	ds::DataArray<Walker> _walkers;
	std::vector<ID> _walker_ids;
	Grid* _grid;
	FlowField* _flowField;
	p2i _startPoint;
	p2i _endPoint;
	Towers _towers;
};