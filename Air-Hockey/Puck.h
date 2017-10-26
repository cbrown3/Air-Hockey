#pragma once
#include "GameEntity.h"
class Puck :
	public GameEntity
{
public:
	Puck();
	~Puck();

	/*Gets and Sets*/
	XMVECTOR getDirecton();
	XMVECTOR getVelocity();
	void setDirection(float x, float y, float z);
	void setVelocity(float x, float y, float z);

	/*Other Methods*/
	void CollisionDetection(/*Paddle a_paddle*/);
	void Reset();
	void RandomVelocity();
	void Update();

private:
	XMVECTOR direction;
	XMVECTOR velocity;

};

