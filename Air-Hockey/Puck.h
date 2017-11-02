#pragma once
#include "GameEntity.h"
#include "Paddle.h"

class Puck :
	public GameEntity
{
public:
	Puck(Mesh* a_mesh, Material* a_mat);
	~Puck();

	/*Gets and Sets*/
	XMVECTOR getDirecton();
	XMVECTOR getVelocity();
	void setDirection(float x, float y, float z);
	void setVelocity(float x, float y, float z);

	/*Other Methods*/
	void CollisionDetection(Paddle* a_paddle);
	void Reset();
	void Update(float dt);

private:
	XMVECTOR direction;
	XMVECTOR velocity;
	XMFLOAT3 puckDir;
	float speed;
	int radius;

};

