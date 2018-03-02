#pragma once
#include <diesel.h>
#include <vector>
#include "..\Grid.h"
#include "lib\DataArray.h"

class FlowField;
class SpriteBatchBuffer;

struct PendingWalkers {	
	int type;
	int count;
	float timer;
	float ttl;
};

struct Walker {
	ID id;
	p2i gridPos;
	ds::vec2 velocity;
	ds::vec2 pos;
	float rotation;
};

struct Bullet {
	ds::vec2 pos;
	float radius;
	ds::vec2 velocity;
	float timer;
	float ttl;
	int energy;
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
	ID target;
};

typedef std::vector<Tower> Towers;

class Battleground {

public:
	Battleground();
	~Battleground();
	void render(SpriteBatchBuffer* buffer);
	void startWalker();
	void startWalkers(int type, int count, float ttl);
	void addTower(ds::vec2& screenPos);
	void tick(float dt);
	void buttonClicked(int index);
	void showGUI();
private:	
	void emittWalker(float dt);
	void moveWalkers(float dt);
	bool isClose(const Tower& tower, const Walker& walker) const;
	void rotateTowers();
	void startBullet(int towerIndex, int energy);
	void moveBullets(float dt);
	bool checkWalkerCollision(const ds::vec2& pos, float radius);
	void fireBullets(float dt);
	ds::DataArray<Walker> _walkers;
	std::vector<Bullet> _bullets;
	Grid* _grid;
	FlowField* _flowField;
	p2i _startPoint;
	p2i _endPoint;
	Towers _towers;
	PendingWalkers _pendingWalkers;
	// debug
	float _dbgTTL;
	bool _dbgShowOverlay;
};