#include "Framework.h"
#include "Camera.h"

Camera::Camera()
{
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matRotation);

	Rotation();
	Move();
}

Camera::~Camera()
{
}


void Camera::Position(float x, float y, float z)
{
	Position(Vector3(x, y, z));
}

void Camera::Position(Vector3 & vec)
{
	position = vec;

	Move();
}

void Camera::Position(Vector3 * vec)
{
	*vec = position;
}

void Camera::Rotation(float x, float y, float z)
{
	Rotation(Vector3(x, y, z));
}

void Camera::Rotation(Vector3 & vec)
{
	rotation = vec;

	Rotation();
}

void Camera::Rotation(Vector3 * vec)
{
	*vec = rotation;
}

void Camera::RotationDegree(float x, float y, float z)
{
	RotationDegree(Vector3(x, y, z));
}

void Camera::RotationDegree(Vector3 & vec)
{
	rotation = vec * 0.01745328f; // vec * Math::PI / 180.0f

	Rotation();
}

void Camera::RotationDegree(Vector3 * vec)
{
	*vec = rotation * 57.295791f;
}

void Camera::GetMatrix(Matrix * matrix)
{
	memcpy(matrix, &matView, sizeof(Matrix));
}

void Camera::Move()
{
	View();
}

void Camera::Rotation()
{
	Matrix X, Y, Z;
	D3DXMatrixRotationX(&X, rotation.x);
	D3DXMatrixRotationY(&Y, rotation.y);
	D3DXMatrixRotationZ(&Z, rotation.z);

	matRotation = X * Y * Z;

	//저장될 값 <- ..의 방향 <- ..공간
	D3DXVec3TransformNormal(&foward, &Vector3(0, 0, 1), &matRotation);
	D3DXVec3TransformNormal(&up, &Vector3(0, 1, 0), &matRotation);
	D3DXVec3TransformNormal(&right, &Vector3(1, 0, 0), &matRotation);

	View();
	
}

void Camera::View()
{
	D3DXMatrixLookAtLH(&matView, &position, &(position + foward), &up);
}
