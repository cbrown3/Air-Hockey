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

	mainCamera->SetSpeed(0.005f);


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
	sampler->Release();
	textureMaterial->Release();
	entity->Release();
	cube->Release();

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;

	//Clean up entities, meshes, cameras, etc.
	delete mainCamera;

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

	/*CREATE MATERIALS*/
	textureMaterial = new Material(vertexShader, pixelShader, designTextureSRV, sampler);
}

void Game::LoadLights()
{
	dirLight = DirectionalLight();

	dirLight.AmbientColor = XMFLOAT4(0.0f, 0.4f, 0.2f, 1.0f);
	dirLight.DiffuseColor = XMFLOAT4(0.2f, 0.2f, 0.3f, 1.0f);
	dirLight.Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	cube = new Mesh("Assets/Models/cube.obj", device);

	entity = new GameEntity(cube, textureMaterial);
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{

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

	entity->PrepareMaterial(viewMatrix, projectionMatrix);
	pixelShader->SetData(
		"light",  //The name of the (eventual) variable in the shader
		&dirLight, //The address of the data to copy
		sizeof(DirectionalLight)); //The size of the data to copy
	pixelShader->SetSamplerState("basicSampler", sampler);
	pixelShader->SetShaderResourceView("srv", designTextureSRV);
	entity->Draw(context);

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
	if (leftButtonPressed)
	{
		mainCamera->RotateCamera(y - prevMousePos.y, x - prevMousePos.x);
	}

	if (rightButtonPressed)
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