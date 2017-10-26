#include "Paddle.h"


Paddle::Paddle(Mesh* mesh, Material* mat) : GameEntity(mesh, mat)
{
	radius = 1;
}

Paddle::~Paddle()
{
}

int Paddle::getRadius()
{
	return radius;
}
