#pragma once
#include <d3d11.h>
#include "Vertex.h"
#include <fstream>
#include <vector>
using namespace DirectX;

class Mesh
{
public:
	Mesh(Vertex* a_vertices, int a_numOfVert, UINT* a_indices, int a_numOfInd, ID3D11Device* a_device);
	Mesh(char* meshData, ID3D11Device * a_device);
	Mesh();
	
	void Release();

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	int GetIndexCount();
	void BufferCreation(Vertex * a_vertices, int a_numOfVert, UINT * a_indices, int a_numOfInd, ID3D11Device * a_device);

private:
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	int numOfIndices;
};

