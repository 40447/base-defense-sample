
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "W_PUNCH.h"
#include "CDirectInput.h"
#include "CPlayer.h"

#include <DirectXMath.h>
#include "Enemy.h"

#include"Explosion.h"


using namespace DirectX;

//=============================================================
// W_PUNCH.cpp
// プレイヤーの近接攻撃（パンチ）制御クラス
// ・左右拳モデルの位置・回転制御
// ・出拳アニメーション
// ・マウス操作による左右交互パンチ処理
// ・AABB（当たり判定）動的生成
//=============================================================
float PUNCHDistance = -20.0f;

// 
static bool isPunching = false;
static bool isPunching2 = false;
static bool DOTPUNCH = false;
// 
static float punchDuration = 0.2f;
// 
static float punchTimer = 0.0f;
static float  deltaTime = 0.016f;
// 
static XMFLOAT3 originalPosition;
static XMFLOAT3 originalPosition2;

void PUNCH::Init()
{
	//  SRT 
	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_Rotation = Vector3(DirectX::XM_PI, 0.0f, 0.0f);
	m_Scale = Vector3(6.0f, 8.0f,6.0f);

	// 
	m_Mesh.Load("assets/model/RPUNCH.fbx", "assets/model/");

	// 
	m_StaticMeshRenderer.Init(m_Mesh);

	
	// 
	m_Mesh2.Load("assets/model/LPUNCH.fbx", "assets/model/");
	m_StaticMeshRenderer2.Init(m_Mesh2);




	// 
	m_Shader.Create(
		"shader/vertexLightingVS.hlsl",
		"shader/vertexLightingPS.hlsl"
	);


	float halfW = 20.0f * 0.5f;
	float halfH = 20.0f * 0.5f;
	float halfD = 20.0f * 0.5f;

	// (AABB) = [min, max]
	m_vAABBMin = Vector3(
		PUNCHPosition.x - halfW,
		PUNCHPosition.y - halfH,
		PUNCHPosition.z - halfD);
	m_vAABBMax = Vector3(
		PUNCHPosition.x + halfW,
		PUNCHPosition.y + halfH,
		PUNCHPosition.z + halfD);

	// 
	m_vAABBMin2 = Vector3(
		PUNCHPosition2.x - halfW,
		PUNCHPosition2.y - halfH,
		PUNCHPosition2.z - halfD);
	m_vAABBMax2 = Vector3(
		PUNCHPosition2.x + halfW,
		PUNCHPosition2.y + halfH,
		PUNCHPosition2.z + halfD);


}

void PUNCH::Update()
{
	// AABB用ハーフサイズ
	float halfW = 20.0f * 0.5f;
	float halfH = 10.0f * 0.5f;
	float halfD = 20.0f * 0.5f;

	// プレイヤー情報取得
	CPlayer *player = GetPlayer();
	Vector3 *playerRot = player->GetRotation();
	Vector3 *playerPos = player->GetPosition();

	// 爆発演出用
	Explosion* explosion = GetExplosion();
	Vector3 expos = *explosion->GetExPositions();

	// 右拳位置をプレイヤーの右方向にオフセット
	Matrix rotationMatrix = Matrix::CreateRotationY(playerRot->y);
	Vector3 rightDirection = Vector3::Right;
	Vector3 offset = Vector3::Transform(rightDirection, rotationMatrix);

	m_Position = *playerPos + offset * -15; // 右拳を左方向に配置
	m_Position.y = playerPos->y;
	m_Rotation.y = playerRot->y;

	// 左拳位置をプレイヤーの左方向にオフセット
	Matrix rotationMatrix2 = Matrix::CreateRotationY(playerRot->y);
	Vector3 leftDirection = Vector3::Left;
	Vector3 offset2 = Vector3::Transform(leftDirection, rotationMatrix);
	m_Position2 = *playerPos + offset2 * PUNCHDistance;
	m_Position2.y = playerPos->y;
	m_Rotation2.y = playerRot->y;

	// 拳の前方向（=攻撃の衝突位置）を計算
	Matrix PUNCHRotationMatrix = Matrix::CreateRotationY(m_Rotation.y);
	Vector3 forwordDirection = Vector3::Forward;
	Vector3 shootOffset = Vector3::Transform(forwordDirection, PUNCHRotationMatrix) * 40.0f;
	PUNCHPosition = m_Position + shootOffset;

	Matrix PUNCHRotationMatrix2 = Matrix::CreateRotationY(m_Rotation2.y);
	Vector3 forwardDirection2 = Vector3::Forward;
	Vector3 shootOffset2 = Vector3::Transform(forwardDirection2, PUNCHRotationMatrix2) * 40.0f;
	PUNCHPosition2 = m_Position2 + shootOffset2;

	// X軸回転を保存（抬拳処理用）
	static float backupRotationX = m_Rotation.x;
	static float backupRotationX2 = m_Rotation2.x;

	// マウス右ボタンで「拳を構える」状態に移行
	if (CDirectInput::GetInstance().GetMouseRButtonCheck()) {
		const float rotationSpeed = 0.1f;
		float targetRotationx = m_Rotation.x;
		float targetRotationx2 = m_Rotation2.x;

		// プレイヤーのX軸角度へ補間（少し上を向くように）
		m_Rotation.x = playerRot->x + (targetRotationx - playerRot->x) * rotationSpeed;
		m_Rotation2.x = playerRot->x + (targetRotationx2 - playerRot->x) * rotationSpeed;

		DOTPUNCH = true;
	}
	else {
		// 離したら元に戻す
		m_Rotation.x = backupRotationX;
		m_Rotation2.x = backupRotationX2;
		DOTPUNCH = false;
	}

	// 左右交互パンチ制御（DOTPUNCHオフ時）
	if (!DOTPUNCH) {
		if (currentPUNCH == 1) {
			UpdatePUNCH(); // 右パンチ
			if (!CDirectInput::GetInstance().GetMouseLButtonCheck() && isPUNCH) {
				isPUNCH = false;
				currentPUNCH = 2; // 次は左
			}
		}
		else if (currentPUNCH == 2) {
			UpdatePUNCH2(); // 左パンチ
			if (!CDirectInput::GetInstance().GetMouseLButtonCheck() && isPUNCH2) {
				isPUNCH2 = false;
				currentPUNCH = 1; // 次は右
			}
		}
	}

	// パンチ（右拳）中：前方に突き出す & AABB更新
	if (isPunching) {
		float angle = m_Rotation.y;
		Vector3 forwardDir = Vector3(sin(angle), 0.0f, cos(angle));
		forwardDir.Normalize();

		float moveSpeed = -50.0f;

		// 拳の位置と当たり判定を前方に移動
		m_Position += forwardDir * moveSpeed;
		PUNCHPosition += forwardDir * moveSpeed;

		m_vAABBMin = Vector3(
			PUNCHPosition.x - halfW,
			PUNCHPosition.y - 10 - halfH,
			PUNCHPosition.z - halfD);
		m_vAABBMax = Vector3(
			PUNCHPosition.x + halfW,
			PUNCHPosition.y - 10 + halfH,
			PUNCHPosition.z + halfD);

		// タイマーで終了を管理
		punchTimer -= deltaTime;
		if (punchTimer <= 0.0f) {
			isPunching = false;
			m_Position = originalPosition;
		}
	}

	// パンチ（左拳）中：同上
	if (isPunching2) {
		float angle = m_Rotation2.y;
		Vector3 forwardDir2 = Vector3(sin(angle), 0.0f, cos(angle));
		forwardDir2.Normalize();

		float moveSpeed2 = -50.0f;

		m_Position2 += forwardDir2 * moveSpeed2;
		PUNCHPosition2 += forwardDir2 * moveSpeed2;

		m_vAABBMin = Vector3(
			PUNCHPosition2.x - halfW,
			PUNCHPosition2.y - 10 - halfH,
			PUNCHPosition2.z - halfD);
		m_vAABBMax = Vector3(
			PUNCHPosition2.x + halfW,
			PUNCHPosition2.y - 10 + halfH,
			PUNCHPosition2.z + halfD);

		punchTimer -= deltaTime;
		if (punchTimer <= 0.0f) {
			isPunching2 = false;
			m_Position2 = originalPosition2;
		}
	}
}


void PUNCH::Draw()
{
	//
	{
		// 
		Matrix r1 = Matrix::CreateFromYawPitchRoll(m_Rotation.y, m_Rotation.x, m_Rotation.z);
		Matrix t1 = Matrix::CreateTranslation(m_Position.x, m_Position.y, m_Position.z);
		Matrix s1 = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);

		// 
		Matrix worldmtx1 = s1 * r1 * t1;

		// 
		Renderer::SetWorldMatrix(&worldmtx1);

		// 
		m_Shader.SetGPU();
		m_StaticMeshRenderer.Draw();
	}

	// 
	{
		
		Matrix r2 = Matrix::CreateFromYawPitchRoll(m_Rotation2.y, m_Rotation2.x, m_Rotation2.z);
		Matrix t2 = Matrix::CreateTranslation(m_Position2.x, m_Position2.y, m_Position2.z);
		Matrix s2 = Matrix::CreateScale(m_Scale2.x, m_Scale2.y, m_Scale2.z);

		// 
		Matrix worldmtx2 = s2 * r2 * t2;

		//
		Renderer::SetWorldMatrix(&worldmtx2);

		// 
		m_Shader.SetGPU();
		m_StaticMeshRenderer2.Draw();
	}
}

void PUNCH::Dispose()
{

}



void PUNCH::UpdatePUNCH() {

	if (CDirectInput::GetInstance().GetMouseLButtonCheck()) {
		// 
		isPunching = true;
		punchTimer = punchDuration;

			// 
		originalPosition = m_Position;

		isPUNCH = true;
	}


}

void PUNCH::UpdatePUNCH2() {
	
	if (CDirectInput::GetInstance().GetMouseLButtonCheck()) {
		
		isPunching2 = true;               
		punchTimer = punchDuration;      

		originalPosition2 = m_Position2; // 元の位置を保存（パンチ終了後に戻すため）

		isPUNCH2 = true;                 // パンチ入力済みフラグ（交互パンチ制御用）
	}
}

