
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "W_GUN.h"
#include "CDirectInput.h"
#include "CPlayer.h"

#include <DirectXMath.h>
#include "Enemy.h"
#include"LUCK.h"


using namespace DirectX;

bool GUNUSE = false;

using namespace DirectX;
float GUNDistance = -20.0f;





void GUN::Init()
{
	// 初始化 SRT 信息
	m_Position = Vector3(0.0f, 1.0f, 0.0f);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	m_Scale = Vector3(5.0f, 5.0f, 5.0f);

	// 加載模型
	m_Mesh.Load("assets/model/00GUN.fbx", "assets/model/");

	// 初始化渲染器
	m_StaticMeshRenderer.Init(m_Mesh);

	// 第二把槍初始化
	m_Mesh2.Load("assets/model/00GUN.fbx", "assets/model/");
	m_StaticMeshRenderer2.Init(m_Mesh2);


	// 初始化著色器
	m_Shader.Create(
		"shader/vertexLightingVS.hlsl",
		"shader/vertexLightingPS.hlsl"
	);

	
}

void GUN::Update()
{

	CPlayer* player = GetPlayer();
	Enemy* enemy = GetEnemy();
	GUN* gun = GetGUN();
	LUCK* luck = GetLUCK();
	if (!player) return; // 確保玩家對象存在

	Vector3* playerPos = player->GetPosition();
	Vector3* playerRot = player->GetRotation();

	Vector3* enemyPos = enemy->GetPosition();
	Vector3* enemyRot = enemy->GetRotation();

	Vector3* gunPos = gun->GetPosition();
	Vector3* gunPos2 = gun->GetPosition2();
	Vector3* gunRot = gun->GetRotation();
	Vector3* gunRot2 = gun->GetRotation2();


	// 計算 GUN 的位置
	Matrix rotationMatrix = Matrix::CreateRotationY(playerRot->y);
	Vector3 rightDirection = Vector3::Right;
	Vector3 offset = Vector3::Transform(rightDirection, rotationMatrix);

	m_Position = *playerPos + offset * GUNDistance;
	m_Position.y = playerPos->y;

	// 設置 GUN 的旋轉
	m_Rotation.y = playerRot->y;

	// 計算槍口位置
	Matrix gunRotationMatrix = Matrix::CreateRotationY(m_Rotation.y);
	Vector3 forwordDirection = Vector3::Forward;
	Vector3 shootOffset = Vector3::Transform(forwordDirection, gunRotationMatrix) * 40.0f;
	m_ShootPosition = m_Position + shootOffset;





	// 更新第二把槍的位置與旋轉
	Matrix rotationMatrix2 = Matrix::CreateRotationY(playerRot->y);
	Vector3 leftDirection = Vector3::Left;
	Vector3 offset2 = Vector3::Transform(rightDirection, rotationMatrix);

	m_Position2 = *playerPos + offset2 * -GUNDistance;
	m_Position2.y = playerPos->y;

	// 設置 GUN 的旋轉
	m_Rotation2.y = playerRot->y;


	// 計算槍口位置
	Matrix gunRotationMatrix2 = Matrix::CreateRotationY(m_Rotation2.y);
	Vector3 forwardDirection2 = Vector3::Forward;
	Vector3 shootOffset2 = Vector3::Transform(forwardDirection2, gunRotationMatrix2) * 40.0f;

	m_ShootPosition2 = m_Position2 + shootOffset;


	// 設置槍口旋轉
	m_ShootRotation2 = m_Rotation2;

	// 處理瞄準模式
	static float backupRotationX = m_Rotation.x;
	static float backupRotationX2 = m_Rotation2.x;

	GUNUSE = CDirectInput::GetInstance().GetMouseRButtonCheck();

	if (GUNUSE) {

		const float rotationSpeed = 0.1f;
		float targetRotationx = m_Rotation.x;
		float targetRotationx2 = m_Rotation2.x;


		m_Rotation.x = playerRot->x + (targetRotationx - playerRot->x) * rotationSpeed; // 瞄準模式旋轉
		m_Rotation2.x = playerRot->x + (targetRotationx2 - playerRot->x) * rotationSpeed; // 瞄準模式旋轉 // 瞄準模式旋轉


	}
	else {
		m_Rotation.x = backupRotationX; // 還原旋轉

		m_Rotation2.x = backupRotationX2; // 還原旋轉

	}

	// 計算玩家與敵人的距離
	if (enemy)
	{
		float distance = (*enemyPos - *playerPos).Length();
		if (distance < 200.0f)
		{

			for (int i = 0; i < 100; i++) {

				Luck();
				luck->m_LUCKAlive[i] = 1;
				Vector3 lockForward = Vector3(sin(enemyRot->y), 0.0f, cos(enemyRot->y)); // 根據玩家的旋轉角度計算前方向
				lockForward.Normalize(); // 確保方向向量為單位向量

				Vector3 lockPosition = *enemyPos + lockForward*-10.0f; // 玩家前方 10 單位的位置[

                  luck->m_LUCKPos[i] = lockPosition;
				  luck->m_LUCKPos[i].y = lockPosition.y +5;
			}
		}
		else {
			for (int i = 0; i < 100; i++) {
				luck->m_LUCKAlive[i] = 0;
			}
		}
		// -----
	}
}

	



void GUN::Draw()
{
	// 繪製第一把槍
	{
		// SRT信息計算
		Matrix r1 = Matrix::CreateFromYawPitchRoll(m_Rotation.y, m_Rotation.x, m_Rotation.z);
		Matrix t1 = Matrix::CreateTranslation(m_Position.x, m_Position.y, m_Position.z);
		Matrix s1 = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);

		// 世界矩陣計算
		Matrix worldmtx1 = s1 * r1 * t1;

		// 設置世界矩陣到GPU
		Renderer::SetWorldMatrix(&worldmtx1);

		// 設置著色器並繪製第一把槍
		m_Shader.SetGPU();
		m_StaticMeshRenderer.Draw();
	}

	// 繪製第二把槍
	{
		// SRT信息計算
		Matrix r2 = Matrix::CreateFromYawPitchRoll(m_Rotation2.y, m_Rotation2.x, m_Rotation2.z);
		Matrix t2 = Matrix::CreateTranslation(m_Position2.x, m_Position2.y, m_Position2.z);
		Matrix s2 = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);

		// 世界矩陣計算
		Matrix worldmtx2 = s2 * r2 * t2;

		// 設置世界矩陣到GPU
		Renderer::SetWorldMatrix(&worldmtx2);

		// 設置著色器並繪製第二把槍
		m_Shader.SetGPU();
		m_StaticMeshRenderer2.Draw();
	}
}

void GUN::Dispose()
{

}

void GUN::Luck()
{
	// 1. 確保必要的物件存在
	GUN* gun = GetGUN();
	Enemy* enemy = GetEnemy();
	CPlayer* player = GetPlayer();
	


	// 2. 取得玩家與敵人、槍的相關位置與旋轉
	Vector3* playerPos = player->GetPosition();
	Vector3* playerRot = player->GetRotation();
	Vector3* enemyPos = enemy->GetPosition();
	Vector3*enemyRot= enemy->GetRotation();
	Vector3* gunShootPos1 = gun->GetShootPosition();
	Vector3* gunShootPos2 = gun->GetShootPosition2();
	Vector3* gunPos2 = gun->GetPosition2();
	Vector3* gunPos1 = gun->GetPosition();
	Vector3* gunRot = gun->GetRotation();
	Vector3* gunRot2 = gun->GetRotation2();



	if (!enemy->IsAlive() || player == nullptr || enemy == nullptr) return; // 檢查指標是否有效


	// 設置最小觸發距離閾值
	const float minLockDistance = 15.0f; // 設定敵人與玩家的最小距離

	// 計算玩家到敵人的距離
	float distanceToEnemy = (*enemyPos - *playerPos).Length();
	float distanceToEnemy2 = (*enemyPos - *gunPos1).Length();
	float distanceToEnemy3 = (*enemyPos - *gunPos2).Length();


	if (distanceToEnemy2 < minLockDistance && distanceToEnemy3 < minLockDistance)
	{
		return; // 如果距離過近，退出不觸發鎖定
	}


	if (CDirectInput::GetInstance().GetMouseRButtonCheck())
	{
	

		// 計算玩家到敵人的方向向量
		Vector3 vGUNDir = *enemyPos - *gunPos1;
		vGUNDir.Normalize(); // 正規化

		// 計算玩家到敵人的方向向量
		Vector3 vGUNDir2 = *enemyPos - *gunPos2;
		vGUNDir2.Normalize(); // 正規化

		// 計算玩家到敵人的方向向量
		Vector3 vPEDir = *enemyPos - *playerPos;
		vPEDir.Normalize(); // 正規化


		// 計算 Y 軸旋轉角度
		float angleY = atan2(vGUNDir.x, vGUNDir.z);
		float angleX = atan2(vGUNDir.y, sqrt(vGUNDir.x * vGUNDir.x + vGUNDir.z * vGUNDir.z));

		// 計算 Y 軸旋轉角度
		float angleY2 = atan2(vGUNDir2.x, vGUNDir2.z);
		float angleX2 = atan2(vGUNDir2.y, sqrt(vGUNDir2.x * vGUNDir2.x + vGUNDir2.z * vGUNDir2.z));

		float angleY3 = atan2(vPEDir.x, vPEDir.z);
		float angleX3 = atan2(vPEDir.y, sqrt(vPEDir.x * vPEDir.x + vPEDir.z * vPEDir.z));


		// 目標角度在允許範圍內
		float targetRotationY = angleY + DirectX::XM_PI;
		float targetRotationY2 = angleY2 + DirectX::XM_PI;
		float targetPlayerRotationY = angleY3 + DirectX::XM_PI;


		const float rotationSpeed = 0.1f; // 每幀旋轉比例（越小越慢）


		playerRot->y = playerRot->y + (targetPlayerRotationY - playerRot->y) * rotationSpeed;


		m_Rotation.y = angleY + DirectX::XM_PI;
		m_Rotation2.y = angleY2 + DirectX::XM_PI;
	
		
		static int bulletShot = 0; // 保留子彈發射計數邏輯

	}
}
 