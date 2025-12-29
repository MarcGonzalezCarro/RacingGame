#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "ModuleState.h"
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

struct Waypoint
{
	int x;
	int y;
	int w;
	int h;
	float angle;         
	PhysBody* sensor;
};
std::vector<Waypoint> waypoints;

class PhysicEntity
{
protected:

	PhysicEntity(PhysBody* _body, Module* _listener)
		: body(_body)
		, listener(_listener)
	{
		body->listener = listener;
	}

public:
	virtual ~PhysicEntity() = default;
	virtual void Update() = 0;
	
	virtual int RayHit(vec2<int> ray, vec2<int> mouse, vec2<float>& normal)
	{
		return 0;
	}

public:
	PhysBody* body;
	Module* listener;
};

class Tire : public PhysicEntity
{
public:
	ModuleRender* renderer;

	Tire(ModuleRender* render,ModulePhysics* physics, int _x, int _y, int _w, int _h, int dir, Module* _listener, Texture2D _texture)
		: PhysicEntity(physics->CreateTire(_x, _y, _w, _h, dir), _listener)
		, texture(_texture), physicsM(physics)
	{
		renderer = render;
	}

	float maxForwardSpeed = 0.25f * 0.7f;  
	float maxBackwardSpeed = 0.05f * 0.5f;
	float maxDriveForce = 1.0f * 0.5f;

	float steeringAngle = 0.0f;
	float maxSteeringAngle = 5.0f;
	float steeringSpeed = 0.05f;

	float maxLateralImpulse = 0.5f;

	int direction = 0;
	Vector2 speed = { 0,0 };

	void Update() override
	{
		if (body == nullptr) {
			LOG("TIRE ERROR: body == NULL");
			return;
		}
		if (body->body == nullptr) {
			LOG("TIRE ERROR: body->body == NULL");
			return;
		}



		physicsM->UpdateTireFriction(maxLateralImpulse, body->body);
		physicsM->UpdateTire(maxForwardSpeed, maxBackwardSpeed, speed, maxDriveForce, body->body);
		physicsM->UpdateTireTurn(direction, maxSteeringAngle, steeringAngle, body->body);

		int x, y;
		body->GetPhysicPosition(x, y);
		float rotation = body->GetRotation() * RAD2DEG;
		Rectangle src = { 0, 0, texture.width, texture.height };
		Rectangle dest = {
			(float)x,
			(float)y,
			(float)body->width * 2,
			(float)body->height * 2
		};
		Vector2 origin = {
			(float)body->width,
			(float)body->height
		};
		
		renderer->Draw(texture, x, y, &src, rotation, body->width, body->height, 0.2f);
		//DrawTexturePro(texture, src, dest, origin, rotation, WHITE);
	}

private:
	Texture2D texture;
	ModulePhysics* physicsM;
};

class Car : public PhysicEntity
{
public:

	Tire* frontLeft;
	Tire* frontRight;
	Tire* rearLeft;
	Tire* rearRight;
	bool isplayer;
	ModuleRender* renderer;
	ModulePhysics* phys;

	int currentWaypoint = 0;
	int currentLap = 0;
	float waypointRadius = 100.0f;

	Timer raceTimer;                 
	Timer lapTimer;                  
	std::vector<double> lapTimes;

	bool raceFinished = false;

	int id;

	Vector2 waypointOffset = { 0.0f, 0.0f };
	float maxOffset = 2.0f;

	Car(ModuleRender* render, ModulePhysics* physics, int x, int y, int w, int h, int dir,
		Tire* fl, Tire* fr, Tire* rl, Tire* rr,
		Module* listener, Texture2D tex, bool isPlayer, int _id)
		: PhysicEntity(physics->CreateCar(x, y, w, h, dir,
			fl->body->body, fr->body->body, rl->body->body, rr->body->body), listener)
		, texture(tex)
		, frontLeft(fl), frontRight(fr), rearLeft(rl), rearRight(rr)
	{
		isplayer = isPlayer;
		renderer = render;
		id = _id;
		phys = physics;
	}

	~Car() override
	{
		Tire* tires[4] = { frontLeft, frontRight, rearLeft, rearRight };

		for (Tire* tire : tires)
		{
			if (!tire) continue;

			phys->DeleteBody(tire->body);
			delete tire;
		}

		frontLeft = frontRight = rearLeft = rearRight = nullptr;

		
		if (body)
		{
			phys->DeleteBody(body);
			body = nullptr;
		}
	}

	void Update() override
	{
		if (body == nullptr) LOG("CAR ERROR: body == NULL");
		if (body->body == nullptr) LOG("CAR ERROR: body->body == NULL");

		
		frontLeft->Update();
		frontRight->Update();
		rearLeft->Update();
		rearRight->Update();


		int x, y;
		body->GetPhysicPosition(x, y);
		float rotation = body->GetRotation() * RAD2DEG;
		Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
		Rectangle dest = {
			(float)x,
			(float)y,
			(float)body->width * 2,
			(float)body->height * 2
		};
		Vector2 origin = {
			(float)body->width - 15,
			(float)body->height
		};
		
		renderer->Draw(texture, x, y, &src, rotation, origin.x, origin.y, 0.2f);
		//DrawTexturePro(texture, src, dest, origin, rotation, WHITE);

	}

	void GoToWaypoint(const Waypoint& wp)
	{
		int x, y;
		body->GetPhysicPosition(x, y);

		float targetX = wp.x + waypointOffset.x;
		float targetY = wp.y + waypointOffset.y;

		float dx = targetX - x;
		float dy = targetY - y;

		float distance = sqrtf(dx * dx + dy * dy);

		if (distance > 0.0f)
		{
			dx /= distance;
			dy /= distance;
		}

		frontLeft->speed.y = frontLeft->maxForwardSpeed;
		frontRight->speed.y = frontRight->maxForwardSpeed;

		float carAngle = body->GetRotation();
		float targetAngle = atan2f(dy, dx) + b2_pi / 2.0f;

		float angleDiff = targetAngle - carAngle;
		while (angleDiff > b2_pi) angleDiff -= 2 * b2_pi;
		while (angleDiff < -b2_pi) angleDiff += 2 * b2_pi;

		if (angleDiff > 0.15f)
		{
			frontLeft->direction = -1;
			frontRight->direction = -1;
		}
		else if (angleDiff < -0.15f)
		{
			frontLeft->direction = 1;
			frontRight->direction = 1;
		}
		else
		{
			frontLeft->direction = 0;
			frontRight->direction = 0;
		}
	}

	bool HasReachedWaypoint(const Waypoint& wp)
	{
		int x, y;
		body->GetPhysicPosition(x, y);

		float dx = wp.x - x;
		float dy = wp.y - y;

		return (dx * dx + dy * dy) <= waypointRadius * waypointRadius;
	}

	void Car::UpdateWaypointProgress()
	{
		if (currentWaypoint >= waypoints.size()) {
			currentWaypoint = 0;
		}
		Waypoint& wp = waypoints[currentWaypoint];

		if (HasReachedWaypoint(wp))
		{
			currentWaypoint++;

			waypointOffset.x = GetRandomValue(-maxOffset, maxOffset);
			waypointOffset.y = GetRandomValue(-maxOffset, maxOffset);

			if (currentWaypoint >= waypoints.size()) {
				currentWaypoint = 0;
				currentLap++;
				if (id == 0) {
					phys->App->audio->PlayFx(phys->App->scene_intro->lap_fx);
					double lapTime = lapTimer.ReadSec();
					lapTimes.push_back(lapTime);
					lapTimer.Start();
					if (currentLap == 3) {
						raceFinished = true;
					}
				}
			}
		}
	}
private:
	Texture2D texture;

};

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	ray_on = false;
	sensed = false;
}

ModuleGame::~ModuleGame()
{
}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	

	circle = LoadTexture("Assets/wheel.png");
	box = LoadTexture("Assets/crate.png");
	rick = LoadTexture("Assets/rick_head.png");
	map = LoadTexture("Assets/Map1/Map.png");
	wheel = LoadTexture("Assets/Rueda.png");
	carT = LoadTexture("Assets/Car.png");

	bonus_fx = App->audio->LoadFx("Assets/bonus.wav");
	lap_fx = App->audio->LoadFx("Assets/Audio/SFX/f1.wav");
	accelerate_fx = App->audio->LoadFx("Assets/Audio/SFX/car.wav");

	App->state->ChangeState(GameState::MENU_MAIN);

	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

void  ModuleGame::CreateCar(int x, int y, int w, int h, float scale, int dir, bool playable, int id) {
	Tire* fl = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, wheel);
	Tire* fr = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, wheel);
	Tire* rl = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, wheel);
	Tire* rr = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, wheel);

	Car* car = new Car(App->renderer,App->physics, x, y, w * scale, h * scale, dir, fl, fr, rl, rr, this, carT, playable, id);
	entities.emplace_back(car);

	if (car->isplayer)
	{
		car->raceTimer.Start();
		car->lapTimer.Start();
		car->lapTimes.clear();
		car->raceFinished = false;
	}
}

void ModuleGame::CreateRace(int x, int y, int w, int h, float scale, int dir) {

	for (int i = 0; i < 10; ++i)
	{
		int ox = (i / 2) * METERS_TO_PIXELS(3);
		int oy = (i % 2) * METERS_TO_PIXELS(2);

		CreateCar(x + ox, y + oy, w, h, scale, dir, i == 0, i);
	}

	onMenu = false;
	onRace = true;
}

// Update: draw background
update_status ModuleGame::Update()
{

	if (IsKeyPressed(KEY_ONE)) {

		debug = !debug;
	}

	if (onRace || onMenu) {
		if (onRace) {
			UpdateLeaderboard();
		}

		App->renderer->Draw(map,0,0,0,0,0,0,4);
		if (debug) {
			DrawWaypointsDebug();
		}
		// Activar rayo con espacio
		if (IsKeyPressed(KEY_SPACE))
		{
			ray_on = !ray_on;
			ray.x = GetMouseX();
			ray.y = GetMouseY();
		}
		// Crear entidades
		
			

		if (IsKeyPressed(KEY_TWO)) {
			CreateCar(GetMouseX() - App->renderer->camera.x, GetMouseY() - App->renderer->camera.y, 50, 100, 1.0f, 0, false,-1);
			printf("Coloco coche en: %f, %f\n", GetMouseX() - App->renderer->camera.x, GetMouseY() - App->renderer->camera.y);
		}
		if (IsKeyPressed(KEY_FOUR)) {

			CreateRace(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 50, 100, 1.0f, 3);

		}

		// Procesar input para cada coche
		for (PhysicEntity* entity : entities)
		{
			Car* car = dynamic_cast<Car*>(entity);
			if (!car) continue;

			if (car->isplayer)
			{

				int x, y; 
				car->body->GetPhysicPosition(x, y);
				App->renderer->camera.x = SCREEN_WIDTH / 2 - x; 
				App->renderer->camera.y = SCREEN_HEIGHT / 2 - y;

				if (IsKeyDown(KEY_W)) {
					car->frontLeft->speed.y = car->frontLeft->maxForwardSpeed;
					car->frontRight->speed.y = car->frontRight->maxForwardSpeed;
					App->audio->PlayFx(accelerate_fx);
				}
				else if (IsKeyDown(KEY_S)) {
					car->frontLeft->speed.y = -car->frontLeft->maxBackwardSpeed;
					car->frontRight->speed.y = -car->frontRight->maxBackwardSpeed;
				}
				else {
					car->frontLeft->speed.y = 0;
					car->frontRight->speed.y = 0;
				}
				if (IsKeyDown(KEY_SPACE)) {
					car->frontLeft->maxDriveForce = 1.0f;
					car->frontRight->maxDriveForce = 1.0f;
				}
				else {
					car->frontLeft->maxDriveForce = 1.0f * 0.5f;
					car->frontRight->maxDriveForce = 1.0f * 0.5f;
				}
				if (IsKeyDown(KEY_A)) {
					car->frontLeft->direction = 1;
					car->frontRight->direction = 1;
				}
				else if (IsKeyDown(KEY_D)) {
					car->frontLeft->direction = -1;
					car->frontRight->direction = -1;
				}
				else {
					car->frontLeft->direction = 0;
					car->frontRight->direction = 0;
				}
				if (car->raceFinished) {
					results.finalLeaderboard = leaderboard;  
					results.lapTimes = car->lapTimes;
					results.totalTime = car->raceTimer.ReadSec();
					App->state->ChangeState(GameState::RESULTS);
				}
			}
			else
			{
				if (onMenu) {
					int x, y;
					car->body->GetPhysicPosition(x, y);
					App->renderer->camera.x = SCREEN_WIDTH / 2 - x;
					App->renderer->camera.y = SCREEN_HEIGHT / 2 - y;
				}
				car->GoToWaypoint(waypoints[car->currentWaypoint]);
			}

			
			car->UpdateWaypointProgress();

			
			car->Update();
		}
	}

	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	for (PhysicEntity* entity : entities)
	{
		Car* car = dynamic_cast<Car*>(entity);
		if (!car) continue;

		if (bodyA == car->body || bodyB == car->body)
		{
			PhysBody* other = (bodyA == car->body) ? bodyB : bodyA;

			for (int i = 0; i < waypoints.size(); ++i)
			{
				if (waypoints[i].sensor == other &&
					car->currentWaypoint == i)
				{
					car->currentWaypoint++;

					if (car->currentWaypoint >= waypoints.size())
					{
						car->currentWaypoint = 0;
						car->currentLap++;
					}
				}
			}
		}
	}

}


void ModuleGame::CreateMockUpCar() {
	CreateCar(1654, 2387, 50, 100, 1.0f, 3, false,-1);
}

void ModuleGame::DestroyMockUpCar() {

	for (auto it = entities.begin(); it != entities.end(); )
	{
		Car* car = dynamic_cast<Car*>(*it);
		if (car && car->id == -1)
		{
			delete car;              
			it = entities.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void ModuleGame::UpdateLeaderboard()
{
	struct CarProgress
	{
		int id;
		int waypoint;
		int lap;
		float distToNext;
	};

	std::vector<CarProgress> progress;

	for (PhysicEntity* entity : entities)
	{
		Car* car = dynamic_cast<Car*>(entity);
		if (!car) continue;

		int x, y;
		car->body->GetPhysicPosition(x, y);

		Waypoint& wp = waypoints[car->currentWaypoint];
		float dx = wp.x - x;
		float dy = wp.y - y;
		float dist = sqrtf(dx * dx + dy * dy);

		progress.push_back({
			car->id,
			car->currentWaypoint,
			car->currentLap,
			dist
			});
	}

	//We sort using current lap first
	std::sort(progress.begin(), progress.end(),[](const CarProgress& a, const CarProgress& b) 
		{
			if (a.lap != b.lap) {
				return a.lap > b.lap;
			}

			if (a.waypoint != b.waypoint) {
				return a.waypoint > b.waypoint;
			}
			
			return a.distToNext < b.distToNext;
		});

	leaderboard.clear();
	for (auto& p : progress)
		leaderboard.push_back(p.id);
}

void ModuleGame::DrawWaypointsDebug()
{
	
	for (int i = 0; i < waypoints.size(); ++i)
	{
		/*Color c = (i == 0) ? GREEN : ORANGE;*/

		/*DrawCircle(
			waypoints[i].x + App->renderer->camera.x,
			waypoints[i].y + App->renderer->camera.y,
			100,
			c
		);

		
		DrawText(
			std::to_string(i).c_str(),
			waypoints[i].x + App->renderer->camera.x + 8,
			waypoints[i].y + App->renderer->camera.y - 8,
			14,
			WHITE
		);*/

		DrawRectanglePro(
			{
				waypoints[i].x + App->renderer->camera.x,
				waypoints[i].y + App->renderer->camera.y,
				(float)waypoints[i].w,
				(float)waypoints[i].h
			},
			{ waypoints[i].w / 2.0f, waypoints[i].h / 2.0f },
			waypoints[i].angle * RAD2DEG,
			Fade(RED, 0.3f)
		);
	}

	
	for (int i = 0; i < waypoints.size() - 1; ++i)
	{
		DrawLine(
			waypoints[i].x + App->renderer->camera.x,
			waypoints[i].y + App->renderer->camera.y,
			waypoints[i + 1].x + App->renderer->camera.x,
			waypoints[i + 1].y + App->renderer->camera.y,
			Fade(WHITE, 0.4f)
		);
	}
}

void ModuleGame::DeleteRace() {
	for (auto it = entities.begin(); it != entities.end(); )
	{
		Car* car = dynamic_cast<Car*>(*it);
		if (car && car->id >= 0)
		{
			delete car;                 
			it = entities.erase(it);   
		}
		else
		{
			++it;
		}
	}
}

void ModuleGame::CreateMapBorders()
{
	// Map bounds (same for all maps)
	const int left = 830;
	const int right = 3300;
	const int top = 513;
	const int bottom = 2695;

	const int thickness = 20; // wall thickness

	int width = right - left;
	int height = bottom - top;

	// Top wall
	PhysBody* border1 = App->physics->CreateStaticRectangle(
		left + width / 2,
		top - thickness / 2,
		width,
		thickness
	);
	mapBodies.push_back(border1);
	// Bottom wall
	PhysBody* border2 = App->physics->CreateStaticRectangle(
		left + width / 2,
		bottom + thickness / 2,
		width,
		thickness
	);
	mapBodies.push_back(border2);
	// Left wall
	PhysBody* border3 = App->physics->CreateStaticRectangle(
		left - thickness / 2,
		top + height / 2,
		thickness,
		height
	);
	mapBodies.push_back(border3);
	// Right wall
	PhysBody* border4 = App->physics->CreateStaticRectangle(
		right + thickness / 2,
		top + height / 2,
		thickness,
		height
	);
	mapBodies.push_back(border4);
}

void ModuleGame::CreateMap(int mapId)
{
	
	CreateMapBorders();

	LoadWaypoints(mapId, true);

	for (PhysicEntity* entity : entities)
	{
		Car* car = dynamic_cast<Car*>(entity);
		if (car)
		{
			car->currentWaypoint = 0;
			car->currentLap = 0;
		}
	}
}

void ModuleGame::LoadWaypoints(int mapId, bool race)
{
	waypoints.clear();

	// Valores por defecto (por seguridad)
	int startX = 0;
	int startY = 0;
	int carW = 50;
	int carH = 100;
	float carScale = 1.0f;
	int carDir = 0;

	std::string mapName = "Assets/Map" + std::to_string(mapId) + "/Map.png";
	UnloadTexture(map);
	map = LoadTexture(mapName.c_str());
	std::string filename = "Assets/Map" + std::to_string(mapId) + "/Map.txt";
	std::ifstream file(filename);

	if (!file.is_open())
	{
		LOG("ERROR: Could not open map file %s", filename.c_str());
		return;
	}

	std::string type;
	while (file >> type)
	{
		if (type == "START")
		{
			file >> startX >> startY;
		}
		else if (type == "CAR")
		{
			file >> carW >> carH >> carScale >> carDir;
		}
		else if (type == "WP")
		{
			Waypoint wp;
			float angleDeg;

			file >> wp.x >> wp.y >> wp.w >> wp.h >> angleDeg;
			wp.angle = angleDeg * DEG2RAD;

			wp.sensor = App->physics->CreateWaypointSensor(
				wp.x, wp.y, wp.w, wp.h, wp.angle, this);

			waypoints.push_back(wp);
		}
	}

	file.close();

	if (race) {
		CreateRace(startX, startY, carW, carH, carScale, carDir);
	}
}

void ModuleGame::DeleteMap()
{
	// Borrar waypoints
	waypoints.clear();

	// Borrar cuerpos físicos del mapa (bordes)
	for (PhysBody* body : mapBodies)
	{
		App->physics->DeleteBody(body);
	}
	mapBodies.clear();

	LOG("Map deleted");
}

double ModuleGame::GetRaceTime() const
{
	for (PhysicEntity* entity : entities)
	{
		Car* car = dynamic_cast<Car*>(entity);
		if (car && car->isplayer)
			return car->raceTimer.ReadSec();
	}
	return 0.0;
}