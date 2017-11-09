#pragma once
#include "GameEntity.h"
#include "Paddle.h"
#include <iostream>

class Puck :
	public GameEntity
{
public:
	Puck(Mesh* a_mesh, Material* a_mat);
	~Puck();

	/*Gets and Sets*/
	XMFLOAT3 getDirecton();
	XMFLOAT3 getVelocity();
	void setDirection(float x, float y, float z);
	void setDirection(XMVECTOR v);
	void setVelocity(float x, float y, float z);

	/*Other Methods*/
	void CollisionDetection(Paddle* a_paddle);
	void Reset();
	void Update(float dt);
	int checkScore();

private:
	XMFLOAT3 direction;
	XMFLOAT3 velocity;
	float speed;

};

