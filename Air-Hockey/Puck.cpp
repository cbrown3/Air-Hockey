#include "Puck.h"



Puck::Puck(Mesh* a_mesh, Material* a_mat): GameEntity(a_mesh, a_mat)
{
<<<<<<< HEAD
	setDirection(1.0f, 0.0f, 1.0f);
	speed = 2.5f;
	velocity = direction * speed;
=======
	RandomVelocity();
	//Update();
>>>>>>> bb88f3488f498911cc6f031b6090caa53a73ad09
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

void Puck::CollisionDetection(Paddle* a_paddle)
{
	XMVECTOR paddlePos = XMLoadFloat3(&a_paddle->GetPosition());
	XMVECTOR puckPos = XMLoadFloat3(&entityPos);
	XMVECTOR vel = XMLoadFloat3(&velocity);
	
	XMVECTOR dist = puckPos - paddlePos;
	float distance = XMVectorGetX(XMVector3Length(dist)); //sqrtf(XMVector3Dot(dist, dist));

	dist = XMVector3Normalize(dist);
	//distance between positions
<<<<<<< HEAD
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
=======
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
>>>>>>> bb88f3488f498911cc6f031b6090caa53a73ad09
}

void Puck::Reset()
{
<<<<<<< HEAD
	SetPosition(0, -0.15f, 0);
}

void Puck::Update(float dt)
=======
	RandomVelocity();
	SetPosition(0, -0.1f, 0);
}

void Puck::Update(float deltaTime, float totalTime)
>>>>>>> bb88f3488f498911cc6f031b6090caa53a73ad09
{
	//check for wall collision and scoring
	//for walls if collides x=-x or z=-z

<<<<<<< HEAD
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

=======
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
>>>>>>> bb88f3488f498911cc6f031b6090caa53a73ad09
}
