#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"Air-Hockey",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	rightButtonPressed = false;
	leftButtonPressed = false;
	middleButtonPressed = false;

	mainCamera = new Camera();

	DebugModeActive = false;
	paused = false;
	lastHit = 0;

	mainCamera->SetSpeed(0.05f);


#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif

}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	//Clean up 
	designTextureSRV->Release();
	paddleTextureSRV->Release();
	//designNormMapSRV->Release();
	sampler->Release();

	delete designMaterial;
	delete paddleMaterial;
	delete TEST_MATERIAL;

	//delete meshes
	delete cube;
	delete sphere;
	delete cylinder;
	delete hockeyPaddle;
	delete hockeyTable;
	

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;

	//Clean up cameras.
	delete mainCamera;


	//delete paddles puck and table.
	delete player1;
	delete player2;
	delete puck;
	delete table;
	delete tableBorder;
	delete TEST_ENTITY;

	//delete shadow related things
	shadowDepthView->Release();
	shadowMapSRV->Release();
	shadowSampler->Release();
	shadowRasterizer->Release();

	//pShadowMapSRV->Release();
	//pShadowCubeTexture->Release();

	delete[] pShadowViewMatrix;
	
	for(int i = 0; i < 6; i++)
	{
		//pShadowCubeDepthView[i]->Release();
	}
	

	pShadowCubeTex->Release();
	

	delete shadowVS;

	//Delete Skybox things
	delete skyVS;
	delete skyPS;
	skySRV->Release();
	skyRasterState->Release();
	skyDepthState->Release();

	//delete Particle things
	delete particleVS;
	delete particlePS;
	delete emitter;
	delete emitter1;
	delete emitter2;
	delete emitter3;

	particleTexture->Release();
	particleBlendState->Release();
	particleDepthState->Release();

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	LoadLights();
	CreateMatrices();
	CreateBasicGeometry();

	pShadowViewMatrix = new XMFLOAT4X4[6];

	//Load the textures
	CreateWICTextureFromFile(
		device,		//The device handles creating new resources (like textures)
		context,	//context
		L"Assets/Textures/airhockey.jpg",
		0,
		&designTextureSRV);

	CreateWICTextureFromFile(
		device,		//The device handles creating new resources (like textures)
		context,	//context
		L"Assets/Textures/PureWhite.png",
		0,
		&designNormMapSRV);

	CreateWICTextureFromFile(
		device,		//The device handles creating new resources (like textures)
		context,	//context
		L"Assets/Textures/hockeypaddle.png",
		0,
		&paddleTextureSRV);

	CreateWICTextureFromFile(
		device,		//The device handles creating new resources (like textures)
		context,	//context
		L"Assets/Textures/puck.png",
		0,
		&puckSRV);

	CreateWICTextureFromFile(
		device,
		context,
		L"Assets/Textures/particle.jpg",
		0,
		&particleTexture);

	CreateDDSTextureFromFile(
		device,
		L"Assets/Textures/GalaxySkyBox.dds",
		0,
		&skySRV);

	CreateDDSTextureFromFile(
		device,
		L"Assets/Textures/paddleNormal.png",
		0,
		&TEST_TEXTURE);

	//States for drawing the sky

	//Rasterize state for drawing the "inside"
	D3D11_RASTERIZER_DESC skyRD = {};
	skyRD.CullMode = D3D11_CULL_FRONT;
	skyRD.FillMode = D3D11_FILL_SOLID;
	skyRD.DepthClipEnable = false;
	device->CreateRasterizerState(&skyRD, &skyRasterState);

	//Depth state for accepting pixels with the same depth as the existing depth
	D3D11_DEPTH_STENCIL_DESC skyDS = {};
	skyDS.DepthEnable = true;
	skyDS.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;//switch to all if it looks weird
	skyDS.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&skyDS, &skyDepthState);


	//Shadow related stuff
	shadowMapSize = 1024;

	D3D11_TEXTURE2D_DESC shadowTexDesc = {};
	shadowTexDesc.Width = shadowMapSize;
	shadowTexDesc.Height = shadowMapSize;
	shadowTexDesc.ArraySize = 1;
	shadowTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowTexDesc.CPUAccessFlags = 0;
	shadowTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowTexDesc.MipLevels = 1;
	shadowTexDesc.MiscFlags = 0;
	shadowTexDesc.SampleDesc.Count = 1;
	shadowTexDesc.SampleDesc.Quality = 0;
	shadowTexDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&shadowTexDesc, 0, &shadowMapTex);
	
	shadowTexDesc.ArraySize = 6;
	shadowTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	device->CreateTexture2D(&shadowTexDesc, 0, &pShadowCubeTex);


	//shadow dpeth stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDepthDesc = {};
	shadowDepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDepthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDepthDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowMapTex, &shadowDepthDesc, &shadowDepthView);


	D3D11_DEPTH_STENCIL_VIEW_DESC CubeDepthDesc = {};
	CubeDepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	CubeDepthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	CubeDepthDesc.Texture2DArray.MipSlice = 0;
	CubeDepthDesc.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < 6; i++)
	{
		CubeDepthDesc.Texture2DArray.FirstArraySlice = i;
		device->CreateDepthStencilView(pShadowCubeTex, &CubeDepthDesc, &pShadowCubeDepthView[i]);
	}

	//not sure if I want a TextureCube or 2DArray. Both seem to work the same
	D3D11_SHADER_RESOURCE_VIEW_DESC cubeSRVDesc = {};
	cubeSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	cubeSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	//cubeSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	cubeSRVDesc.TextureCube.MostDetailedMip = 0;
	cubeSRVDesc.TextureCube.MipLevels = 1;
	//cubeSRVDesc.Texture2DArray.MostDetailedMip = 0;
	//cubeSRVDesc.Texture2DArray.MipLevels = 1;
	//cubeSRVDesc.Texture2DArray.ArraySize = 6;
	//cubeSRVDesc.Texture2DArray.FirstArraySlice = 1;
	device->CreateShaderResourceView(pShadowCubeTex, &cubeSRVDesc, &pShadowMapSRV);

	//shadow resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowMapTex, &shadowSRVDesc, &shadowMapSRV);


	//shadow sampler state
	D3D11_SAMPLER_DESC shadowSamplerDesc = {};
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	device->CreateSamplerState(&shadowSamplerDesc, &shadowSampler);

	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000;
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);
	
	//Create a sampler state
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);
	
	//depth state for particles
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &particleDepthState);

	//additive blending for particles
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	/*Emitter(DirectX::XMFLOAT3 pos,
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
		ID3D11Device* device,
		SimpleVertexShader* vs,
		SimplePixelShader* ps,
		ID3D11ShaderResourceView* texture);*/
	XMFLOAT4 particleStartColor = { 0.5f, 0.2f, 0.0f, 1.0f };
	XMFLOAT4 particleEndColor = { 1.0f, 0.1f, 0.0f, 0.0f };
	emitter = new Emitter(
		XMFLOAT3(3.5f, 0.0f, -1.5f),//pos
		XMFLOAT3(-2.0f, 2.0f, 2.0f),//vel
		XMFLOAT3(0.0f, -1.0f, 0.0f),//acc
		particleStartColor,//start color
		particleEndColor,//end color
		0.1f,//start size
		2.5f,//end size
		1000,//max
		100,//rate
		true,//loop
		false,//active
		2,//lifetime
		device,
		particleVS,
		particlePS,
		particleTexture
	);
	emitter1 = new Emitter(
		XMFLOAT3(3.5f, 0.0f, 1.5f),//pos
		XMFLOAT3(-2.0f, 2.0f, -2.0f),//vel
		XMFLOAT3(0.0f, -1.0f, 0.0f),//acc
		particleStartColor,//start color
		particleEndColor,//end color
		0.1f,//start size
		2.5f,//end size
		1000,//max
		100,//rate
		true,//loop
		false,//active
		2,//lifetime
		device,
		particleVS,
		particlePS,
		particleTexture
	);
	emitter2 = new Emitter(
		XMFLOAT3(-3.5f, 0.0f, -1.5f),//pos
		XMFLOAT3(2.0f, 2.0f, 2.0f),//vel
		XMFLOAT3(0.0f, -1.0f, 0.0f),//acc
		particleStartColor,//start color
		particleEndColor,//end color
		0.1f,//start size
		2.5f,//end size
		1000,//max
		100,//rate
		true,//loop
		false,//active
		2,//lifetime
		device,
		particleVS,
		particlePS,
		particleTexture
	);
	emitter3 = new Emitter(
		XMFLOAT3(-3.5f, 0.0f, 1.5f),//pos
		XMFLOAT3(2.0f, 2.0f, -2.0f),//vel
		XMFLOAT3(0.0f, -1.0f, 0.0f),//acc
		particleStartColor,//start color
		particleEndColor,//end color
		0.1f,//start size
		2.5f,//end size
		1000,//max
		100,//rate
		true,//loop
		false,//active
		2,//lifetime
		device,
		particleVS,
		particlePS,
		particleTexture
	);
	particleTimer = 0;

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::cout << "\nGame Has Started\n";

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	//Load in shaders for shadow
	shadowVS = new SimpleVertexShader(device, context);
	shadowVS->LoadShaderFile(L"ShadowVs.cso");

	//Load in shaders for sky
	skyVS = new SimpleVertexShader(device, context);
	skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, context);
	skyPS->LoadShaderFile(L"SkyPS.cso");

	//load particle shaders
	particleVS = new SimpleVertexShader(device, context);
	particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, context);
	particlePS->LoadShaderFile(L"ParticlePS.cso");

	/*CREATE MATERIALS*/
	designMaterial = new Material(vertexShader, pixelShader, designTextureSRV, sampler);
	paddleMaterial = new Material(vertexShader, pixelShader, paddleTextureSRV, sampler);
	TEST_MATERIAL = new Material(vertexShader, pixelShader, TEST_TEXTURE, sampler);
	puckMaterial = new Material(vertexShader, pixelShader, puckSRV, sampler);
}

void Game::LoadLights()
{
	dirLight = DirectionalLight();

	dirLight.AmbientColor = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	dirLight.DiffuseColor = XMFLOAT4(.6f, .6f, .6f, 1.0f);
	dirLight.Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);

	pointLight.AmbientColor = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	pointLight.DiffuseColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pointLight.Position = XMFLOAT3(0.0f, -0.1f, 0.0f);
}

void Game::CreateShadowMapDirectionalOnly()
{
	context->OMSetRenderTargets(0, 0, shadowDepthView);
	context->ClearDepthStencilView(shadowDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer);

	//viewport setup
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	//set shaders
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjMatrix);

	context->PSSetShader(0, 0, 0);

	shadowVS->SetMatrix4x4("world", player1->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	player1->Draw(context);

	shadowVS->SetMatrix4x4("world", player2->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	player2->Draw(context);

	shadowVS->SetMatrix4x4("world", table->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	table->Draw(context);

	shadowVS->SetMatrix4x4("world", puck->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	puck->Draw(context);

	//setting things back to normal
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetState(0);

	viewport.Width = this->width;
	viewport.Height = this->height;
	context->RSSetViewports(1, &viewport);

}
//Makes the Shadow Map (call each frame at the beginning) (Currently works with the directional light)
void Game::CreateShadowMap()
{
	context->OMSetRenderTargets(0, 0, shadowDepthView);
	context->ClearDepthStencilView(shadowDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer);

	//viewport setup
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	//set shaders
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjMatrix);

	context->PSSetShader(0, 0, 0);

	shadowVS->SetMatrix4x4("world", player1->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	player1->Draw(context);

	shadowVS->SetMatrix4x4("world", player2->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	player2->Draw(context);

	shadowVS->SetMatrix4x4("world", table->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	table->Draw(context);

	shadowVS->SetMatrix4x4("world", puck->GetWorldMatrix());
	shadowVS->CopyAllBufferData();
	puck->Draw(context);

	/*/setting things back to normal
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetState(0);

	viewport.Width = this->width;
	viewport.Height = this->height;
	context->RSSetViewports(1, &viewport);
	/*/

	//Point Light Shadows (dear god)
	pShadowView1 = XMMatrixLookAtLH(XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(pointLight.Position.x + 1, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(0, 1, 0, 1));
	pShadowView2 = XMMatrixLookAtLH(XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(pointLight.Position.x - 1, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(0, 1, 0, 1));
	pShadowView3 = XMMatrixLookAtLH(XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(pointLight.Position.x, pointLight.Position.y + 1, pointLight.Position.z, 1), XMVectorSet(0, 0, 1, 1)); // height (last param) might need to be 0 0 1
	pShadowView4 = XMMatrixLookAtLH(XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(pointLight.Position.x, pointLight.Position.y - 1, pointLight.Position.z, 1), XMVectorSet(0, 0, -1, 1));// height (last param) might need to be 0 0 -1
	pShadowView5 = XMMatrixLookAtLH(XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z + 1, 1), XMVectorSet(0, 1, 0, 1));
	pShadowView6 = XMMatrixLookAtLH(XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z, 1), XMVectorSet(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z - 1, 1), XMVectorSet(0, 1, 0, 1));

	//pShadowViewMatrix[0] = {};

	XMStoreFloat4x4(&pShadowViewMatrix[0], XMMatrixTranspose(pShadowView1));
	XMStoreFloat4x4(&pShadowViewMatrix[1], XMMatrixTranspose(pShadowView2));
	XMStoreFloat4x4(&pShadowViewMatrix[2], XMMatrixTranspose(pShadowView3));
	XMStoreFloat4x4(&pShadowViewMatrix[3], XMMatrixTranspose(pShadowView4));
	XMStoreFloat4x4(&pShadowViewMatrix[4], XMMatrixTranspose(pShadowView5));
	XMStoreFloat4x4(&pShadowViewMatrix[5], XMMatrixTranspose(pShadowView6));


	context->RSSetViewports(1, &viewport);
	for (int i = 0; i < 6; i++)
	{
		context->OMSetRenderTargets(0, 0, pShadowCubeDepthView[i]);
		context->ClearDepthStencilView(pShadowCubeDepthView[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
		context->RSSetState(shadowRasterizer);
		

		shadowVS->SetShader();
		shadowVS->SetMatrix4x4("view", pShadowViewMatrix[i]);
		shadowVS->SetMatrix4x4("projection", pShadowProjMatrix);

		context->PSSetShader(0, 0, 0);

		shadowVS->SetMatrix4x4("world", player1->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		player1->Draw(context);

		shadowVS->SetMatrix4x4("world", player2->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		player2->Draw(context);

		shadowVS->SetMatrix4x4("world", puck->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		puck->Draw(context);
	}


	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetState(0);

	viewport.Width = this->width;
	viewport.Height = this->height;
	context->RSSetViewports(1, &viewport);

	

	
	//for (int i = 0; i < 6; i++)
	//{
	//	cubeData[i].pSysMem = pShadowCubeTex[i];
	//	cubeData[i].SysMemPitch = shadowMapSize / 8;
	//	cubeData[i].SysMemSlicePitch = 0;
	//}

	//device->CreateTexture2D(&cubeMapDesc, &cubeData[0], &pShadowCubeTexture);
	

}

// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	XMMATRIX shadowView = XMMatrixLookAtLH(XMVectorSet(-10, 10, 0, 0), XMVectorSet(0, 0, 0, 0), XMVectorSet(0, 1, 0, 0));

	XMStoreFloat4x4(&shadowViewMatrix, XMMatrixTranspose(shadowView));

	XMMATRIX shadowProj = XMMatrixOrthographicLH(10.0f, 10.0f, 0.1f, 100.0f);

	XMStoreFloat4x4(&shadowProjMatrix, XMMatrixTranspose(shadowProj));

	XMMATRIX pShadowProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, .1f, 3.0f);
	//XMMATRIX pShadowProj = XMMatrixPerspectiveLH(1.0f, 1.0f, .01f, 5.0f);



	XMStoreFloat4x4(&pShadowProjMatrix, XMMatrixTranspose(pShadowProj));
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	cube = new Mesh("Assets/Models/cube.obj", device);
	sphere = new Mesh("Assets/Models/sphere.obj", device);
	cylinder = new Mesh("Assets/Models/cylinder.obj", device);
	hockeyPaddle = new Mesh("Assets/Models/hockeypaddle.obj", device);
	hockeyTable = new Mesh("Assets/Models/hockeytable.obj", device);

	TEST_ENTITY = new GameEntity(cube, TEST_MATERIAL);

	puck = new Puck(cylinder, designMaterial);
	player1 = new Paddle(hockeyPaddle, paddleMaterial);
	player2 = new Paddle(hockeyPaddle, paddleMaterial);
	table = new GameEntity(cube, designMaterial);
	tableBorder = new GameEntity(hockeyTable, TEST_MATERIAL);

	player1->SetPosition(-2.5f, -0.225f, 0.0f);
	player2->SetPosition(2.5f, -0.225f, 0.0f);
	puck->SetPosition(0.0f, -.2f, 0.0f);
	table->SetPosition(0.0f, -.5f, 0.0f);
	tableBorder->SetPosition(1.5f, -2.25f, 11.232f);
	player1->SetScale(0.5f, 0.5f, 0.5f);
	player2->SetScale(0.5f, 0.5f, 0.5f);
	puck->SetScale(0.5f, 0.1f, 0.5f);
	table->SetScale(8.0f, 0.5f, 4.5f);
	tableBorder->SetScale(0.0325f, 0.03f, 0.03f);
	TEST_ENTITY->SetScale(.1f, .1f, .1f);

	player1->UpdateWorldMatrix();
	player2->UpdateWorldMatrix();
	puck->UpdateWorldMatrix();
	table->UpdateWorldMatrix();
	tableBorder->UpdateWorldMatrix();
	TEST_ENTITY->UpdateWorldMatrix();
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//CreateShadowMapDirectionalOnly();
	CreateShadowMap();

	if (!paused)
	{
		//Puck Movement and Collision
		puck->Update(deltaTime);
		puck->CollisionDetection(player1);
		puck->CollisionDetection(player2);
		//Update Point Light Direction
		pointLight.Position = XMFLOAT3(puck->GetPosition().x, .15f, puck->GetPosition().z);

		//Paddle Movement
		PlayerMovement(deltaTime);

		//Camera Movement during Debug Mode
		

		score = puck->checkScore();
	}
	if (particlesActive)
	{
		if(particleTimer<1)
			particleTimer += deltaTime;
		else
		{
			emitter->Deactivate();
			emitter1->Deactivate();
			emitter2->Deactivate();
			emitter3->Deactivate();
			particlesActive = false;
			particleTimer = 0;
		}

	}
	//Score
	if (score == 1) {
		std::cout << "player 1 scored\n";
		emitter->Activate();
		emitter1->Activate();
		particlesActive = true;
	}
	if (score == 2) {
		std::cout << "player 2 scored\n";
		emitter2->Activate();
		emitter3->Activate();
		particlesActive = true;
	}
	

	if (GetAsyncKeyState('T') & 0x8000 && (totalTime > lastHit)) 
	{
		if (DebugModeActive)
		{
			DebugModeActive = false;
		}
		else 
		{
			DebugModeActive = true;
		}

		lastHit = totalTime + 1;

		
	}

	if (GetAsyncKeyState(' ') & 0x8000 && (totalTime > lastHit))
	{
		if (paused)
		{
			paused = false;
		}
		else
		{
			paused = true;
		}

		lastHit = totalTime + 1;
	}

	
	emitter->Update(deltaTime);
	emitter1->Update(deltaTime);
	emitter2->Update(deltaTime);
	emitter3->Update(deltaTime);

	CameraMovement();
	mainCamera->Update();
	
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };//{ 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	viewMatrix = mainCamera->getViewMatrix();
	projectionMatrix = mainCamera->getProjMatrix();

	
	pixelShader->SetData(
		"light",  //The name of the (eventual) variable in the shader
		&dirLight, //The address of the data to copy
		sizeof(DirectionalLight)); //The size of the data to copy

	

	pixelShader->SetData( //sending over the point light
		"pLight",
		&pointLight,
		sizeof(PointLight));

	vertexShader->SetMatrix4x4("shadowViewMat", shadowViewMatrix);
	vertexShader->SetMatrix4x4("shadowProjMat", shadowProjMatrix);
	vertexShader->SetMatrix4x4("CubeShadowProjMat", pShadowProjMatrix);

	pixelShader->SetSamplerState("ShadowSampler", shadowSampler);
	pixelShader->SetShaderResourceView("ShadowMap", shadowMapSRV);
	pixelShader->SetShaderResourceView("ShadowCubeMap", pShadowMapSRV);

	pixelShader->SetFloat3("cameraPosition", mainCamera->getPositon()); //sending cam position for specular
	
	pixelShader->SetShaderResourceView("SkyTexture", skySRV);

	pixelShader->SetSamplerState("basicSampler", sampler);

	pixelShader->SetShaderResourceView("srv", paddleTextureSRV);

	pixelShader->SetShaderResourceView("NormalMap", TEST_TEXTURE);
	
	//Same as above														 //Drawing objects

	player1->PrepareMaterial(viewMatrix, projectionMatrix);
	player1->Draw(context);

	player2->PrepareMaterial(viewMatrix, projectionMatrix);
	player2->Draw(context);

	//Sending Normal Map to Pixel Shader
	pixelShader->SetShaderResourceView("NormalMap", designNormMapSRV);

	pixelShader->SetShaderResourceView("srv", puckSRV);

	puck->PrepareMaterial(viewMatrix, projectionMatrix);
	puck->Draw(context);
	
	pixelShader->SetShaderResourceView("srv", designTextureSRV);
	table->PrepareMaterial(viewMatrix, projectionMatrix);
	table->Draw(context);

	tableBorder->PrepareMaterial(viewMatrix, projectionMatrix);
	tableBorder->Draw(context);

	/**///Test entity drawing
	if (DebugModeActive) 
	{
		pixelShader->SetShaderResourceView("srv", designTextureSRV);
		TEST_ENTITY->SetPosition(pointLight.Position.x, pointLight.Position.y, pointLight.Position.z);
		TEST_ENTITY->PrepareMaterial(viewMatrix, projectionMatrix);
		TEST_ENTITY->Draw(context);
	}
	/**/
	//Particle states
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);
	context->OMSetDepthStencilState(particleDepthState, 0);
	if (!DebugModeActive) {
		emitter->Draw(context, mainCamera);

		emitter1->Draw(context, mainCamera);

		emitter2->Draw(context, mainCamera);

		emitter3->Draw(context, mainCamera);
	}
	
	//draw the sky LAST, this should make it so it draws wherever there isn't
	//already something there and sets the depth to 1.0
	ID3D11Buffer* skyVB = cube->GetVertexBuffer();
	ID3D11Buffer* skyIB = cube->GetIndexBuffer();

	// Set the buffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	// Set up the sky shaders
	skyVS->SetMatrix4x4("view", mainCamera->getViewMatrix());
	skyVS->SetMatrix4x4("projection", mainCamera->getProjMatrix());
	skyVS->CopyAllBufferData();
	skyVS->SetShader();
	
	//skyPS->SetShaderResourceView("SkyTexture", pShadowMapSRV); //Shadow Cube Tex for Debugging
	skyPS->SetShaderResourceView("SkyTexture", skySRV);
	skyPS->SetSamplerState("BasicSampler", sampler);
	skyPS->SetShader();

	// Set up the render state options
	context->RSSetState(skyRasterState);
	context->OMSetDepthStencilState(skyDepthState, 0);



	//Draw sky
	context->DrawIndexed(cube->GetIndexCount(), 0, 0);
	/**/


	//reset the blendstate
	context->OMSetBlendState(0, blend, 0xffffffff);

	// Reset any states we've changed for the next frame!
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

void Game::PlayerMovement(float deltaTime)
{
	//Movement for Player 1
	if (DebugModeActive == false)
	{
		if (GetAsyncKeyState('W') & 0x8000)
		{
			player1->MoveAbsolute(0.0f, 0.0f, 5 * deltaTime);

			if (player1->GetPosition().z > 1.2f)
			{
				player1->SetPosition(player1->GetPosition().x, player1->GetPosition().y, 1.2f);
			}
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			player1->MoveAbsolute(5 * deltaTime, 0.0f, 0.0f);

			if (player1->GetPosition().x > -0.8f)
			{
				player1->SetPosition(-0.8f, player1->GetPosition().y, player1->GetPosition().z);
			}
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			player1->MoveAbsolute(0.0f, 0.0f, -5 * deltaTime);

			if (player1->GetPosition().z < -1.2f)
			{
				player1->SetPosition(player1->GetPosition().x, player1->GetPosition().y, -1.2f);
			}
		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			player1->MoveAbsolute(-5 * deltaTime, 0.0f, 0.0f);

			if (player1->GetPosition().x < -3.2f)
			{
				player1->SetPosition(-3.2f, player1->GetPosition().y, player1->GetPosition().z);
			}
		}
	}

	//Movement for Player 2
	if (GetAsyncKeyState('L') & 0x8000)
	{
		player2->MoveAbsolute(5 * deltaTime, 0.0f, 0.0f);

		if (player2->GetPosition().x > 2.8f)
		{
			player2->SetPosition(2.8f, player2->GetPosition().y, player2->GetPosition().z);
		}
	}
	if (GetAsyncKeyState('I') & 0x8000)
	{
		player2->MoveAbsolute(0.0f, 0.0f, 5 * deltaTime);

		if (player2->GetPosition().z > 1.2f)
		{
			player2->SetPosition(player2->GetPosition().x, player2->GetPosition().y, 1.2f);
		}

	}
	if (GetAsyncKeyState('K') & 0x8000)
	{
		player2->MoveAbsolute(0.0f, 0.0f, -5 * deltaTime);

		if (player2->GetPosition().z < -1.2f)
		{
			player2->SetPosition(player2->GetPosition().x, player2->GetPosition().y, -1.2f);
		}
	}
	if (GetAsyncKeyState('J') & 0x8000)
	{
		player2->MoveAbsolute(-5 * deltaTime, 0.0f, 0.0f);

		if (player2->GetPosition().x < 0.8f)
		{
			player2->SetPosition(0.8f, player2->GetPosition().y, player2->GetPosition().z);
		}
	}
}

void Game::CameraMovement()
{
	if (DebugModeActive)
	{
		if (GetAsyncKeyState('W') & 0x8000)
		{
			mainCamera->MoveForward();
		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			mainCamera->StrafeLeft();
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			mainCamera->MoveBack();
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			mainCamera->StrafeRight();
		}
		if (GetAsyncKeyState('E') & 0x8000)
		{
			mainCamera->MoveUp();
		}
		if (GetAsyncKeyState('Q') & 0x8000)
		{
			mainCamera->MoveDown();
		}
	}
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	if (buttonState & 0x0001)
	{
		/*Left button is down*/
		leftButtonPressed = true;
	}

	if (buttonState & 0x002)
	{
		/*Right button is down*/
		rightButtonPressed = true;
	}

	if (buttonState & 0x0003)
	{
		/*Middle button is down*/
		middleButtonPressed = true;
	}


	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here
	leftButtonPressed = false;
	rightButtonPressed = false;
	middleButtonPressed = false;

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (leftButtonPressed && DebugModeActive)
	{
		mainCamera->RotateCamera(y - float(prevMousePos.y), x - float(prevMousePos.x));
	}

	if (rightButtonPressed && DebugModeActive)
	{
		if (prevMousePos.x > x)
		{
			mainCamera->StrafeLeft();
		}
		if (prevMousePos.x < x)
		{
			mainCamera->StrafeRight();
		}
		if (prevMousePos.y > y)
		{
			mainCamera->MoveUp();
		}
		if (prevMousePos.y < y)
		{
			mainCamera->MoveDown();
		}
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
	if (wheelDelta > 0)
	{
		mainCamera->MoveForward(0.5f);
	}
	if (wheelDelta < 0)
	{
		mainCamera->MoveBack(0.5f);
	}
}
#pragma endregion