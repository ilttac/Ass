#pragma once
#include "Framework.h"

class ParticleData
{
public:
	enum class BlendType
	{
		Opaque = 0,
		Additive,
		AlphaBlen
	}Type = BlendType::Opaque;

	bool bLoop = false;

	wstring TextureFile = L"";

	float Readytime = 1.0f;
	float ReadyRandomTime = 0;

	float StartVelocity = 1;//시작
	float EndVelocity = 1;//종료

	float MinHorizontalVelocity = 0.0f;
	float MaxHorizontalVelocity = 0.0f;

	float MinVerticalVelocity = 0.0f;
	float MaxVerticalVelocity = 0.0f;

	Vector3 Gravity = Vector3(0, 0, 0);

	Color MinColor = Color(1, 1, 1, 1);
	Color MaxColor = Color(1, 1, 1, 1);

	float MinRotateSpeed = 0;
	float MaxRotateSpeed = 0;

	float MinStartSize = 100;
	float MaxStartSize = 100;

	float MinEndSize = 100;
	float MaxEndSize = 100;
};
