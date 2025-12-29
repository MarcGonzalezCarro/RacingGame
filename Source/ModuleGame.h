#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>
#include <set>

class PhysBody;
class PhysicEntity;

struct RaceResults
{
	std::vector<int> finalLeaderboard;
	std::vector<double> lapTimes;
	double totalTime = 0.0;
};

class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	bool CleanUp();
	void CreateCar(int x, int y, int w, int h, float scale, int dir, bool playable, int id);
	void CreateRace(int x, int y, int w, int h, float scale, int dir);
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB);

	void CreateMockUpCar();

	void DestroyMockUpCar();

	void UpdateLeaderboard();

	void DrawWaypointsDebug();

	void DeleteRace();

	void CreateMapBorders();

	void CreateMap(int mapId);

	void LoadWaypoints(int mapId, bool race);

	void DeleteMap();

	double GetRaceTime() const;

public:

	std::vector<PhysicEntity*> entities;
	
	PhysBody* sensor;
	bool sensed;

	Texture2D circle;
	Texture2D box;
	Texture2D rick;
	Texture2D map;
	Texture2D wheel;
	Texture2D carT;
	uint32 lap_fx;
	uint32 accelerate_fx;

	bool onRace = false;
	bool onMenu = true;
	bool debug = false;

	uint32 bonus_fx;

	vec2<int> ray;
	bool ray_on;

	std::vector<PhysBody*> mapBodies;
	std::set<std::set<PhysicEntity*>> collidingEntities;
	std::vector<int> leaderboard;

	RaceResults results;
};
