#include "GameEntity.h"


GameEntity::GameEntity()
{
	entityPos = XMFLOAT3();
	entityRot = XMFLOAT3();
	entityScale = XMFLOAT3();

	XMMATRIX trans = XMMatrixTranslation(entityPos.x, entityPos.y, entityPos.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&entityRot));
	XMMATRIX scale = XMMatrixScaling(entityScale.x, entityScale.y, entityScale.z);

	XMMATRIX world = scale * rot * trans;

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));


}

GameEntity::GameEntity(Mesh* a_mesh, Material* a_mat)
{

	material = a_mat;

	entityMesh = a_mesh;

	entityPos = XMFLOAT3(0,0,0);
	entityRot = XMFLOAT3(0,0,0);
	entityScale = XMFLOAT3(1,1,1);

	XMMATRIX trans = XMMatrixTranslation(entityPos.x, entityPos.y, entityPos.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&entityRot));
	XMMATRIX scale = XMMatrixScaling(entityScale.x, entityScale.y, entityScale.z);

	XMMATRIX world = scale * rot * trans;

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	
	
}

void GameEntity::Release()
{
	delete entityMesh;
	delete material;
}

XMFLOAT4X4 GameEntity::GetWorldMatrix()
{
	return worldMatrix;
}

XMFLOAT3 GameEntity::GetPosition()
{
	return entityPos;
}

XMFLOAT3 GameEntity::GetRotation()
{
	return entityRot;
}

XMFLOAT3 GameEntity::GetScale()
{
	return entityScale;
}

void GameEntity::SetWorldMatrix(XMMATRIX a_matrix)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(a_matrix));
}

void GameEntity::SetPosition(float x, float y, float z)
{
	entityPos = XMFLOAT3(x, y, z);

	XMMATRIX trans = XMMatrixTranslation(x, y, z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&entityRot));
	XMMATRIX scale = XMMatrixScaling(entityScale.x, entityScale.y, entityScale.z);

	XMMATRIX world = scale * rot * trans;

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
}

void GameEntity::SetRotation(float x, float y, float z)
{
	entityRot = XMFLOAT3(x, y, z);

	XMMATRIX trans = XMMatrixTranslation(entityPos.x, entityPos.y, entityPos.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&entityRot));
	XMMATRIX scale = XMMatrixScaling(entityScale.x, entityScale.y, entityScale.z);

	XMMATRIX world = scale * rot * trans;

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
}

void GameEntity::SetScale(float x, float y, float z)
{
	entityScale = XMFLOAT3(x, y, z);

	XMMATRIX trans = XMMatrixTranslation(entityPos.x, entityPos.y, entityPos.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&entityRot));
	XMMATRIX scale = XMMatrixScaling(x, y, z);

	XMMATRIX world = scale * rot * trans;

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
}

void GameEntity::MoveAbsolute(float x, float y, float z)
{
	entityPos.x += x;
	entityPos.y += y;
	entityPos.z += z;
}

void GameEntity::MoveRelative(float x, float y, float z)
{
	XMVECTOR direction = XMVector3Rotate(
		XMVectorSet(x, y, z, 0),
		XMLoadFloat3(&entityRot));

	XMStoreFloat3(
		&entityPos,
		XMLoadFloat3(&entityPos) + direction);
}

void GameEntity::MoveForward(float dist)
{

}

void GameEntity::Draw(ID3D11DeviceContext* a_context)
{
	//Update World Matrix
	UpdateWorldMatrix();

	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* vBuffer = entityMesh->GetVertexBuffer();
	ID3D11Buffer* iBuffer = entityMesh->GetIndexBuffer();

	a_context->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);
	a_context->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Finally do the actual drawing
	a_context->DrawIndexed(
		entityMesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}

void GameEntity::UpdateWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(entityPos.x, entityPos.y, entityPos.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&entityRot));
	XMMATRIX scale = XMMatrixScaling(entityScale.x, entityScale.y, entityScale.z);

	XMMATRIX world = scale * rot * trans;

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
}

void GameEntity::PrepareMaterial(XMFLOAT4X4 a_viewMat, XMFLOAT4X4 a_projMat)
{
	
	SimpleVertexShader* vShader = material->getVertexShader();
	SimplePixelShader* pShader = material->getPixelShader();
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	vShader->SetMatrix4x4("world", worldMatrix);
	vShader->SetMatrix4x4("view", a_viewMat);
	vShader->SetMatrix4x4("projection", a_projMat);

	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	vShader->CopyAllBufferData();
	pShader->CopyAllBufferData();
	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	vShader->SetShader();
	pShader->SetShader();
}
