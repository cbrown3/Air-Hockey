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
	XMFLOAT3 getDirecton();
	XMFLOAT3 getVelocity();
	void setDirection(float x, float y, float z);
	void setVelocity(float x, float y, float z);

	/*Other Methods*/
	void CollisionDetection(Paddle a_paddle);
	void Reset();
	void RandomVelocity();
	void Update(float deltaTime, float totalTime);
	int checkScore();

private:
	XMFLOAT3 direction;
	XMFLOAT3 velocity;
	int radius;

};

