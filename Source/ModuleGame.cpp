#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

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

	Tire(ModuleRender* render,ModulePhysics* physics, int _x, int _y, int _w, int _h, Module* _listener, Texture2D _texture)
		: PhysicEntity(physics->CreateTire(_x, _y, _w, _h), _listener)
		, texture(_texture), physicsM(physics)
	{
		renderer = render;
	}

	float maxForwardSpeed = 5.0f;
	float maxBackwardSpeed = 2.0f;
	float maxDriveForce = 2.0f;

	float steeringAngle = 0.0f;
	float maxSteeringAngle = 15.0f;
	float steeringSpeed = 3.0f;

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

		renderer->Draw(texture, x, y, &src, rotation, texture.width / 2, texture.height / 2);
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

	Car(ModuleRender* render, ModulePhysics* physics, int x, int y, int w, int h,
		Tire* fl, Tire* fr, Tire* rl, Tire* rr,
		Module* listener, Texture2D tex, bool isPlayer)
		: PhysicEntity(physics->CreateCar(x, y, w, h,
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
			(float)body->height / 2
		};
		renderer->Draw(texture, x, y, &src, (double)rotation, texture.width / 2, texture.height / 2);
		//DrawTexturePro(texture, src, dest, origin, rotation, WHITE);

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

	bonus_fx = App->audio->LoadFx("Assets/bonus.wav");

	sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT, SCREEN_WIDTH, 50);

	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

void  ModuleGame::CreateCar(int x, int y, int w, int h, bool playable) {
	Tire* fl = new Tire(App->renderer,App->physics, x, y, 10, 20, this, box);
	Tire* fr = new Tire(App->renderer,App->physics, x, y, 10, 20, this, box);
	Tire* rl = new Tire(App->renderer,App->physics, x, y, 10, 20, this, box);
	Tire* rr = new Tire(App->renderer,App->physics, x, y, 10, 20, this, box);

	Car* car = new Car(App->renderer,App->physics, x, y, w, h, fl, fr, rl, rr, this, box, true);
	entities.emplace_back(car);
}

void ModuleGame::CreateRace() {

}

// Update: draw background
update_status ModuleGame::Update()
{
	// Activar rayo con espacio
	if (IsKeyPressed(KEY_SPACE))
	{
		ray_on = !ray_on;
		ray.x = GetMouseX();
		ray.y = GetMouseY();
	}
	// Crear entidades
	if (IsKeyPressed(KEY_ONE))
		entities.emplace_back(new Tire(App->renderer,App->physics, GetMouseX(), GetMouseY(), 5, 10, this, box));

	if (IsKeyPressed(KEY_TWO)) {
		if (IsKeyPressed(KEY_TWO))
		{
			CreateCar(SCREEN_WIDTH/2,SCREEN_HEIGHT/2,50,100,true);
		}
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
			
			/*App->renderer->camera.x = x;
			App->renderer->camera.y = y;*/
			
			car->Update();

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
