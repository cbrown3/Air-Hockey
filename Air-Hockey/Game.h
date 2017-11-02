#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include <DirectXMath.h>
#include "GameEntity.h"
#include "Camera.h"
#include "Material.h"
#include "Light.h"
#include "Paddle.h"
#include "Puck.h"

class Game
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);
private:

	//Bool for Debug Mode (allow/disallow cam movement)
	bool DebugModeActive;
	int lastHit;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders();
	void LoadLights();
	void CreateMatrices();
	void CreateBasicGeometry();
	void CreateShadowMap();



	// Buffers to hold actual geometry data
	ID3D11Buffer* triVertBuffer;
	ID3D11Buffer* triIndBuffer;
	ID3D11Buffer* sqVertBuffer;
	ID3D11Buffer* sqIndBuffer;
	ID3D11Buffer* diaVertBuffer;
	ID3D11Buffer* diaIndBuffer;

	//int for the amount of indices
	int triNumOfInd;
	int sqNumOfInd;
	int diaNumOfInd;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	//For Shadow Mapping
	int shadowMapSize;
	ID3D11ShaderResourceView* shadowMapSRV;
	ID3D11Texture2D* shadowMapTex;
	ID3D11DepthStencilView* shadowDepthView;
	ID3D11SamplerState* shadowSampler;
	ID3D11RasterizerState* shadowRasterizer;

	SimpleVertexShader* shadowVS;

	XMFLOAT4X4 shadowViewMatrix, shadowProjMatrix;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	//Meshes
	Mesh* cube;
	Mesh* sphere;
	Mesh* cylinder;

	//Entities
	GameEntity* entity;
	GameEntity* entity2;
	
	Paddle* player1;
	Paddle* player2;

	Puck* puck;

	GameEntity* table;



	//Cameras
	Camera* mainCamera;

	//Mouse Press Booleans
	bool rightButtonPressed;
	bool leftButtonPressed;
	bool middleButtonPressed;

	//Materials
	Material* textureMaterial;

	//Lights
	DirectionalLight dirLight;
	PointLight pointLight;

	/*Texture Fields*/

	//An SRV is good enough to load textures with DirectX TK and then
	//using them with shaders
	ID3D11ShaderResourceView* designTextureSRV;
	ID3D11ShaderResourceView* woodTextureSRV;
	ID3D11ShaderResourceView* fabricTextureSRV;


	//Needed for sampling options (like filter and address modes)
	ID3D11SamplerState* sampler;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};

