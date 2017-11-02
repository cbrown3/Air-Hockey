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
	void CollisionDetection(Paddle* a_paddle);
	void Reset();
<<<<<<< HEAD
	void Update(float dt);

private:
	XMVECTOR direction;
	XMVECTOR velocity;
	XMFLOAT3 puckDir;
	float speed;
=======
	void RandomVelocity();
	void Update(float deltaTime, float totalTime);
	int checkScore();

private:
	XMFLOAT3 direction;
	XMFLOAT3 velocity;
>>>>>>> bb88f3488f498911cc6f031b6090caa53a73ad09
	int radius;

};

