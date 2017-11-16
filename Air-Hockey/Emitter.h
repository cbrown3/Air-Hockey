#pragma once
#include <vector>
#include <DirectXMath.h>

#include "Material.h"
#include "Camera.h"


struct Particle
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 StartVel;
	float Size;
	float Age;
};

struct ParticleVertex 
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT4 Color;
	float Size;
};

class Emitter
{
public:
	Emitter();
	Emitter(Material* a_mat,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT3 vel,
		int maxParticles,
		int emissionRate,
		bool loopable,
		bool active);
	~Emitter();

	void Activate();
	void Deactivate();

	void Update(float dt);

	void UpdateSingleParticle(float dt, int index);
	void SpawnParticle();

	void CopyParticlesToGPU(ID3D11DeviceContext* context);
	void CopyOneParticle(int index);
	void Draw(ID3D11DeviceContext* context, Camera* camera);

private:
	//cyclical buffer stuff
	std::vector<Particle>* particles;
	int max;
	int firstDeadIndex;
	int firstAliveIndex;
	bool loop;
	bool active;

	//drawing stuff
	std::vector<ParticleVertex>* particleVertices;
	Material* mat;//make new mat with texture, ps, vs. if using more textures need new materials or to separate out the components in this class
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11ShaderResourceView* texture;

	//age dependent variables
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float startSize;
	float endSize;
	DirectX::XMFLOAT3 acceleration;

	//emitter variables
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	
	//rate stuff
	int particlesPerSecond;
	float secondsPerParticle;
	int floatTimeSinceEmit;


};

