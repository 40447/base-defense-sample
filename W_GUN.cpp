
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
	
	m_Position = Vector3(0.0f, 1.0f, 0.0f);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	m_Scale = Vector3(5.0f, 5.0f, 5.0f);

	
	m_Mesh.Load("assets/model/00GUN.fbx", "assets/model/");

	
	m_StaticMeshRenderer.Init(m_Mesh);

	
	m_Mesh2.Load("assets/model/00GUN.fbx", "assets/model/");
	m_StaticMeshRenderer2.Init(m_Mesh2);


	
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
	if (!player) return; 

	Vector3* playerPos = player->GetPosition();
	Vector3* playerRot = player->GetRotation();

	Vector3* enemyPos = enemy->GetPosition();
	Vector3* enemyRot = enemy->GetRotation();

	Vector3* gunPos = gun->GetPosition();
	Vector3* gunPos2 = gun->GetPosition2();
	Vector3* gunRot = gun->GetRotation();
	Vector3* gunRot2 = gun->GetRotation2();


	
	Matrix rotationMatrix = Matrix::CreateRotationY(playerRot->y);
	Vector3 rightDirection = Vector3::Right;
	Vector3 offset = Vector3::Transform(rightDirection, rotationMatrix);

	m_Position = *playerPos + offset * GUNDistance;
	m_Position.y = playerPos->y;

	
	m_Rotation.y = playerRot->y;

	
	Matrix gunRotationMatrix = Matrix::CreateRotationY(m_Rotation.y);
	Vector3 forwordDirection = Vector3::Forward;
	Vector3 shootOffset = Vector3::Transform(forwordDirection, gunRotationMatrix) * 40.0f;
	m_ShootPosition = m_Position + shootOffset;





	
	Matrix rotationMatrix2 = Matrix::CreateRotationY(playerRot->y);
	Vector3 leftDirection = Vector3::Left;
	Vector3 offset2 = Vector3::Transform(rightDirection, rotationMatrix);

	m_Position2 = *playerPos + offset2 * -GUNDistance;
	m_Position2.y = playerPos->y;

	
	m_Rotation2.y = playerRot->y;


	Matrix gunRotationMatrix2 = Matrix::CreateRotationY(m_Rotation2.y);
	Vector3 forwardDirection2 = Vector3::Forward;
	Vector3 shootOffset2 = Vector3::Transform(forwardDirection2, gunRotationMatrix2) * 40.0f;

	m_ShootPosition2 = m_Position2 + shootOffset;


	
	m_ShootRotation2 = m_Rotation2;


	static float backupRotationX = m_Rotation.x;
	static float backupRotationX2 = m_Rotation2.x;

	GUNUSE = CDirectInput::GetInstance().GetMouseRButtonCheck();

	if (GUNUSE) {

		const float rotationSpeed = 0.1f;
		float targetRotationx = m_Rotation.x;
		float targetRotationx2 = m_Rotation2.x;


		m_Rotation.x = playerRot->x + (targetRotationx - playerRot->x) * rotationSpeed;
		m_Rotation2.x = playerRot->x + (targetRotationx2 - playerRot->x) * rotationSpeed; 


	}
	else {
		m_Rotation.x = backupRotationX; 
		m_Rotation2.x = backupRotationX2; 

	}

	
	if (enemy)
	{
		float distance = (*enemyPos - *playerPos).Length();
		if (distance < 200.0f)
		{

			for (int i = 0; i < 100; i++) {

				Luck();
				luck->m_LUCKAlive[i] = 1;
				Vector3 lockForward = Vector3(sin(enemyRot->y), 0.0f, cos(enemyRot->y)); 
				lockForward.Normalize(); 

				Vector3 lockPosition = *enemyPos + lockForward*-10.0f; 

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
	
	{
		
		Matrix r1 = Matrix::CreateFromYawPitchRoll(m_Rotation.y, m_Rotation.x, m_Rotation.z);
		Matrix t1 = Matrix::CreateTranslation(m_Position.x, m_Position.y, m_Position.z);
		Matrix s1 = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);

		
		Matrix worldmtx1 = s1 * r1 * t1;

		
		Renderer::SetWorldMatrix(&worldmtx1);

		
		m_Shader.SetGPU();
		m_StaticMeshRenderer.Draw();
	}

	{
		
		Matrix r2 = Matrix::CreateFromYawPitchRoll(m_Rotation2.y, m_Rotation2.x, m_Rotation2.z);
		Matrix t2 = Matrix::CreateTranslation(m_Position2.x, m_Position2.y, m_Position2.z);
		Matrix s2 = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);

		
		Matrix worldmtx2 = s2 * r2 * t2;

	
		Renderer::SetWorldMatrix(&worldmtx2);

		
		m_Shader.SetGPU();
		m_StaticMeshRenderer2.Draw();
	}
}

void GUN::Dispose()
{

}

void GUN::Luck()
{
	
	GUN* gun = GetGUN();
	Enemy* enemy = GetEnemy();
	CPlayer* player = GetPlayer();
	


	
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



	if (!enemy->IsAlive() || player == nullptr || enemy == nullptr) return; 

	const float minLockDistance = 15.0f; 
	
	float distanceToEnemy = (*enemyPos - *playerPos).Length();
	float distanceToEnemy2 = (*enemyPos - *gunPos1).Length();
	float distanceToEnemy3 = (*enemyPos - *gunPos2).Length();


	if (distanceToEnemy2 < minLockDistance && distanceToEnemy3 < minLockDistance)
	{
		return;
	}


	if (CDirectInput::GetInstance().GetMouseRButtonCheck())
	{
	

		
		Vector3 vGUNDir = *enemyPos - *gunPos1;
		vGUNDir.Normalize(); 

		
		Vector3 vGUNDir2 = *enemyPos - *gunPos2;
		vGUNDir2.Normalize(); 

		
		Vector3 vPEDir = *enemyPos - *playerPos;
		vPEDir.Normalize(); 

		
		float angleY = atan2(vGUNDir.x, vGUNDir.z);
		float angleX = atan2(vGUNDir.y, sqrt(vGUNDir.x * vGUNDir.x + vGUNDir.z * vGUNDir.z));

		
		float angleY2 = atan2(vGUNDir2.x, vGUNDir2.z);
		float angleX2 = atan2(vGUNDir2.y, sqrt(vGUNDir2.x * vGUNDir2.x + vGUNDir2.z * vGUNDir2.z));

		float angleY3 = atan2(vPEDir.x, vPEDir.z);
		float angleX3 = atan2(vPEDir.y, sqrt(vPEDir.x * vPEDir.x + vPEDir.z * vPEDir.z));


		
		float targetRotationY = angleY + DirectX::XM_PI;
		float targetRotationY2 = angleY2 + DirectX::XM_PI;
		float targetPlayerRotationY = angleY3 + DirectX::XM_PI;


		const float rotationSpeed = 0.1f; 


		playerRot->y = playerRot->y + (targetPlayerRotationY - playerRot->y) * rotationSpeed;


		m_Rotation.y = angleY + DirectX::XM_PI;
		m_Rotation2.y = angleY2 + DirectX::XM_PI;
	
		
		static int bulletShot = 0; 

	}
}
 
