#include "Puck.h"



Puck::Puck(Mesh* a_mesh, Material* a_mat): GameEntity(a_mesh, a_mat)
{
setDirection(1.0f, 0.0f, 1.0f);
speed = 3.0f;
velocity = XMFLOAT3(direction.x * speed, 0, direction.z*speed);
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

void Puck::setDirection(XMVECTOR v)
{
	XMStoreFloat3(&direction, v);
}

void Puck::setVelocity(float x, float y, float z)
{
	velocity = XMFLOAT3(x, y, z);
}

void Puck::CollisionDetection(Paddle* a_paddle)
{
	/*
	Vect1 is the direction of the puck before hitting the wall
	Vect2 is after the wall
	WallN is the normal of the cylinder. paddlepos-puckpos normalized
	DOT is the dot product

	Vect2 = Vect1 - 2 * WallN * (WallN DOT Vect1)
	*/
	XMVECTOR paddlePos = XMLoadFloat3(&a_paddle->GetPosition());
	XMVECTOR puckPos = XMLoadFloat3(&entityPos);
	//distance between positions
	XMVECTOR diff = XMVectorSubtract(puckPos, paddlePos);
	XMVECTOR length = XMVector3Length(diff);
	XMFLOAT3 distVec = XMFLOAT3();
	XMStoreFloat3(&distVec, diff);

	float dist = 0.0f;

	XMStoreFloat(&dist, length);

	float radii = (GetScale().x + a_paddle->GetScale().x)*0.55f;

	//if<radius1+radius2 they collide
	if (radii > dist)
	{

		XMVECTOR puckDir = XMLoadFloat3(&direction);
		XMVECTOR wallN = XMVector3Normalize(diff);
		XMVECTOR newDir = puckDir - 2 * wallN * XMVector3Dot(wallN, puckDir);
		
		//XMVector3Normalize(newDir);
		
		setDirection(XMVectorSetY(newDir, 0.0f));
		//need to set position to be away from paddle
		SetPosition(GetPosition().x + distVec.x*0.4f, GetPosition().y, GetPosition().z + distVec.z*0.4f);
	}
}

void Puck::Reset()
{
	SetPosition(0, -0.15f, 0);
}

void Puck::Update(float dt)
{
	//check for wall collision and scoring
	//for walls if collides x=-x or z=-z



	//Checking for Wall Collision
	if (GetPosition().x > 4.0f)
	{
		setDirection(-direction.x, 0.0f, direction.z);
		SetPosition(4.0f, -0.15f, GetPosition().z);
	}
	if (GetPosition().x < -4.0f)
	{
		setDirection(-direction.x, 0.0f, direction.z);
		SetPosition(-4.0f, -0.15f, GetPosition().z);
	}
	if (GetPosition().z > 2.0f)
	{
		setDirection(direction.x, 0.0f, -direction.z);
		SetPosition(GetPosition().x, -0.15f, 2.0f);
	}
	if (GetPosition().z < -2.0f)
	{
		setDirection(direction.x, 0.0f, -direction.z);
		SetPosition(GetPosition().x, -0.15f, -2.0f);
	}
	
	XMVECTOR puckDir = XMLoadFloat3(&direction);



	puckDir = XMVector3Normalize(puckDir);

	XMVECTOR puckVel = puckDir* speed * dt;

	XMVECTOR puckPos = XMLoadFloat3(&entityPos);
	puckPos = XMVectorAdd(puckVel, puckPos);
	XMStoreFloat3(&entityPos, puckPos);
	XMStoreFloat3(&velocity, puckVel);
	XMStoreFloat3(&direction, puckDir);
	entityPos.y = -0.15f;
	std::cout << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
}

int Puck::checkScore()
{
	if (GetPosition().z < 1.0f && GetPosition().z > -1.0f)
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
