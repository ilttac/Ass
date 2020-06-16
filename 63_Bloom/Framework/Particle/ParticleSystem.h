#pragma once

class ParticleSystem : public Renderer
{
public:
	ParticleSystem(wstring file);//xml 파일
	~ParticleSystem();

	void Reset();

	void Add(Vector3& position);
public:
	void Update();

private:
	void MapVertices();//맵으로열어서 정점정보 업데이트
	void Activate();
	void Deactivate();

public:
	void Render();

public:
	ParticleData& GetData() { return data; }
	void SetTexture(wstring file)
	{
		SafeDelete(map);
		map = new Texture(file);
	}
private:
	void ReadFile(wstring file);

private:
	struct VertexParticle
	{
		Vector3 Position;
		Vector2 Corner;
		Vector3 Velocity;
		Vector4 Random;//x:주기,y:크기,z:회전정보,w:색상정보
		float Time;//재생시간
	};

private:
	struct Desc
	{
		Color MinColor;
		Color MaxColor;

		Vector3 Gravity;
		float EndVelocity;

		Vector2 StartSize;
		Vector2 EndSize;
		
		Vector2 RotateSpeed;
		float ReadyTime;
		float ReadyRandomTime;

		float CurrentTime;
		float Padding[3];
	}desc;

private:
	ParticleData data;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	Texture* map = NULL;
	ID3DX11EffectShaderResourceVariable* sMap;

	VertexParticle* vertices = NULL;
	UINT* indices = NULL;

	float currentTime = 0.0f;
	float lastAddTime = 0.0f;//한번에 업데이트 못하게 하기위함

	UINT leadCount = 0; //현재운영중인 파티클 개수
	UINT gpuCount = 0; //실제로 보낼 파티클 개수

	UINT activeCount = 0; //쉐이더 로보낸것중에  활성화할 갯수 
	UINT deactiveCount = 0; //하나씩 비활성화될 변수

};