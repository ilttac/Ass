#include "Framework.h"
#include "Frustum.h"

Frustum::Frustum(Camera* camera, Perspective* perspective)
	: camera(camera), perspective(perspective)
{
	if (camera == NULL)
		this->camera = Context::Get()->GetCamera();

	if (perspective == NULL)
		this->perspective = Context::Get()->GetPerspective();
}

Frustum::~Frustum()
{
}

void Frustum::Update()
{
	Matrix V, P;
	camera->GetMatrix(&V);
	perspective->GetMatrix(&P);

	Matrix M = V * P;
	//Right
	planes[0].a = M._14 + M._11;
	planes[0].b = M._24 + M._21;
	planes[0].c = M._34 + M._31;
	planes[0].d = M._44 + M._41;

	//Left
	planes[1].a = M._14 - M._11;
	planes[1].b = M._24 - M._21;
	planes[1].c = M._34 - M._31;
	planes[1].d = M._44 - M._41;

	//Top
	planes[2].a = M._14 + M._12;
	planes[2].b = M._24 + M._22;
	planes[2].c = M._34 + M._32;
	planes[2].d = M._44 + M._42;

	//Bottom
	planes[3].a = M._14 - M._12;
	planes[3].b = M._24 - M._22;
	planes[3].c = M._34 - M._32;
	planes[3].d = M._44 - M._42;

	//Near
	planes[4].a = M._13;
	planes[4].b = M._23;
	planes[4].c = M._33;
	planes[4].d = M._43;

	//Far
	planes[5].a = M._14 - M._13;
	planes[5].b = M._24 - M._23;
	planes[5].c = M._34 - M._33;
	planes[5].d = M._44 - M._43;

	for (int i = 0; i < 6; i++)
		D3DXPlaneNormalize(&planes[i], &planes[i]);
}

void Frustum::Render()
{
	Vector3 dest[5];
	Vector3 position = camera->GetPosition();

	float height = perspective->GetHeight();
	float width = perspective->GetWidth();
	float zn = perspective->GetZn();
	float zf = perspective->GetZf();
	float fov = perspective->GetFov();
	float aspect = width / height;

	Matrix temp;
	Matrix temp2;
	//Camera
	Vector3 front = camera->Foward() * zf ;
	Vector3 Up = camera->Up();
	Vector3 Right = camera->Right();
	Vector3 Down = -Up;
	Vector3 Left = -Right;

	float len = D3DXVec3Length(&front) / cos(fov / Math::PI * 2);
	Vector3 leftSide = front;
	Vector3 rightSide = front;

	D3DXMatrixRotationY(&temp,- fov/Math::PI * 2);
	D3DXVec3TransformCoord(&leftSide, &leftSide, &temp);

	D3DXMatrixRotationY(&temp2,fov / Math::PI * 2);
	D3DXVec3TransformCoord(&rightSide, &rightSide, &temp2);

	leftSide *= (len / zf);
	rightSide *= (len / zf);
	float widthHalf = sqrt(pow(len, 2) - pow(zf, 2));
	float heightHalf = (height / width) * widthHalf;

	//camera zn
	dest[0] = position;
	//왼쪽 위
	dest[1] = front +(Left* widthHalf)+ (Up * heightHalf) + position;
	//오른쪽 위
	dest[2] = front + (Right * widthHalf) + (Up * heightHalf) + position;
	//dest[2] = rightSide + (Up * heightHalf) + position;
	//오른쪽 아래
	dest[3] = front + (Right * widthHalf) + (Down * heightHalf) + position;
	/*dest[3] = rightSide + (Down * heightHalf) + position;*/
	//왼쪽 아래
	dest[4] = front + (Left * widthHalf) + (Down * heightHalf) + position;
	/*dest[4] = leftSide + (Down * heightHalf) + position;*/

	Color color = Color(1, 0, 0, 1);
	DebugLine::Get()->RenderLine(dest[0], dest[1], color);
	DebugLine::Get()->RenderLine(dest[0], dest[2], color);
	DebugLine::Get()->RenderLine(dest[0], dest[3], color);
	DebugLine::Get()->RenderLine(dest[0], dest[4], color);

	//back
	DebugLine::Get()->RenderLine(dest[1], dest[2], color);
	DebugLine::Get()->RenderLine(dest[2], dest[3], color);
	DebugLine::Get()->RenderLine(dest[3], dest[4], color);
	DebugLine::Get()->RenderLine(dest[4], dest[1], color);
}

void Frustum::Planes(Plane * plane)
{
	memcpy(plane, this->planes, sizeof(Plane) * 6);
}

bool Frustum::CheckPoint(Vector3 & position)
{
	for (int i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &position) < 0.0f)
			return false;
	}

	return true;
}

bool Frustum::CheckCube(Vector3 & center, Vector3 & size)
{
	for (int i = 0; i < 6; i++)
	{
		//좌하안
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y - size.y, center.z - size.z)) >= 0.0f)
			continue;

		//우하안
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y - size.y, center.z - size.z)) >= 0.0f)
			continue;

		//좌상안
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y + size.y, center.z - size.z)) >= 0.0f)
			continue;

		//좌하밖
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y - size.y, center.z + size.z)) >= 0.0f)
			continue;

		//우상안
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y + size.y, center.z - size.z)) >= 0.0f)
			continue;

		//우하밖
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y - size.y, center.z + size.z)) >= 0.0f)
			continue;

		//좌상밖
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x - size.x, center.y + size.y, center.z + size.z)) >= 0.0f)
			continue;

		//우상밖
		if (D3DXPlaneDotCoord(&planes[i], &Vector3(center.x + size.x, center.y + size.y, center.z + size.z)) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::CheckCube(Vector3 & center, float radius)
{
	Vector3 check;

	for (int i = 0; i < 6; i++)
	{
		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		return false;
	}

	return true;

}
