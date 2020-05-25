#include "Framework.h"
#include "SquareCollider.h"

SquareCollider::SquareCollider(Transform* transform, Transform* init)
	: transform(transform), init(init)
{
	lines[0] = Vector3(-0.5f, 0.0f, -0.5f);
	lines[1] = Vector3(-0.5f, 0.0f, +0.5f);
	lines[2] = Vector3(+0.5f, 0.0f, -0.5f);
	lines[3] = Vector3(+0.5f, 0.0f, +0.5f);

}

SquareCollider::~SquareCollider()
{
}

bool SquareCollider::IsIntersect(SquareCollider * other)
{
	return Collision(this->square, other->square);
}

void SquareCollider::Update()
{
	SetSquare();
}

void SquareCollider::Render(Color color)
{
	Vector3 dest[4];

	Transform temp;
	temp.World(transform->World());

	if (init != NULL)
		temp.World(init->World() * transform->World());

	Matrix world = temp.World();

	for (UINT i = 0; i < 4; i++)
		D3DXVec3TransformCoord(&dest[i], &lines[i], &world);

	//Front
	DebugLine::Get()->RenderLine(dest[0], dest[1], color);
	DebugLine::Get()->RenderLine(dest[1], dest[3], color);
	DebugLine::Get()->RenderLine(dest[3], dest[2], color);
	DebugLine::Get()->RenderLine(dest[2], dest[0], color);
}

void SquareCollider::SetSquare()
{
	Transform temp;
	temp.World(transform->World());

	if (init != NULL)
		temp.World(init->World() * transform->World());

	temp.Position(&square.Position);

	Vector3 scale;
	temp.Scale(&scale);
	square.HalfSize = scale * 0.5f;

	square.Left = square.Position.x - scale.x * 0.5f;
	square.Top = square.Position.z + scale.z * 0.5f;
	square.Right = square.Position.x + scale.x * 0.5f;
	square.Bottom = square.Position.z - scale.z * 0.5f;
}



bool SquareCollider::Collision(Square & box1, Square & box2)
{
	return box1.Left < box2.Right
		&& box1.Right > box2.Left
		&& box1.Top > box2.Bottom
		&& box1.Bottom < box2.Top;
}

Vector3 SquareCollider::Cross(Vector3 & vec1, Vector3 & vec2)
{
	float x = vec1.y * vec2.z - vec1.z * vec2.y;
	float y = vec1.z * vec2.x - vec1.x * vec2.z;
	float z = vec1.x * vec2.y - vec1.y * vec2.x;

	return Vector3(x, y, z);
}
