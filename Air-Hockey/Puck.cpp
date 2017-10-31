#include "Puck.h"



Puck::Puck(Mesh* a_mesh, Material* a_mat): GameEntity(a_mesh, a_mat)
{
	RandomVelocity();
	Update();
}

Puck::~Puck()
{
}

XMVECTOR Puck::getDirecton()
{
	return direction;
}

XMVECTOR Puck::getVelocity()
{
	return velocity;
}

void Puck::setDirection(float x, float y, float z)
{
	direction = XMVectorSet(x, y, z, 0);
}

void Puck::setVelocity(float x, float y, float z)
{
	velocity = XMVectorSet(x, y, z, 0);
}

void Puck::CollisionDetection(Paddle a_paddle)
{
	XMVECTOR paddlePos = XMLoadFloat3(&a_paddle.GetPosition());
	XMVECTOR puckPos = XMLoadFloat3(&entityPos);

	//distance between positions
	//if<radius1+radius2 they collide

	/*
	Vect1 is the direction of the ball before hitting the wall
	Vect2 is after the wall
	WallN is the normal of the cylinder. paddlepos-puckpos normalized
	DOT is the dot product

	Vect2 = Vect1 - 2 * WallN * (WallN DOT Vect1)	
	*/

	
}


void Puck::Reset()
{
	SetPosition(0, 0, 0);
}

void Puck::Update()
{
	//check for wall collision and scoring
	//for walls if collides x=-x or z=-z

	XMVECTOR puckPos = XMLoadFloat3(&entityPos);
	puckPos = XMVectorAdd(velocity, puckPos);
	XMStoreFloat3(&entityPos, puckPos);
	direction = XMVector3Normalize(velocity);


	//Checking for Wall Collision
	if (GetPosition().x > 2.5)
	{
		Reset();
	}
	if (GetPosition().x < -2.5)
	{
		Reset();
	}
}
