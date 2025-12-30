#pragma once

#include "Module.h"
#include "Globals.h"

#include "box2d\box2d.h"

#define GRAVITY_X 0.0f
#define GRAVITY_Y -7.0f

#define PIXELS_PER_METER 50.0f // if touched change METER_PER_PIXEL too
#define METER_PER_PIXEL 0.02f // this is 1 / PIXELS_PER_METER !

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

// Small class to return to other modules to track position and rotation of physics bodies
class PhysBody
{
public:
	PhysBody() : listener(NULL), body(NULL)
	{
	}

	//void GetPosition(int& x, int& y) const;
	void GetPhysicPosition(int& x, int& y) const;
	float GetRotation() const;
	bool Contains(int x, int y) const;
	int RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const;

public:
	int width, height;
	b2Body* body;
	Module* listener;
};

// Module --------------------------------------
class ModulePhysics : public Module, public b2ContactListener
{
public:
	ModulePhysics(Application* app, bool start_enabled = true);
	~ModulePhysics();

	bool Start();
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

	PhysBody* CreateCircle(int x, int y, int radius);
	PhysBody* CreateCircle(int x, int y, int radius, b2Vec2 initialVelocity, float mass);
	PhysBody* CreateRectangle(int x, int y, int width, int height);
	PhysBody* CreateCar(int x, int y, int width, int height, int dir, b2Body* tire1, b2Body* tire2, b2Body* tire3, b2Body* tire4);
	PhysBody* CreateTire(int x, int y, int width, int height, int dir);
	void UpdateTire(float maxForwardSpeed, float maxBackwardSpeed, Vector2 currentForwardVelocity, float maxDriveForce, b2Body* body);

	void UpdateTireTurn(int direction, float maxSteerAngle, float steerAngle, b2Body* body);

	void UpdateTireFriction(float maxLateralImpulse, b2Body* body);

	b2WheelJoint* CreateWheelJoint(PhysBody* carBody, PhysBody* wheelBody, const b2Vec2& axis);
	b2RevoluteJoint* CreateSteerJoint(PhysBody* car, PhysBody* wheel, b2Vec2 anchor, float lowerDeg, float upperDeg);
	PhysBody* CreateRectangleSensor(int x, int y, int width, int height);
	PhysBody* CreateStaticRectangle(int x, int y, int width, int height);
	PhysBody* CreateWaypointSensor(int x, int y, int w, int h, float angleRad, Module* listener);
	PhysBody* CreateChain(int x, int y, const int* points, int size);
	void DeleteBody(PhysBody* body);

	// b2ContactListener ---
	void BeginContact(b2Contact* contact);

	void BeginMouseDrag(int mouseX, int mouseY);

	void UpdateMouseDrag(int mouseX, int mouseY);

	void EndMouseDrag();
	void DrawMouseJointDebug();
private:
	float accumulator = 0.0f;
	bool debug;
	b2World* world;
	b2MouseJoint* mouse_joint;
	b2Body* ground;
};


