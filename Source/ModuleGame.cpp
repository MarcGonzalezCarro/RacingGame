#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

struct Waypoint
{
	int x;
	int y;
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

	float maxForwardSpeed = 0.25f * 0.5f;  
	float maxBackwardSpeed = 0.05f * 0.5f;
	float maxDriveForce = 1.0f * 0.5f;

	float steeringAngle = 0.0f;
	float maxSteeringAngle = 5.0f;
	float steeringSpeed = 0.05f;

	float maxLateralImpulse = 2.5f;

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

	int currentWaypoint = 0;
	float waypointRadius = 60.0f;

	Vector2 waypointOffset = { 0.0f, 0.0f };
	float maxOffset = 2.0f;

	Car(ModuleRender* render, ModulePhysics* physics, int x, int y, int w, int h, int dir,
		Tire* fl, Tire* fr, Tire* rl, Tire* rr,
		Module* listener, Texture2D tex, bool isPlayer)
		: PhysicEntity(physics->CreateCar(x, y, w, h, dir,
			fl->body->body, fr->body->body, rl->body->body, rr->body->body), listener)
		, texture(tex)
		, frontLeft(fl), frontRight(fr), rearLeft(rl), rearRight(rr)
	{
		isplayer = isPlayer;
		renderer = render;
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
			(float)body->width,
			(float)body->height
		};
		Vector2 origin = {
			(float)body->width / 2,
			(float)body->height
		};
		
		renderer->Draw(texture, x, y, &src, rotation, body->width / 2, body->height / 2, 0.2f);
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
	map = LoadTexture("Assets/MapaTemporal.png");

	bonus_fx = App->audio->LoadFx("Assets/bonus.wav");
	

	waypoints.push_back({ 203, 1740 });
	waypoints.push_back({ 159, 967 });
	waypoints.push_back({ 260, 730 });
	waypoints.push_back({ 158, 429 });
	waypoints.push_back({ 201, 195 });
	waypoints.push_back({ 1856, 176 });
	waypoints.push_back({ 1907, 466 });
	waypoints.push_back({ 856, 507 });
	waypoints.push_back({ 846, 796 });
	waypoints.push_back({ 1623, 862 });
	waypoints.push_back({ 1692, 1449 });
	waypoints.push_back({ 1922, 1649 });
	waypoints.push_back({ 1666, 1774 });
	


	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

void  ModuleGame::CreateCar(int x, int y, int w, int h, float scale, int dir, bool playable) {
	Tire* fl = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, box);
	Tire* fr = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, box);
	Tire* rl = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, box);
	Tire* rr = new Tire(App->renderer,App->physics, x, y, 10 * scale, 20 * scale, dir, this, box);

	Car* car = new Car(App->renderer,App->physics, x, y, w * scale, h * scale, dir, fl, fr, rl, rr, this, box, playable);
	entities.emplace_back(car);
}

void ModuleGame::CreateRace(int x, int y, int w, int h, float scale, int dir) {

	for (int i = 0; i < 10; ++i)
	{
		int ox = (i / 2) * METERS_TO_PIXELS(3);
		int oy = (i % 2) * METERS_TO_PIXELS(2);

		CreateCar(x + ox, y + oy, w, h, scale, dir, i == 0);
	}

	onRace = true;
}

// Update: draw background
update_status ModuleGame::Update()
{
	if (onRace) {

		DebugClickWaypoint();

		App->renderer->Draw(map,0,0,0,0,0,0,3);

		// Activar rayo con espacio
		if (IsKeyPressed(KEY_SPACE))
		{
			ray_on = !ray_on;
			ray.x = GetMouseX();
			ray.y = GetMouseY();
		}
		// Crear entidades
		if (IsKeyPressed(KEY_ONE))
			entities.emplace_back(new Tire(App->renderer, App->physics, GetMouseX(), GetMouseY(), 5, 10, 1, this, box));

		if (IsKeyPressed(KEY_TWO)) {
			CreateCar(GetMouseX() - App->renderer->camera.x, GetMouseY() - App->renderer->camera.y, 50, 100, 1.0f, 0, false);
			printf("Coloco coche en: %f, %f\n", GetMouseX() - App->renderer->camera.x, GetMouseY() - App->renderer->camera.y);
		}
		if (IsKeyPressed(KEY_THREE)) {

			CreateCar(0, 0, 50, 100, 0.5f, 0, false);

		}
		if (IsKeyPressed(KEY_FOUR)) {

			CreateRace(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 50, 100, 1.0f, 3);

		}

		// Procesar input para cada coche
		for (PhysicEntity* entity : entities)
		{
			Car* car = dynamic_cast<Car*>(entity);
			if (!car) continue;
			if (car->isplayer) {

				int x, y;
				car->body->GetPhysicPosition(x, y);



				// adelante
				if (IsKeyDown(KEY_W)) {
					car->frontLeft->speed.y = car->frontLeft->maxForwardSpeed;
					car->frontRight->speed.y = car->frontRight->maxForwardSpeed;
				}
				else if (IsKeyDown(KEY_S)) {
					car->frontLeft->speed.y = -car->frontLeft->maxBackwardSpeed;
					car->frontRight->speed.y = -car->frontRight->maxBackwardSpeed;
				}
				else {
					car->frontLeft->speed.y = 0;
					car->frontRight->speed.y = 0;
				}

				// giro
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

				App->renderer->camera.x = SCREEN_WIDTH / 2 - x;
				App->renderer->camera.y = SCREEN_HEIGHT / 2 - y;

				car->Update();

			}
			else {

				if (car->currentWaypoint < waypoints.size())
				{
					Waypoint& wp = waypoints[car->currentWaypoint];

					car->GoToWaypoint(wp);

					if (car->HasReachedWaypoint(wp))
					{
						car->currentWaypoint++;

						// Offset aleatorio para el siguiente punto
						car->waypointOffset.x = GetRandomValue(-car->maxOffset, car->maxOffset);
						car->waypointOffset.y = GetRandomValue(-car->maxOffset, car->maxOffset);

						if (car->currentWaypoint >= waypoints.size())
							car->currentWaypoint = 0;
					}
				}
				car->Update();
			}
		}
	}

	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	if (!bodyA || !bodyB)
	{
		return;
	}

	bool isCircle1 = false;
	bool isCircle2 = false;


	const b2Fixture* fixture = bodyA->body->GetFixtureList();
	while (fixture != NULL)
	{
		if (const b2CircleShape* circle = dynamic_cast<const b2CircleShape*>(fixture->GetShape()))
		{
			//sumradius += circle->m_radius;
			isCircle1 = true;
			break;
		}
		fixture = fixture->GetNext();
	}

	const b2Fixture* fixture2 = bodyB->body->GetFixtureList();
	while (fixture2 != NULL)
	{
		if (const b2CircleShape* circle = dynamic_cast<const b2CircleShape*>(fixture2->GetShape()))
		{
			//sumradius += circle->m_radius;
			isCircle2 = true;
			break;
		}
		fixture2 = fixture2->GetNext();
	}

	if (isCircle1 && isCircle2)
	{
		std::set<PhysicEntity*> colliding;
		//Delete both bodies
		for (PhysicEntity* entity : entities)
		{
			if (entity->body == bodyA || entity->body == bodyB)
			{
				colliding.emplace(entity);
			}
		}
		collidingEntities.emplace(colliding);


	}


}

void ModuleGame::DebugClickWaypoint()
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		// Mouse en pantalla
		int mouseX = GetMouseX();
		int mouseY = GetMouseY();

		// Convertir a coordenadas de mundo (IMPORTANTE)
		int worldX = mouseX - App->renderer->camera.x;
		int worldY = mouseY - App->renderer->camera.y;

		printf("Waypoint clicked -> X: %d, Y: %d\n", worldX, worldY);

		// Opcional: guardarlo directamente
		waypoints.push_back({ worldX, worldY });

		// Feedback visual
		DrawCircle(worldX, worldY, 6, RED);
	}
}