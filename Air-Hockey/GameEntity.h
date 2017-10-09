#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Mesh.h"
#include "SimpleShader.h"
#include "Material.h"

using namespace DirectX;

class GameEntity
{  
public:
	GameEntity();
	GameEntity(Mesh* a_mesh, Material* a_mat);
	
	void Release();

	/* Gets and Sets */
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();
	XMFLOAT3 GetScale();
	Material* getMaterial();
	void SetWorldMatrix(XMMATRIX a_matrix);
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	/*Movement Methods*/
	void MoveAbsolute(float x, float y, float z);
	void MoveRelative(float x, float y, float z);
	void MoveForward(float dist);

	/*Drawing Methods*/
	void Draw(ID3D11DeviceContext* a_context);

	/*Update WorldMatrix*/
	void UpdateWorldMatrix();

	/*Materials*/
	void PrepareMaterial(XMFLOAT4X4 a_viewMat, XMFLOAT4X4 a_projMat);

private:
	//WorldMatrix
	XMFLOAT4X4 worldMatrix;
	XMFLOAT3 entityPos;
	XMFLOAT3 entityRot;
	XMFLOAT3 entityScale;

	//Mesh
	Mesh* entityMesh;

	//SimpleShaders
	Material* material;
};

