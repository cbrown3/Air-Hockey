#include "Camera.h"



Camera::Camera()
{
	camPos = XMFLOAT3(0, 5, -6);
	camDir = XMFLOAT3(0, 0, 1);
	camRotX = .7;
	camRotY = 0;

	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0.0f);
	XMVECTOR dir = XMVector3Rotate(
		XMVectorSet(0, 0, speed, 0), rot);

	XMStoreFloat4x4(&viewMatrix,
		XMMatrixLookToLH(
			XMLoadFloat3(&camPos),
			dir, XMVectorSet(0, 1, 0, 0)));

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)1280 / 720,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projMatrix, XMMatrixTranspose(P));
}


Camera::~Camera()
{
}

XMFLOAT3 Camera::getPositon()
{
	return camPos;
}

XMFLOAT4X4 Camera::getViewMatrix()
{
	return viewMatrix;
}

XMFLOAT4X4 Camera::getProjMatrix()
{
	return projMatrix;
}

float Camera::GetSpeed()
{
	return speed;
}

void Camera::SetSpeed(float a_speed)
{
	speed = a_speed;
}

void Camera::MoveForward()
{
	XMVECTOR direction = XMVector3Rotate(
		XMVectorSet(0, 0, speed, 0),
		XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0));

	XMStoreFloat3(
		&camPos,
		XMLoadFloat3(&camPos) + direction);
}

void Camera::MoveForward(float a_speed)
{
	XMVECTOR direction = XMVector3Rotate(
		XMVectorSet(0, 0, a_speed, 0),
		XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0));

	XMStoreFloat3(
		&camPos,
		XMLoadFloat3(&camPos) + direction);
}

void Camera::MoveBack()
{
	XMVECTOR direction = XMVector3Rotate(
		XMVectorSet(0, 0, -speed, 0),
		XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0));

	XMStoreFloat3(
		&camPos,
		XMLoadFloat3(&camPos) + direction);
}

void Camera::MoveBack(float a_speed)
{
	XMVECTOR direction = XMVector3Rotate(
		XMVectorSet(0, 0, -a_speed, 0),
		XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0));

	XMStoreFloat3(
		&camPos,
		XMLoadFloat3(&camPos) + direction);
}

void Camera::StrafeRight()
{
	XMVECTOR direction = XMVector3Rotate(
		XMVectorSet(speed, 0, 0, 0),
		XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0));

	XMStoreFloat3(
		&camPos,
		XMLoadFloat3(&camPos) + direction);
}

void Camera::StrafeLeft()
{
	XMVECTOR direction = XMVector3Rotate(
		XMVectorSet(-speed, 0, 0, 0),
		XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0));

	XMStoreFloat3(
		&camPos,
		XMLoadFloat3(&camPos) + direction);
}

void Camera::MoveUp()
{
	camPos.y += speed;
}

void Camera::MoveDown()
{
	camPos.y -= speed;
}

void Camera::RotateCamera(float a_xangle, float a_yangle)
{
	camRotX += a_xangle * 0.0075f;
	camRotY += a_yangle * 0.0075f;
}

void Camera::Update()
{
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(camRotX, camRotY, 0.0f);
	XMVECTOR dir = XMVector3Rotate(
		XMVectorSet(0, 0, speed, 0), rot);
	
	XMStoreFloat4x4(&viewMatrix,XMMatrixTranspose(
		XMMatrixLookToLH(
		XMLoadFloat3(&camPos), 
		dir, XMVectorSet(0, 1, 0, 0))));
}
