#include "Puck.h"



Puck::Puck(Mesh* a_mesh, Material* a_mat): GameEntity(a_mesh, a_mat)
{
	RandomVelocity();
	//Update();
}

Puck::~Puck()
{
}

XMFLOAT3 Puck::getDirecton()
{
	return direction;
}

XMFLOAT3 Puck::getVelocity()
{
	return velocity;
}

void Puck::setDirection(float x, float y, float z)
{
	direction = XMFLOAT3(x, y, z);
}

void Puck::setVelocity(float x, float y, float z)
{
	velocity = XMFLOAT3(x, y, z);
}

void Puck::CollisionDetection(Paddle a_paddle)
{
	XMVECTOR paddlePos = XMLoadFloat3(&a_paddle.GetPosition());
	XMVECTOR puckPos = XMLoadFloat3(&entityPos);
	XMVECTOR vel = XMLoadFloat3(&velocity);
	
	XMVECTOR dist = puckPos - paddlePos;
	float distance = XMVectorGetX(XMVector3Length(dist)); //sqrtf(XMVector3Dot(dist, dist));

	dist = XMVector3Normalize(dist);
	//distance between positions
	//if<radius1+radius2 they collide
	if (distance < 0.5f)
	{
		/*
		Vect1 is the direction of the puck before hitting the wall
		Vect2 is after the wall
		WallN is the normal of the cylinder. paddlepos-puckpos normalized
		DOT is the dot product

		Vect2 = Vect1 - 2 * WallN * (WallN DOT Vect1)
		*/
		XMVECTOR tempVel = (vel - 2 * dist *XMVector3Dot(dist, vel));
		XMStoreFloat3(&velocity, tempVel);
		velocity.y = 0;
	}
	
}

void Puck::RandomVelocity()
{
	float randNum = rand() % 1;
	//float randNum2 = rand() % 2000 + 1;
	velocity = XMFLOAT3(2, 0, 0); //randNum, 0, randNum);
}

void Puck::Reset()
{
	RandomVelocity();
	SetPosition(0, -0.1f, 0);
}

void Puck::Update(float deltaTime, float totalTime)
{
	//check for wall collision and scoring
	//for walls if collides x=-x or z=-z

	XMVECTOR puckPos = XMLoadFloat3(&entityPos);
	XMVECTOR vel = XMLoadFloat3(&velocity);
	puckPos = XMVectorAdd(vel*deltaTime, puckPos);
	XMVectorSetY(puckPos, -0.1f);
	XMStoreFloat3(&entityPos, puckPos);
	XMStoreFloat3(&direction, XMVector3Normalize(vel));


	//Checking for Wall Collision
	if (GetPosition().x > 4 || GetPosition().x < -4)
	{
		setVelocity(-velocity.x, 0.0f, velocity.z);
	}
	if (GetPosition().z > 2 || GetPosition().z < -2)
	{
		setVelocity(velocity.x, 0.0f, -velocity.z);
	}
}

int Puck::checkScore()
{
	if (GetPosition().z < 0.5f && GetPosition().z > -0.5f)
	{
		if (GetPosition().x > 3.99f) {
			Reset();
			return 1;
		}
		if (GetPosition().x < -3.99f) {
			Reset();
			return 2;
		}
	}

	return 0;
}
