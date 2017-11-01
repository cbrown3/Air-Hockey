#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"

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
	lastHit = 0;

	mainCamera->SetSpeed(0.001f);


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
	fabricTextureSRV->Release();
	sampler->Release();

	delete textureMaterial;

	//delete basic entities
	delete entity;
	delete entity2;

	//delete meshes
	delete cube;
	delete sphere;
	

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;

	//Clean up cameras.
	delete mainCamera;


	//delete paddles puck and table.
	delete player1;
	delete player2;
	delete table;
	//delete puck;  //puck not crrently being created anywhere

	//delete shadow related things
	shadowDepthView->Release();
	shadowMapSRV->Release();
	shadowSampler->Release();
	shadowRasterizer->Release();
	delete shadowVS;
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

	//Load the textures
	CreateWICTextureFromFile(
		device,		//The device handles creating new resources (like textures)
		context,	//context
		L"Assets/Textures/design.jpg",
		0,
		&designTextureSRV);
	
	CreateWICTextureFromFile(
		device,		//The device handles creating new resources (like textures)
		context,	//context
		L"Assets/Textures/fabric.JPG",
		0,
		&fabricTextureSRV);
	

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

	//shadow dpeth stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDepthDesc = {};
	shadowDepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDepthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDepthDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowMapTex, &shadowDepthDesc, &shadowDepthView);

	//shadow resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowMapTex, &shadowSRVDesc, &shadowMapSRV);

	shadowMapTex->Release();

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

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

	shadowVS = new SimpleVertexShader(device, context);
	shadowVS->LoadShaderFile(L"ShadowVs.cso");

	/*CREATE MATERIALS*/
	textureMaterial = new Material(vertexShader, pixelShader, fabricTextureSRV, sampler);
}

void Game::LoadLights()
{
	dirLight = DirectionalLight();

	dirLight.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight.DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dirLight.Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);

	pointLight.AmbientColor = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	pointLight.DiffuseColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pointLight.Position = XMFLOAT3(0.0f, 1.0f, 0.0f);
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

	//setting things back to normal
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetState(0);

	viewport.Width = this->width;
	viewport.Height = this->height;
	context->RSSetViewports(1, &viewport);
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
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	cube = new Mesh("Assets/Models/cube.obj", device);
	sphere = new Mesh("Assets/Models/sphere.obj", device);

	player1 = new Paddle(cube, textureMaterial);
	player2 = new Paddle(cube, textureMaterial);

	player1->SetPosition(-2.5f, 0.0f, 0.0f);
	player2->SetPosition(2.5f, 0.0f, 0.0f);
	player1->SetScale(0.5f, 0.5f, 0.5f);
	player2->SetScale(0.5f, 0.5f, 0.5f);

	player1->UpdateWorldMatrix();
	player2->UpdateWorldMatrix();

	entity = new GameEntity(sphere, textureMaterial);
	entity2 = new GameEntity(sphere, textureMaterial);
	entity->SetPosition(20.0f, 1.0f, 0.0f);
	entity->UpdateWorldMatrix();
	entity2->SetScale(.01f, .01f, .01f);
	entity2->UpdateWorldMatrix();


	//if the cube is 1x1x1 then the x border will be 4 to -4 and the z border will be -1 to 3
	table = new GameEntity(cube, textureMaterial);
	table->SetPosition(0.0f, -.5f, 1.0f);
	table->SetScale(8.0f, 0.5f, 4.0f);
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
	
	CreateShadowMap();

	//Movement for the main object
	if (GetAsyncKeyState('J') & 0x8000)
	{
		
	}
	if (GetAsyncKeyState('I') & 0x8000)
	{
		player2->MoveAbsolute(0.0f, 0.0f, 3 * deltaTime);

		if (player2->GetPosition().z > 1)
		{
			player2->SetPosition(player2->GetPosition().x, player2->GetPosition().y, 1);
		}

	}
	if (GetAsyncKeyState('K') & 0x8000)
	{
		player2->MoveAbsolute(0.0f, 0.0f, -3 * deltaTime);
		
		if (player2->GetPosition().z < -1)
		{
			player2->SetPosition(player2->GetPosition().x, player2->GetPosition().y, -1);
		}
	}
	if (GetAsyncKeyState('L') & 0x8000)
	{
	
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

	entity->UpdateWorldMatrix();
	if (DebugModeActive) {
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
	else
	{
		if (GetAsyncKeyState('W') & 0x8000)
		{
			player1->MoveAbsolute(0.0f, 0.0f, 3 * deltaTime);

			if (player1->GetPosition().z > 1)
			{
				player1->SetPosition(player1->GetPosition().x, player1->GetPosition().y, 1);
			}
		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			player1->MoveAbsolute(0.0f, 0.0f, -3 * deltaTime);

			if (player1->GetPosition().z < -1)
			{
				player1->SetPosition(player1->GetPosition().x, player1->GetPosition().y, -1);
			}
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			
		}

	}
	

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
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

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

	pixelShader->SetSamplerState("ShadowSampler", shadowSampler);
	pixelShader->SetShaderResourceView("ShadowMap", shadowMapSRV);

	pixelShader->SetFloat3("cameraPosition", mainCamera->getPositon()); //sending cam position for specular
	

	pixelShader->SetSamplerState("basicSampler", sampler);

	pixelShader->SetShaderResourceView("srv", fabricTextureSRV); //NEEDS TO BE SET UP FOR EACH ENTITY, might want to have a way to get srv from the entity, otherwise a lot of manual work needs to be done
	entity->PrepareMaterial(viewMatrix, projectionMatrix);
	entity->Draw(context);

	pixelShader->SetShaderResourceView("srv", designTextureSRV); //Same as above
	entity2->PrepareMaterial(viewMatrix, projectionMatrix);
	entity2->Draw(context);

	player1->PrepareMaterial(viewMatrix, projectionMatrix);
	player1->Draw(context);

	player2->PrepareMaterial(viewMatrix, projectionMatrix);
	player2->Draw(context);

	table->PrepareMaterial(viewMatrix, projectionMatrix);
	table->Draw(context);
	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
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