#include "Puck.h"



Puck::Puck()
{
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

void Puck::CollisionDetection(/*Paddle a_paddle*/)
{
	
}

void Puck::Reset()
{
	SetPosition(0, 2, 0);
}

void Puck::RandomVelocity()
{
	float randNum = rand() % 10 + 1;
	velocity = XMVectorSet(randNum, 0, randNum, 0);
}

void Puck::Update()
{
	direction = XMVector3Normalize(velocity);
}
