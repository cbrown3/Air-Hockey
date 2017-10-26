#pragma once
#include "GameEntity.h"


class Paddle :
	public GameEntity
{
public:
	Paddle(Mesh* mesh, Material* mat);
	~Paddle();

	int getRadius();
	

private:
	int radius;


};