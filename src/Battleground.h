#pragma once
#include <diesel.h>
#include <vector>
#include <ds_sprite_app.h>
#include "Grid.h"
#include "lib\DataArray.h"
#include "ApplicationContext.h"

class FlowField;
class SpriteBatchBuffer;

struct Level {
	const char* name;
	WalkerType::Enum types[32];
	int count[32];
	int numWaves;
};

typedef std::vector<Tower> Towers;

class Battleground : public ds::Scene {

public:
	Battleground();
	~Battleground();
	void render(SpriteBatchBuffer* buffer);
	void startWalker(int definitionIndex);
	void startWalkers(int definitionIndex, int count, float ttl);
	void addTower(ds::vec2& screenPos,int defIndex);
	void update(float dt);
	void showGUI();
private:
	void startAnimation(int index);
	void readTowerDefinitions();
	void buildPath();
	void emittWalker(float dt);
	void moveWalkers(float dt);
	bool isClose(const Tower& tower, const Walker& walker) const;
	void rotateTowers();
	void startBullet(int towerIndex, int energy);
	void moveBullets(float dt);
	bool checkWalkerCollision(const ds::vec2& pos, float radius, int energy);
	void fireBullets(float dt);
	ds::DataArray<Walker> _walkers;
	ds::DataArray<Bullet> _bullets;
	Grid* _grid;
	FlowField* _flowField;
	p2i _startPoint;
	p2i _endPoint;
	Towers _towers;
	int _selectedTower;
	PendingWalkers _pendingWalkers;
	WalkerDefinition _definitions[20];
	TowerDefinition _towerDefinitions[10];
	std::vector<p2i> _path;
	// debug
	float _dbgTTL;
	bool _dbgShowOverlay;
	int _dbgWalkerIndex;
	bool _dbgShowPath;
	int _dbgTowerType;
};