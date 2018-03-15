#pragma once
#include <diesel.h>
#include "lib\DataArray.h"

// ---------------------------------------------------------------
// tower rotation animation
// ---------------------------------------------------------------
struct RotationAnimation {
	float angle;
	int direction;
	float timer;
	float ttl;
};

// ---------------------------------------------------------------
// Tower
// ---------------------------------------------------------------
struct Tower {
	int type;
	int gx;
	int gy;
	ds::vec4 texture;
	ds::vec2 position;
	float radius;
	int energy;
	float timer;
	float bulletTTL;
	float direction;
	ID target;
	int level;
	RotationAnimation animation;
	int animationState;
};

// ---------------------------------------------------------------
// Tower definition
// ---------------------------------------------------------------
struct TowerDefinition {
	ds::vec4 texture;
	int radius;
	int energy;
	float bulletTTL;
};

// ---------------------------------------------------------------
// walker type
// ---------------------------------------------------------------
struct WalkerType {

	enum Enum {
		SIMPLE_CELL,
		FAST_CELL
	};
};

// ---------------------------------------------------------------
// walker definition
// ---------------------------------------------------------------
struct WalkerDefinition {
	ds::vec4 texture;
	int energy;
	ds::Color color;
	float velocity;
};

// ---------------------------------------------------------------
// pending walkers
// ---------------------------------------------------------------
struct PendingWalkers {
	int definitionIndex;
	int count;
	float timer;
	float ttl;
};

// ---------------------------------------------------------------
// walker
// ---------------------------------------------------------------
struct Walker {
	ID id;
	p2i gridPos;
	float velocity;
	ds::vec2 pos;
	float rotation;
	WalkerType::Enum type;
	int definitionIndex;
	int energy;
};

// ---------------------------------------------------------------
// bullet
// ---------------------------------------------------------------
struct Bullet {
	ID id;
	ds::vec2 pos;
	float radius;
	ds::vec2 velocity;
	float timer;
	float ttl;
	int energy;
};
