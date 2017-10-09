#pragma once
#include <DirectXMath.h>
#include "DXCore.h"
using namespace DirectX;

class Camera
{
public:
	Camera();
	~Camera();

	/*Gets and Sets*/
	XMFLOAT4X4 getViewMatrix();
	XMFLOAT4X4 getProjMatrix();
	XMFLOAT3 getPositon(); 
	float GetSpeed();
	void SetSpeed(float a_speed);

	/*Movement*/
	void MoveForward();
	void MoveForward(float a_speed);
	void MoveBack();
	void MoveBack(float a_speed);
	void StrafeRight();
	void StrafeLeft();
	void MoveUp();
	void MoveDown();
	void RotateCamera(float a_xangle, float a_yangle);

	

	void Update();

private:
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projMatrix;
	XMFLOAT3 camPos;
	XMFLOAT3 camDir;
	float camRotX;
	float camRotY;
	float speed;
};
