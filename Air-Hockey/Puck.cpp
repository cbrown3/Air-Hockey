#include "Puck.h"



Puck::Puck(Mesh* a_mesh, Material* a_mat): GameEntity(a_mesh, a_mat)
{
	setDirection(1.0f, 0.0f, 1.0f);
	speed = 2.5f;
	velocity = direction * speed;
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

void Puck::CollisionDetection(Paddle* a_paddle)
{
	XMVECTOR paddlePos = XMLoadFloat3(&a_paddle->GetPosition());
	XMVECTOR puckPos = XMLoadFloat3(&entityPos);

	//distance between positions
	XMVECTOR diff = XMVectorSubtract(paddlePos, puckPos);
	XMVECTOR length = XMVector3Length(diff);
	
	float dist = 0.0f;

	XMStoreFloat(&dist, length);

	float radii = GetScale().x + a_paddle->GetScale().x;
	//if<radius1+radius2 they collide
	if (radii > dist)
	{
		XMVECTOR wallN = XMVector3Normalize(diff);
		XMVECTOR newDir = direction - 2 * wallN * XMVector3Dot(wallN, direction);
		XMFLOAT3 floatDir = XMFLOAT3();
		XMStoreFloat3(&floatDir, newDir);

		setDirection(floatDir.x, floatDir.y, floatDir.z);
		
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

	XMStoreFloat3(&puckDir, direction);

	//Checking for Wall Collision
	if (GetPosition().x > 4.0f)
	{
		setDirection(-1.0f, 0.0f, puckDir.z);
	}
	if (GetPosition().x < -4.0f)
	{
		setDirection(1.0f, 0.0f, puckDir.z);
	}
	if (GetPosition().z > 2.0f)
	{
		setDirection(puckDir.x, 0.0f, -1.0f);
	}
	if (GetPosition().z < -2.0f)
	{
		setDirection(puckDir.x, 0.0f, 1.0f);
	}

	velocity = direction * speed * dt;

	XMVECTOR puckPos = XMLoadFloat3(&entityPos);
	puckPos = XMVectorAdd(velocity, puckPos);
	XMStoreFloat3(&entityPos, puckPos);

	entityPos.y = -0.15f;

}
