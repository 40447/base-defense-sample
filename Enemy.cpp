
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "Enemy.h"
#include "CDirectInput.h"
#include"CPlaneMesh.h"
#include<iostream>
#include"CPlayer.h"


float desiredFiringDistance = 60;  
float distanceBuffer = 50;         
float movementSpeed;
bool enemyattack = false;

float RandomRange(float minValue, float maxValue)

{
    // rand() 會回傳 [0, RAND_MAX] 間的整數
    // 轉成 0.0 ~ 1.0 間的浮點，再線性映射到 [minValue, maxValue]
    float zeroToOne = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return minValue + (maxValue - minValue) * zeroToOne;
}

using namespace DirectX::SimpleMath;

extern float deltaTime;  


void Enemy::Init()
{
	// モデルの初期化
	m_Mesh.Load(
		"assets/model/enemy001.fbx",				// モデル名
		"assets/model/");						// テクスチャのパス

	//レンダラ初期化
	m_StaticMeshRenderer.Init(m_Mesh);

	// シェーダーの初期化
	m_Shader.Create(
		"shader/vertexLightingVS.hlsl",			// 頂点シェーダー
		"shader/vertexLightingPS.hlsl");		// ピクセルシェーダー

	//m_Position = Vector3(0.0f,-20.0f, 0.0f);
	m_Rotation.x += DirectX::XM_PI / 2;
	m_Scale = Vector3(3.0f, 3.0f, 3.0f);



	float halfW = 20.0f * 0.5f;
	float halfH = 20.0f * 0.5f;
	float halfD = 20.0f * 0.5f;

	// (AABB) = [min, max]
	m_vAABBMin = Vector3(m_Position.x - halfW,
		m_Position.y - halfH,
		m_Position.z - halfD);
	m_vAABBMax = Vector3(m_Position.x + halfW,
		m_Position.y + halfH,
		m_Position.z + halfD);


	Respawn();

	isAlive = true;
}

float deltaTime = 0.016f;

void Enemy::Update()
{


	if (!isAlive) {
		
		respawnTimer += deltaTime;
		//  AABB clear
		m_vAABBMin = Vector3::Zero;
		m_vAABBMax = Vector3::Zero;


		//m_Position = Vector3::Zero;


		
		if (respawnTimer >= respawnDelay) {
			respawnTimer = 0.0f; 
			Respawn();
		}
		return; 
	}

	CPlayer* player = GetPlayer();
	
    if (!player)
    {
        
        m_State = EnemyState::Idle;
    }
    else
    {
        Vector3 playerPos = *player->GetPosition();
        Vector3 toPlayer = playerPos - m_Position;
        float distanceToPlayer = toPlayer.Length();

       
        switch (m_State)
        {
        case EnemyState::Idle:
        {
			enemyattack = false;
            //do nothing 
            // <300， Chase
            if (distanceToPlayer < 300.0f)
            {
                m_State = EnemyState::Chase;
				
            }
            break;
        }

        case EnemyState::Patrol:

        {
			enemyattack = false;
			//std::cout << "Enemy is patrolling!" << std::endl;

          

            // no player
            if (!m_HasPatrolTarget)
            {
                // 在圓形範圍內取得隨機角度與半徑
                float angle = RandomRange(0.0f, DirectX::XM_2PI);
                float radius = RandomRange(0.0f, m_PatrolRange);

                // 計算 XZ 平面位置
                float randomX = cosf(angle) * radius;
                float randomZ = sinf(angle) * radius;

                // 設定給 m_PatrolTarget (Y 可以視需要做高度處理)
                m_PatrolTarget.x = randomX;
                m_PatrolTarget.y = 0.0f;
                m_PatrolTarget.z = randomZ;

                m_HasPatrolTarget = true;
            }
            else
            {
                // 有目標，朝目標前進
                Vector3 dir = m_PatrolTarget - m_Position;
                float dist = dir.Length();
               
                // 若已接近目標 (dist < 10)，就重置以便重新選目標
                if (dist < 10.0f)
                {
                    m_HasPatrolTarget = false;
                }
                else
                {
                    // 移動並面向目標
                    dir.Normalize();
                    m_Position += dir * (m_PatrolSpeed * deltaTime);

                    float yaw = atan2f(dir.x, dir.z);
                    yaw += DirectX::XM_PI; // 若模型需要轉 180 度修正
                    m_Rotation.y = yaw;
                }
            }

            // 若偵測到玩家進入一定範圍，就切換到追擊狀態
            if (distanceToPlayer < engageDistance)
            {
                m_State = EnemyState::Chase;
            }
            break;
        }

        case EnemyState::Chase:
        {
			enemyattack = false;
            if (distanceToPlayer > attackDistance + distanceBuffer)
            {
                toPlayer.y = 0.0f;
                toPlayer.Normalize();
                m_Position += toPlayer * (moveSpeed * deltaTime);

                float yaw = atan2f(toPlayer.x, toPlayer.z);
                yaw += DirectX::XM_PI;
                m_Rotation.y = yaw;
            }
            else if (distanceToPlayer < attackDistance - distanceBuffer)
            {
                m_State = EnemyState::Retreat;
            }




            if (distanceToPlayer <= SHOOTDistance + distanceBuffer &&
                distanceToPlayer >= attackDistance - distanceBuffer)
            {
                Vector3 attackDir = toPlayer;
                attackDir.y = 0.0f;
                attackDir.Normalize();
                float yaw = atan2f(attackDir.x, attackDir.z);
                yaw += DirectX::XM_PI;
                m_Rotation.y = yaw;

                attackTimer += deltaTime;
                if (attackTimer >= attackInterval)
                {
                    attackTimer = 0.0f;
                  //  std::cout << "Enemy attacks the player while chasing!" << std::endl;


					enemyattack = true;


                }
            }
            break;
        }

        case EnemyState::Attack:
        {
            // 面向玩家
            toPlayer.y = 0.0f;
            toPlayer.Normalize();
            float yaw = atan2f(toPlayer.x, toPlayer.z);
            yaw += DirectX::XM_PI;
            m_Rotation.y = yaw;

            // 執行攻擊（含攻擊冷卻計時器）
         
				enemyattack = true;

			//	std::cout << "Enemy attacks the player!" << std::endl;

            // 攻擊狀態下檢查距離
            if (distanceToPlayer > attackDistance + distanceBuffer)
            {
                // 太遠 -> 回到 Chase
                m_State = EnemyState::Chase;
            }
            else if (distanceToPlayer < attackDistance - distanceBuffer)
            {
                // 太近 -> Retreat
                m_State = EnemyState::Retreat;
            }
            break;
        }

        case EnemyState::Retreat:
        {

            // 與玩家距離過近 -> 往相反方向移動
            Vector3 fromPlayer = m_Position - playerPos;
            fromPlayer.y = 0.0f;
            fromPlayer.Normalize();

            m_Position += fromPlayer * (retreatSpeed * deltaTime);
			//std::cout << "Enemy retreats from the player!" << std::endl;

            // 面向與玩家相反 (要不要轉頭看玩家可視需求調整)
            float yaw = atan2f(fromPlayer.x, fromPlayer.z);
            yaw += DirectX::XM_PI;
            m_Rotation.y = yaw;

            // 如果退到攻擊距離外，就改成 Chase 或 Attack
            float newDist = (playerPos - m_Position).Length();
            if (newDist > attackDistance + distanceBuffer)
            {
                m_State = EnemyState::Chase;
            }
            else if (newDist <= attackDistance)
            {
                m_State = EnemyState::Attack;
            }
            break;
        }
        } // switch (m_State)
    }
	float halfW = 30.0f * 0.5f;
	float halfH = 100.0f * 0.5f;
	float halfD = 30.0f * 0.5f;
	m_vAABBMin = Vector3(m_Position.x - halfW,
		m_Position.y - halfH,
		m_Position.z - halfD);
	m_vAABBMax = Vector3(m_Position.x + halfW,
		m_Position.y + halfH,
		m_Position.z + halfD);

}
//


//

//
//




void Enemy::Draw()
{

	if (!isAlive) return;


	// SRT情報作成
	Matrix r =
		Matrix::CreateFromYawPitchRoll(
			m_Rotation.y,
			m_Rotation.x,
			m_Rotation.z);

	Matrix t = Matrix::CreateTranslation(
		m_Position.x,
		m_Position.y,
		m_Position.z);

	Matrix s = Matrix::CreateScale(
		m_Scale.x,
		m_Scale.y,
		m_Scale.z);

	Matrix worldmtx;

	worldmtx = s * r * t;

	Renderer::SetWorldMatrix(&worldmtx);		// GPUにセット

	m_Shader.SetGPU();

	m_StaticMeshRenderer.Draw();


}
void Enemy::Respawn()
{
	// 1) 隨機產生 0 ~ 2*PI 的角度
	float angle = ((float)rand() / RAND_MAX) * DirectX::XM_2PI;

	// 2) 半徑 (圓周半徑 500)
	float radius = 500.0f;

	// 3) 設置敵人新位置 (XZ 平面上)
	m_Position.x = cosf(angle) * radius;
	m_Position.y = 0.0f;           // 若需要高度，請依需求調整
	m_Position.z = sinf(angle) * radius;

	// 重置 AABB
	float halfW = 10.0f * 0.5f;
	float halfH = 20.0f * 0.5f;
	float halfD = 10.0f * 0.5f;
	m_vAABBMin = Vector3(m_Position.x - halfW,
		m_Position.y - halfH,
		m_Position.z - halfD);
	m_vAABBMax = Vector3(m_Position.x + halfW,
		m_Position.y + halfH,
		m_Position.z + halfD);

	// 設置為存活狀態
	isAlive = true;

	///////////////
	m_State = EnemyState::Patrol;

	////////////////////
	//std::cout << "敵人已重新生成" << std::endl;
}
void Enemy::Dispose()
{

}