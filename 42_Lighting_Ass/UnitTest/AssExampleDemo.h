#pragma once

#include "Systems/IExecute.h"

class AssExampleDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {};
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {};
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void Mesh();
	void Michelle();
	void Hallin();
	void Archer();
	void Pass(UINT mesh, UINT model, UINT anim);
	
	void PlayerTracking(ModelAnimator* monster, ModelAnimator* player);
	void MonsterPatrol(ModelAnimator* monster, bool& mbPatrolState,float& randomX,float& randomZ);
	void RotateTowards(ModelAnimator* modelAnim, Vector3 targetPos);
	void MonsterAttack(ModelAnimator* monster, ModelAnimator* player,string monsterName);
	void PlayerMove();
	void PlayerWeaponChange();
private:
	const static UINT arrowCount = 100;
	Shader * shader;
	
	class SkyCube* sky;

	Material* floor;
	MeshRender* grid;
	
	Model* weapon;
	ModelRender* archerWeapon[arrowCount];
	Transform* archerWeaponTransform;
	Vector3 arrowNorMal[arrowCount];

	ModelAnimator* michelle = NULL;
	ModelAnimator* hallin = NULL;
	ModelAnimator* archer = NULL;

	UINT weaponCount;

	//Monster 가 여러명이라면 index 로 관리
	float ranX[10] = { 0.0f , };
	float ranZ[10] = { 0.0f ,};

	

	Vector3 cameraPos;


	bool ColliderRenderSwitchState;

	bool bPatrolState; //순찰중이면 ranX,Z의 값을 바꾸면 안됨.
	bool bSearchState; //search range에 들어오면 true
	bool bAttakRangeState;

	//archer
	bool bArcherPatrolState;	//순찰중이면 ranX,Z의 값을 바꾸면 안됨.
	bool bArcherSearchState;	//search range에 들어오면 true
	bool bArcherAttakRangeState;


	bool bWeaponcolliderState;
	bool bWeaponArcherObbState;
	bool bPlayerAttackState;
	
	enum CameraState
	{
		eCameraIdle = 0,
		eCameraLeft ,
		eCameraTop ,
		eCameraRight,
		eCameraBottom
	};

	CameraState cameraState;

	//player
	struct ColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}collider[4];

	struct PlayerObbCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}playerObbCollider[4];
	struct WeaponCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}weaponCollider[4];
	
	//hallin
	struct HallinObbCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}hallinObbCollider[4];
	struct MColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}mcollider[4];

	struct MAttackColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}mAtkcollider[4];

	//Archer
	struct ArcherObbColider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}archerObbColider[4];
	struct ArcherAttackCollider
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}archerAtkCollider[4];	

	struct ArcherSearchCollider
	{
		Transform* Init;
		Transform* Transform;
		SquareCollider* Collider;
	}archerSerachCollider[4];
	struct ArcherArrowCollider
	{
		Transform* Init;
		Transform* Transform;
		Collider* Collider;
	}archerArrowCollider[4];
	///////////

	vector<MeshRender*> meshes;
	vector<ModelRender*> models;
	vector<ModelAnimator*> animators;
};