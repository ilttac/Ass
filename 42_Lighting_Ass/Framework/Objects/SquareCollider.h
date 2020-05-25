#pragma once

class SquareCollider
{
private:
	struct Square;

public:
	SquareCollider(Transform* transform, Transform* init = NULL);
	~SquareCollider();

	bool IsIntersect(SquareCollider* other);

	void Update();
	void Render(Color color);

	Transform* GetTransform() { return transform; }

private:
	void SetSquare();

private:
	bool Collision(Square& box1, Square& box2);
	Vector3 Cross(Vector3& vec1, Vector3& vec2);


private:
	struct Square
	{
		Vector3 Position;
		float Left;
		float Top;
		float Right;
		float Bottom;

		Vector3 HalfSize;
	}square;

private:
	Transform * init;
	Transform* transform;

	Vector3 lines[4];
};