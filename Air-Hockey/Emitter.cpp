#include "Emitter.h"



Emitter::Emitter()
{
}

Emitter::Emitter(DirectX::XMFLOAT3 pos, 
	DirectX::XMFLOAT3 vel, 
	DirectX::XMFLOAT3 acc,
	DirectX::XMFLOAT4 startColor,
	DirectX::XMFLOAT4 endColor,
	float startSize,
	float endSize,
	int maxParticles,
	int emissionRate,
	bool loopable,
	bool active, 
	float lifetime,
	ID3D11Device * device,
	SimpleVertexShader * vs,
	SimplePixelShader * ps,
	ID3D11ShaderResourceView * texture)
{
	position = pos;
	velocity = vel;
	acceleration = acc;
	this->startColor = startColor;
	this->endColor = endColor;
	this->startSize = startSize;
	this->endSize = endSize;
	max = maxParticles;
	particlesPerSecond = emissionRate;
	secondsPerParticle = 1.0f/particlesPerSecond;
	loop = loopable;
	this->active = active;
	this->lifetime = lifetime;
	this->vs = vs;
	this->ps = ps;
	this->texture = texture;

	firstAliveIndex = 0;
	firstDeadIndex = 0;
	timeSinceEmit = 0;
	livingParticleCount = 0;
	//initialize arrays
	particles = new Particle[max];

	particleVertices = new ParticleVertex[4 * max];
	for (int i = 0; i < maxParticles * 4; i += 4) {
		particleVertices[i + 0].UV = XMFLOAT2(0, 0);
		particleVertices[i + 1].UV = XMFLOAT2(1, 0);
		particleVertices[i + 2].UV = XMFLOAT2(1, 1);
		particleVertices[i + 3].UV = XMFLOAT2(0, 1);
	}

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(ParticleVertex) * 4 * maxParticles;
	device->CreateBuffer(&vbDesc, 0, &vertexBuffer);

	unsigned int* indices = new unsigned int[maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4) {
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * max * 6;
	device->CreateBuffer(&ibDesc, &indexData, &indexBuffer);

	delete[]indices;
}

Emitter::Emitter(Material* a_mat, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 vel, int maxParticles, int emissionRate, bool loopable, bool active, ID3D11Device * device)
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
	delete[] particles;
	delete[] particleVertices;
	vertexBuffer->Release();
	indexBuffer->Release();
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
	if (livingParticleCount || active) {
		if (firstAliveIndex < firstDeadIndex)
		{
			// First alive is BEFORE first dead, so the "living" particles are contiguous
			// 
			// 0 -------- FIRST ALIVE ----------- FIRST DEAD -------- MAX
			// |    dead    |            alive       |         dead    |

			// First alive is before first dead, so no wrapping
			for (int i = firstAliveIndex; i < firstDeadIndex; i++)
				UpdateSingleParticle(dt, i);
		}
		else {
			// First alive is AFTER first dead, so the "living" particles wrap around
			// 
			// 0 -------- FIRST DEAD ----------- FIRST ALIVE -------- MAX
			// |    alive    |            dead       |         alive   |

			// Update first half (from firstAlive to max particles)
			for (int i = firstAliveIndex; i < max; i++)
				UpdateSingleParticle(dt, i);

			// Update second half (from 0 to first dead)
			for (int i = 0; i < firstDeadIndex; i++)
				UpdateSingleParticle(dt, i);
		}
		if (active) {
			timeSinceEmit += dt;
			while (timeSinceEmit > secondsPerParticle) {
				SpawnParticle();
				timeSinceEmit -= secondsPerParticle;
			}
		}
	}
}

void Emitter::UpdateSingleParticle(float dt, int index)
{
	if (particles[index].Age >= lifetime)
		return;
	particles[index].Age += dt;
	if (particles[index].Age >= lifetime) 
	{
		firstAliveIndex++;
		firstAliveIndex %= max;
		livingParticleCount--;
		return;
	}

	float agePercent = particles[index].Age / lifetime;
	//lerp color
	XMStoreFloat4(
		&particles[index].Color,
		XMVectorLerp(
			XMLoadFloat4(&startColor),
			XMLoadFloat4(&endColor),
			agePercent));
	/**/
	//lerp size
	particles[index].Size = startSize + agePercent * (endSize - startSize);

	//position
	XMVECTOR startPos = XMLoadFloat3(&position);
	XMVECTOR startVel = XMLoadFloat3(&particles[index].StartVel);
	XMVECTOR accel = XMLoadFloat3(&acceleration);
	float t = particles[index].Age;

	XMStoreFloat3(
		&particles[index].Position,
		accel * t * t / 2.0f + startVel * t + startPos);

}

void Emitter::SpawnParticle()
{
	if (livingParticleCount == max)
		return;

	particles[firstDeadIndex].Age = 0;
	particles[firstDeadIndex].Size = startSize;
	particles[firstDeadIndex].Color = startColor;
	particles[firstDeadIndex].Position = position;
	particles[firstDeadIndex].StartVel = velocity;
	particles[firstDeadIndex].StartVel.x += ((float)rand() / RAND_MAX) * 0.4f - 0.2f;
	particles[firstDeadIndex].StartVel.y += ((float)rand() / RAND_MAX) * 0.4f - 0.2f;
	particles[firstDeadIndex].StartVel.z += ((float)rand() / RAND_MAX) * 0.4f - 0.2f;

	firstDeadIndex++;
	firstDeadIndex %= max;

	livingParticleCount++;
}

void Emitter::CopyParticlesToGPU(ID3D11DeviceContext * context)
{
	if (firstAliveIndex < firstDeadIndex) {
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			CopyOneParticle(i);
	}
	else {
		for (int i = firstAliveIndex; i < max; i++)
			CopyOneParticle(i);
		for (int i = 0; i < firstDeadIndex; i++)
			CopyOneParticle(i);
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, particleVertices, sizeof(ParticleVertex) * 4 * max);

	context->Unmap(vertexBuffer, 0);
}

void Emitter::CopyOneParticle(int index)
{
	int i = index * 4;

	particleVertices[i + 0].Position = particles[index].Position;
	particleVertices[i + 1].Position = particles[index].Position;
	particleVertices[i + 2].Position = particles[index].Position;
	particleVertices[i + 3].Position = particles[index].Position;

	particleVertices[i + 0].Size = particles[index].Size;
	particleVertices[i + 1].Size = particles[index].Size;
	particleVertices[i + 2].Size = particles[index].Size;
	particleVertices[i + 3].Size = particles[index].Size;

	particleVertices[i + 0].Color = particles[index].Color;
	particleVertices[i + 1].Color = particles[index].Color;
	particleVertices[i + 2].Color = particles[index].Color;
	particleVertices[i + 3].Color = particles[index].Color;
}

void Emitter::Draw(ID3D11DeviceContext * context, Camera * camera)
{
	//if (active) {
		CopyParticlesToGPU(context);

		//set up buffers
		UINT stride = sizeof(ParticleVertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		vs->SetMatrix4x4("view", camera->getViewMatrix());
		vs->SetMatrix4x4("projection", camera->getProjMatrix());
		vs->SetShader();
		vs->CopyAllBufferData();

		ps->SetShaderResourceView("particle", texture);
		ps->SetShader();
		ps->CopyAllBufferData();


		if (firstAliveIndex < firstDeadIndex)
		{
			context->DrawIndexed(livingParticleCount * 6, firstAliveIndex * 6, 0);
		}
		else
		{
			context->DrawIndexed(firstDeadIndex * 6, 0, 0);

			context->DrawIndexed((max - firstAliveIndex) * 6, firstAliveIndex * 6, 0);
		}
	//}
}
