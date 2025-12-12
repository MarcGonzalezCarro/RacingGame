#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#include "p2Point.h"

#include <math.h>

ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	world = NULL;
	mouse_joint = NULL;
	debug = true;
}

// Destructor
ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics 2D environment");

	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));
	world->SetContactListener(this);

	return true;
}

update_status ModulePhysics::PreUpdate()
{
	accumulator += App->deltaTime;

	const float timeStep = 1.0f / 60.0f;
	const int velocityIterations = 6;
	const int positionIterations = 2;

	while (accumulator >= timeStep)
	{
		world->Step(timeStep, velocityIterations, positionIterations);
		accumulator -= timeStep;
	}

	for (b2Contact* c = world->GetContactList(); c; c = c->GetNext())
	{
		if (c->GetFixtureA()->IsSensor() && c->IsTouching())
		{
			b2BodyUserData data1 = c->GetFixtureA()->GetBody()->GetUserData();
			b2BodyUserData data2 = c->GetFixtureA()->GetBody()->GetUserData();

			PhysBody* pb1 = (PhysBody*)data1.pointer;
			PhysBody* pb2 = (PhysBody*)data2.pointer;
			if (pb1 && pb2 && pb1->listener)
				pb1->listener->OnCollision(pb1, pb2);
		}
	}

	return UPDATE_CONTINUE;
}

PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.gravityScale = 0.0f;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = pbody->height = radius;

	return pbody;
}

PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius, b2Vec2 initialVelocity, float mass)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = mass / (b2_pi * radius * radius);

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = pbody->height = radius;
	b->SetLinearVelocity(initialVelocity);

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangle(int x, int y, int width, int height)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = (int)(width * 0.5f);
	pbody->height = (int)(height * 0.5f);

	return pbody;
}

PhysBody* ModulePhysics::CreateCar(int x, int y, int width, int height, b2Body* tire1, b2Body* tire2, b2Body* tire3, b2Body* tire4)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.gravityScale = 0.0f;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y)); // posición inicial del cuerpo

	b2Body* b = world->CreateBody(&body);

	pbody->body = b;
	pbody->width = width;
	pbody->height = height;

	b2Vec2 norm[8] = {
		{  0.15f,  0.10f },
		{  0.30f,  0.00f },
		{  0.28f, -0.25f },
		{  0.10f, -0.80f },
		{ -0.10f, -0.80f },
		{ -0.28f, -0.25f },
		{ -0.30f,  0.00f },
		{ -0.15f,  0.10f }
	};

	b2Vec2 vertices[8];
	for (int i = 0; i < 8; ++i)
	{
		vertices[i].x = PIXEL_TO_METERS(norm[i].x * width);
		vertices[i].y = PIXEL_TO_METERS(norm[i].y * height);
	}

	b2PolygonShape polygonShape;
	polygonShape.Set(vertices, 8);
	b2Fixture* fixture = b->CreateFixture(&polygonShape, 0.1f);

	b2RevoluteJoint* flJoint, * frJoint;

	//car class constructor
	b2RevoluteJointDef jointDef;
	jointDef.bodyA = b;
	jointDef.enableLimit = true;
	jointDef.lowerAngle = 0;
	jointDef.upperAngle = 0;
	jointDef.localAnchorB.SetZero();

	//BackLeft
	jointDef.bodyB = tire1;
	jointDef.localAnchorA.Set(PIXEL_TO_METERS(-width * 0.30f),PIXEL_TO_METERS(-height * 0.80f));
	jointDef.enableLimit = true;
	jointDef.lowerAngle = 0;
	jointDef.upperAngle = 0;
	world->CreateJoint(&jointDef);
	//BackRight
	jointDef.bodyB = tire2;
	jointDef.localAnchorA.Set(PIXEL_TO_METERS(width * 0.30f), PIXEL_TO_METERS(-height * 0.80f));
	jointDef.enableLimit = true;
	jointDef.lowerAngle = 0;
	jointDef.upperAngle = 0;
	world->CreateJoint(&jointDef);
	//FrontLeft
	jointDef.bodyB = tire3;
	jointDef.localAnchorA.Set(PIXEL_TO_METERS(-width * 0.30f),PIXEL_TO_METERS(height * 0.05f));
	jointDef.enableLimit = true;
	jointDef.lowerAngle = -0.5f;   // ≈ -30º
	jointDef.upperAngle = 0.5f;   // ≈ +30º
	world->CreateJoint(&jointDef);
	//FrontRight
	jointDef.bodyB = tire4;
	jointDef.localAnchorA.Set(PIXEL_TO_METERS(width * 0.30f),PIXEL_TO_METERS(height * 0.05f));
	jointDef.enableLimit = true;
	jointDef.lowerAngle = -0.5f;
	jointDef.upperAngle = 0.5f;
	world->CreateJoint(&jointDef);

	return pbody;
}

PhysBody* ModulePhysics::CreateTire(int x, int y, int width, int height)
{
	PhysBody* tire = new PhysBody();

    b2BodyDef body;
    body.type = b2_dynamicBody;
	body.gravityScale = 0.0f;
    body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
    body.userData.pointer = reinterpret_cast<uintptr_t>(tire);

    b2Body* b = world->CreateBody(&body);

    b2PolygonShape box;
    box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

    b2FixtureDef fixture;
    fixture.shape = &box;
    fixture.density = 1.0f;

    b->CreateFixture(&fixture);

    tire->body = b;
    tire->width = width / 2;
    tire->height = height / 2;

    return tire;
}

void ModulePhysics::UpdateTire(float maxForwardSpeed, float maxBackwardSpeed, Vector2 currentForwardVelocity, float maxDriveForce, b2Body* body) {
	float desiredSpeed = currentForwardVelocity.y;

	//find current speed in forward direction
	b2Vec2 currentForwardNormal = body->GetWorldVector(b2Vec2(0, -1));
	b2Vec2 forwardVelocity;
	forwardVelocity.x = currentForwardVelocity.x;
	forwardVelocity.y = currentForwardVelocity.y;

	float currentSpeed = b2Dot(forwardVelocity, currentForwardNormal);

	//apply necessary force
	float force = 0;
	if (desiredSpeed > currentSpeed)
		force = maxDriveForce;
	else if (desiredSpeed < currentSpeed)
		force = -maxDriveForce;
	else
		return;
	body->ApplyForce(force * currentForwardNormal, body->GetWorldCenter(), true);
}

void ModulePhysics::UpdateTireTurn(int direction, float maxSteerAngle, float steerAngle,b2Body* body) {

	for (b2JointEdge* edge = body->GetJointList(); edge; edge = edge->next)
	{
		b2RevoluteJoint* rev = dynamic_cast<b2RevoluteJoint*>(edge->joint);
		if (rev)
		{
			// asumimos que es la joint de dirección
			float lockAngle = maxSteerAngle;
			float turnSpeedPerSec = 30;
			float turnPerStep = turnSpeedPerSec / 60.0f;

			float desiredAngle = 0;
			switch (direction)
			{
			case -1: desiredAngle = lockAngle; break;
			case 1:  desiredAngle = -lockAngle; break;
			default: desiredAngle = 0; break;
			}

			float angleDiff = desiredAngle - steerAngle;
			angleDiff = b2Clamp(angleDiff, -turnPerStep, turnPerStep);
			steerAngle += angleDiff;

			rev->SetLimits(steerAngle, steerAngle);
		}
	}
}
void ModulePhysics::UpdateTireFriction(float maxLateralImpulse, b2Body* body) {

	b2Vec2 currentRightNormal = body->GetWorldVector(b2Vec2(1, 0));
	b2Vec2 currentForwardNormal = body->GetWorldVector(b2Vec2(0, -1));
	b2Vec2 lateralVelocity = b2Dot(currentRightNormal, body->GetLinearVelocity()) * currentRightNormal;
	b2Vec2 forwardVelocity = b2Dot(currentForwardNormal, body->GetLinearVelocity()) * currentForwardNormal;
	b2Vec2 impulse = body->GetMass() * -lateralVelocity;

	if (impulse.Length() > maxLateralImpulse)
		impulse *= maxLateralImpulse / impulse.Length();
	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

	//angular velocity
	body->ApplyAngularImpulse(0.1f * body->GetInertia() * -body->GetAngularVelocity(), true);

	//forward linear velocity
	float forwardSpeed = forwardVelocity.Normalize();
	float dragForceMagnitude = -0.01 * forwardSpeed;
	body->ApplyForce(dragForceMagnitude * forwardVelocity, body->GetWorldCenter(), true);
}

b2WheelJoint* ModulePhysics::CreateWheelJoint(
	PhysBody* carBody,
	PhysBody* wheelBody,
	const b2Vec2& axis
)
{
	b2WheelJointDef jointDef;
	jointDef.Initialize(
		carBody->body,
		wheelBody->body,
		wheelBody->body->GetPosition(),
		axis
	);

	// Configurar parámetros de suspensión/motor (opcional)
	jointDef.enableMotor = false;
	jointDef.motorSpeed = 0.0f;
	jointDef.maxMotorTorque = 500.0f;

	return (b2WheelJoint*)world->CreateJoint(&jointDef);
}

b2RevoluteJoint* ModulePhysics::CreateSteerJoint(PhysBody* car, PhysBody* wheel, b2Vec2 anchor, float lowerDeg, float upperDeg)
{
	b2RevoluteJointDef jointDef;
	jointDef.bodyA = car->body;
	jointDef.bodyB = wheel->body;
	jointDef.localAnchorA = car->body->GetLocalPoint(anchor);
	jointDef.localAnchorB = wheel->body->GetLocalPoint(anchor);
	jointDef.enableLimit = true;
	jointDef.lowerAngle = lowerDeg * DEG2RAD;
	jointDef.upperAngle = upperDeg * DEG2RAD;
	jointDef.enableMotor = false;

	return (b2RevoluteJoint*)world->CreateJoint(&jointDef);
}

PhysBody* ModulePhysics::CreateRectangleSensor(int x, int y, int width, int height)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.isSensor = true;

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = width;
	pbody->height = height;

	return pbody;
}

PhysBody* ModulePhysics::CreateChain(int x, int y, const int* points, int size)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2ChainShape shape;
	b2Vec2* p = new b2Vec2[size / 2];

	for (int i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}

	shape.CreateLoop(p, size / 2);

	b2FixtureDef fixture;
	fixture.shape = &shape;

	b->CreateFixture(&fixture);

	delete p;

	pbody->body = b;
	pbody->width = pbody->height = 0;

	return pbody;
}

void ModulePhysics::DeleteBody(PhysBody* body)
{
	world->DestroyBody(body->body);
}

// 
update_status ModulePhysics::PostUpdate()
{
	if (IsKeyPressed(KEY_F1))
	{
		debug = !debug;
	}

	if (!debug)
	{
		return UPDATE_CONTINUE;
	}

	// Bonus code: this will iterate all objects in the world and draw the circles
	// You need to provide your own macro to translate meters to pixels
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
		{
			switch (f->GetType())
			{
				// Draw circles ------------------------------------------------
			case b2Shape::e_circle:
			{
				b2CircleShape* shape = (b2CircleShape*)f->GetShape();
				b2Vec2 pos = f->GetBody()->GetPosition();

				DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(shape->m_radius), Color{ 0, 0, 0, 128 });
			}
			break;

			// Draw polygons ------------------------------------------------
			case b2Shape::e_polygon:
			{
				b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
				int32 count = polygonShape->m_count;
				b2Vec2 prev, v;

				for (int32 i = 0; i < count; ++i)
				{
					v = b->GetWorldPoint(polygonShape->m_vertices[i]);
					if (i > 0)
						DrawLine(METERS_TO_PIXELS(prev.x) + App->renderer->camera.x, METERS_TO_PIXELS(prev.y) + App->renderer->camera.y, METERS_TO_PIXELS(v.x) + App->renderer->camera.x, METERS_TO_PIXELS(v.y) + App->renderer->camera.y, RED);

					prev = v;
				}

				v = b->GetWorldPoint(polygonShape->m_vertices[0]);
				DrawLine(METERS_TO_PIXELS(prev.x) + App->renderer->camera.x, METERS_TO_PIXELS(prev.y) + App->renderer->camera.y, METERS_TO_PIXELS(v.x) + App->renderer->camera.x, METERS_TO_PIXELS(v.y) + App->renderer->camera.y, RED);
			}
			break;

			// Draw chains contour -------------------------------------------
			case b2Shape::e_chain:
			{
				b2ChainShape* shape = (b2ChainShape*)f->GetShape();
				b2Vec2 prev, v;

				for (int32 i = 0; i < shape->m_count; ++i)
				{
					v = b->GetWorldPoint(shape->m_vertices[i]);
					if (i > 0)
						DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);
					prev = v;
				}

				v = b->GetWorldPoint(shape->m_vertices[0]);
				DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);
			}
			break;

			// Draw a single segment(edge) ----------------------------------
			case b2Shape::e_edge:
			{
				b2EdgeShape* shape = (b2EdgeShape*)f->GetShape();
				b2Vec2 v1, v2;

				v1 = b->GetWorldPoint(shape->m_vertex0);
				v1 = b->GetWorldPoint(shape->m_vertex1);
				DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), BLUE);
			}
			break;
			}
		}
	}

	return UPDATE_CONTINUE;
}


// Called before quitting
bool ModulePhysics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world!
	delete world;

	return true;
}

void PhysBody::GetPhysicPosition(int& x, int& y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x);
	y = METERS_TO_PIXELS(pos.y);
}

float PhysBody::GetRotation() const
{
	return body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	const b2Fixture* fixture = body->GetFixtureList();

	while (fixture != NULL)
	{
		if (fixture->GetShape()->TestPoint(body->GetTransform(), p) == true)
			return true;
		fixture = fixture->GetNext();
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	int ret = -1;

	b2RayCastInput input;
	b2RayCastOutput output;

	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;

	const b2Fixture* fixture = body->GetFixtureList();

	while (fixture != NULL)
	{
		if (fixture->GetShape()->RayCast(&output, input, body->GetTransform(), 0) == true)
		{
			// do we want the normal ?

			float fx = (float)(x2 - x1);
			float fy = (float)(y2 - y1);
			float dist = sqrtf((fx * fx) + (fy * fy));

			normal_x = output.normal.x;
			normal_y = output.normal.y;

			return (int)(output.fraction * dist);
		}
		fixture = fixture->GetNext();
	}

	return ret;
}

void ModulePhysics::BeginContact(b2Contact* contact)
{
	b2BodyUserData dataA = contact->GetFixtureA()->GetBody()->GetUserData();
	b2BodyUserData dataB = contact->GetFixtureB()->GetBody()->GetUserData();

	PhysBody* physA = (PhysBody*)dataA.pointer;
	PhysBody* physB = (PhysBody*)dataB.pointer;

	if (physA && physA->listener != NULL)
		physA->listener->OnCollision(physA, physB);

	if (physB && physB->listener != NULL)
		physB->listener->OnCollision(physB, physA);
}
