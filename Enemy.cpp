
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "Enemy.h"
#include "CDirectInput.h"
#include"CPlaneMesh.h"
#include<iostream>
#include"CPlayer.h"

//============================================================
// 敵キャラクターのAIロジック（ステートマシン）
// 状態：Idle → Patrol → Chase → Attack / Retreat 切替
//============================================================

float desiredFiringDistance = 60;  
float distanceBuffer = 50;         
float movementSpeed;
bool enemyattack = false;

float RandomRange(float minValue, float maxValue)

{
 
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
    // 敵が死亡中の場合の処理
    if (!isAlive) {
        respawnTimer += deltaTime;

        // AABB をリセット（当たり判定無効化）
        m_vAABBMin = Vector3::Zero;
        m_vAABBMax = Vector3::Zero;

        // 一定時間経過で再出現
        if (respawnTimer >= respawnDelay) {
            respawnTimer = 0.0f;
            Respawn();  // 敵をリセット・再配置
        }
        return;
    }

    // プレイヤー情報の取得
    CPlayer* player = GetPlayer();

    if (!player)
    {
        // プレイヤーがいなければ待機
        m_State = EnemyState::Idle;
    }
    else
    {
        // プレイヤーまでの距離を計算
        Vector3 playerPos = *player->GetPosition();
        Vector3 toPlayer = playerPos - m_Position;
        float distanceToPlayer = toPlayer.Length();

        // 敵の現在の状態による行動切替
        switch (m_State)
        {
        // 【待機状態】何もしない。プレイヤーが近づくと追跡開始
        case EnemyState::Idle:
        {
            enemyattack = false;

            // プレイヤーが300以内に入ったら追跡開始
            if (distanceToPlayer < 300.0f) {
                m_State = EnemyState::Chase;
            }
            break;
        }

        // 【巡回状態】範囲内をランダム移動し、プレイヤーが来たら切替
        case EnemyState::Patrol:
        {
            enemyattack = false;

            if (!m_HasPatrolTarget)
            {
                // ランダムな角度・距離から移動先を生成
                float angle = RandomRange(0.0f, DirectX::XM_2PI);
                float radius = RandomRange(0.0f, m_PatrolRange);

                float randomX = cosf(angle) * radius;
                float randomZ = sinf(angle) * radius;

                m_PatrolTarget = Vector3(randomX, 0.0f, randomZ);
                m_HasPatrolTarget = true;
            }
            else
            {
                // 目標地点まで移動
                Vector3 dir = m_PatrolTarget - m_Position;
                float dist = dir.Length();

                if (dist < 10.0f) {
                    m_HasPatrolTarget = false;  // 到達したら次へ
                } else {
                    dir.Normalize();
                    m_Position += dir * (m_PatrolSpeed * deltaTime);

                    // 向き補正
                    float yaw = atan2f(dir.x, dir.z);
                    yaw += DirectX::XM_PI;
                    m_Rotation.y = yaw;
                }
            }

            // プレイヤー接近で追跡に切替
            if (distanceToPlayer < engageDistance) {
                m_State = EnemyState::Chase;
            }
            break;
        }

        // 【追跡状態】プレイヤーとの距離を維持しつつ接近または攻撃
        case EnemyState::Chase:
        {
            enemyattack = false;

            if (distanceToPlayer > attackDistance + distanceBuffer)
            {
                // プレイヤー方向へ移動
                toPlayer.y = 0.0f;
                toPlayer.Normalize();
                m_Position += toPlayer * (moveSpeed * deltaTime);

                // 向き補正
                float yaw = atan2f(toPlayer.x, toPlayer.z);
                yaw += DirectX::XM_PI;
                m_Rotation.y = yaw;
            }
            // プレイヤーが近すぎたら後退状態へ
            else if (distanceToPlayer < attackDistance - distanceBuffer)
            {
                m_State = EnemyState::Retreat;
            }

            // 攻撃可能距離内なら射撃処理
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
                    enemyattack = true;  // 弾を発射するフラグ
                }
            }
            break;
        }

        // 【攻撃状態】その場で射撃、距離によって移動切替
        case EnemyState::Attack:
        {
            // プレイヤーの方向に向ける
            toPlayer.y = 0.0f;
            toPlayer.Normalize();
            float yaw = atan2f(toPlayer.x, toPlayer.z);
            yaw += DirectX::XM_PI;
            m_Rotation.y = yaw;

            enemyattack = true;

            // 状況に応じて追跡 or 後退に遷移
            if (distanceToPlayer > attackDistance + distanceBuffer) {
                m_State = EnemyState::Chase;
            } else if (distanceToPlayer < attackDistance - distanceBuffer) {
                m_State = EnemyState::Retreat;
            }
            break;
        }

        // 【後退状態】プレイヤーとの距離が近すぎる場合に後ろへ下がる
        case EnemyState::Retreat:
        {
            Vector3 fromPlayer = m_Position - playerPos;
            fromPlayer.y = 0.0f;
            fromPlayer.Normalize();

            m_Position += fromPlayer * (retreatSpeed * deltaTime);

            float yaw = atan2f(fromPlayer.x, fromPlayer.z);
            yaw += DirectX::XM_PI;
            m_Rotation.y = yaw;

            // 攻撃距離に戻ったら状態を切り替え
            float newDist = (playerPos - m_Position).Length();
            if (newDist > attackDistance + distanceBuffer) {
                m_State = EnemyState::Chase;
            } else if (newDist <= attackDistance) {
                m_State = EnemyState::Attack;
            }
            break;
        }

        } // switch (m_State)
    }

    // 当たり判定ボックス（AABB）の更新
    float halfW = 30.0f * 0.5f;
    float halfH = 100.0f * 0.5f;
    float halfD = 30.0f * 0.5f;
    m_vAABBMin = Vector3(
        m_Position.x - halfW,
        m_Position.y - halfH,
        m_Position.z - halfD
    );
    m_vAABBMax = Vector3(
        m_Position.x + halfW,
        m_Position.y + halfH,
        m_Position.z + halfD
    );
}




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
	
	float angle = ((float)rand() / RAND_MAX) * DirectX::XM_2PI;

	float radius = 500.0f;

	
	m_Position.x = cosf(angle) * radius;
	m_Position.y = 0.0f;           
	m_Position.z = sinf(angle) * radius;

	
	float halfW = 10.0f * 0.5f;
	float halfH = 20.0f * 0.5f;
	float halfD = 10.0f * 0.5f;
	m_vAABBMin = Vector3(m_Position.x - halfW,
		m_Position.y - halfH,
		m_Position.z - halfD);
	m_vAABBMax = Vector3(m_Position.x + halfW,
		m_Position.y + halfH,
		m_Position.z + halfD);

	
	isAlive = true;

	///////////////
	m_State = EnemyState::Patrol;

	////////////////////
	
}
void Enemy::Dispose()
{

}
