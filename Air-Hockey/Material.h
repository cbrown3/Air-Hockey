#pragma once
#include "SimpleShader.h"

class Material
{
public:
	Material(SimpleVertexShader * a_vShader, SimplePixelShader * a_pShader);
	Material(SimpleVertexShader* a_vShader,
		SimplePixelShader* a_pShader,
		ID3D11ShaderResourceView* a_textureSRV,
	ID3D11SamplerState* a_sampler);
	
	void Release();

	/*Get Methods*/
	SimpleVertexShader* getVertexShader();
	SimplePixelShader* getPixelShader();
	ID3D11ShaderResourceView* getTextureSRV();
	ID3D11SamplerState* getSampler();

private:
	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11ShaderResourceView* textureSRV;
	ID3D11SamplerState* sampler;
};

