#include "Emitter.h"



Emitter::Emitter()
{
}

Emitter::Emitter(Material* a_mat, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 vel, int maxParticles, int emissionRate, bool loopable, bool active)
{
	mat = a_mat;
	position = pos;
	velocity = vel;
	max = maxParticles;
	particlesPerSecond = emissionRate;
	loop = loopable;
	this->active = active;
}


Emitter::~Emitter()
{
}

void Emitter::Activate()
{
	active = true;
}

void Emitter::Deactivate()
{
	active = false;
}

void Emitter::Update(float dt)
{
}

void Emitter::UpdateSingleParticle(float dt, int index)
{
}

void Emitter::SpawnParticle()
{
}

void Emitter::CopyParticlesToGPU(ID3D11DeviceContext * context)
{
}

void Emitter::CopyOneParticle(int index)
{
}

void Emitter::Draw(ID3D11DeviceContext * context, Camera * camera)
{
}
