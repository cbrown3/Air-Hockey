#include "Material.h"


Material::Material(SimpleVertexShader * a_vShader, SimplePixelShader * a_pShader)
{
	vertexShader = a_vShader;
	pixelShader = a_pShader;
}

Material::Material(SimpleVertexShader * a_vShader, SimplePixelShader * a_pShader, ID3D11ShaderResourceView * a_textureSRV, ID3D11SamplerState * a_sampler)
{
	vertexShader = a_vShader;
	pixelShader = a_pShader;
	textureSRV = a_textureSRV;
	sampler = a_sampler;
}

Material::~Material()
{
}

SimpleVertexShader * Material::getVertexShader()
{
	return vertexShader;
}

SimplePixelShader * Material::getPixelShader()
{
	return pixelShader;
}

ID3D11ShaderResourceView * Material::getTextureSRV()
{
	return textureSRV;
}

ID3D11SamplerState * Material::getSampler()
{
	return sampler;
}
