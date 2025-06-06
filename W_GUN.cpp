
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "W_GUN.h"
#include "CDirectInput.h"
#include "CPlayer.h"

#include <DirectXMath.h>
#include "Enemy.h"
#include"LUCK.h"

//=============================================================
// W_GUN.cpp
// プレイヤーの両腕に装着された銃の制御クラス
// ・プレイヤーの位置／回転に合わせた銃モデルの更新
// ・右クリックでロックオン処理
// ・LUCK（追尾型ロックエフェクト）のトリガー
//=============================================================
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
    // プレイヤー・敵・武器・エフェクト参照取得
    CPlayer* player = GetPlayer();
    Enemy* enemy = GetEnemy();
    GUN* gun = GetGUN();
    LUCK* luck = GetLUCK();

    if (!player) return;

    // 座標・回転情報のポインタ取得
    Vector3* playerPos = player->GetPosition();
    Vector3* playerRot = player->GetRotation();

    Vector3* enemyPos = enemy->GetPosition();
    Vector3* enemyRot = enemy->GetRotation();

    Vector3* gunPos = gun->GetPosition();
    Vector3* gunPos2 = gun->GetPosition2();
    Vector3* gunRot = gun->GetRotation();
    Vector3* gunRot2 = gun->GetRotation2();

    // プレイヤーの右方向へオフセットを計算（右手用銃）
    Matrix rotationMatrix = Matrix::CreateRotationY(playerRot->y);
    Vector3 rightDirection = Vector3::Right;
    Vector3 offset = Vector3::Transform(rightDirection, rotationMatrix);

    // プレイヤーの右側に銃を配置
    m_Position = *playerPos + offset * GUNDistance;
    m_Position.y = playerPos->y;

    // 銃のY軸回転をプレイヤーに合わせる
    m_Rotation.y = playerRot->y;

    // 銃口位置を前方方向に40単位分ずらして設定
    Matrix gunRotationMatrix = Matrix::CreateRotationY(m_Rotation.y);
    Vector3 forwordDirection = Vector3::Forward;
    Vector3 shootOffset = Vector3::Transform(forwordDirection, gunRotationMatrix) * 40.0f;
    m_ShootPosition = m_Position + shootOffset;

    // --- 左手用銃の処理（位置2、回転2）---

    Matrix rotationMatrix2 = Matrix::CreateRotationY(playerRot->y);
    Vector3 leftDirection = Vector3::Left;
    Vector3 offset2 = Vector3::Transform(rightDirection, rotationMatrix);  // ← ← ← ❗❗可能寫錯？應該用 leftDirection 才對

    m_Position2 = *playerPos + offset2 * -GUNDistance;
    m_Position2.y = playerPos->y;

    m_Rotation2.y = playerRot->y;

    Matrix gunRotationMatrix2 = Matrix::CreateRotationY(m_Rotation2.y);
    Vector3 forwardDirection2 = Vector3::Forward;
    Vector3 shootOffset2 = Vector3::Transform(forwardDirection2, gunRotationMatrix2) * 40.0f;
    m_ShootPosition2 = m_Position2 + shootOffset;

    m_ShootRotation2 = m_Rotation2;

    // 抬槍旋轉用のX軸バックアップ
    static float backupRotationX = m_Rotation.x;
    static float backupRotationX2 = m_Rotation2.x;

    // 右クリック押下で抬槍（X軸回転）
    GUNUSE = CDirectInput::GetInstance().GetMouseRButtonCheck();

    if (GUNUSE) {
        const float rotationSpeed = 0.1f;
        float targetRotationx = m_Rotation.x;
        float targetRotationx2 = m_Rotation2.x;

        // プレイヤーのX軸回転へなめらかに補間
        m_Rotation.x = playerRot->x + (targetRotationx - playerRot->x) * rotationSpeed;
        m_Rotation2.x = playerRot->x + (targetRotationx2 - playerRot->x) * rotationSpeed;
    }
    else {
        // 押していなければ元の角度に戻す
        m_Rotation.x = backupRotationX;
        m_Rotation2.x = backupRotationX2;
    }

    // -------- LoCK（追尾）エフェクト処理 --------
    if (enemy)
    {
        float distance = (*enemyPos - *playerPos).Length();

        if (distance < 200.0f)
        {
            // 敵が近いとき、LUCK 生成（100個）
            for (int i = 0; i < 100; i++) {
                Luck();  // LUCK初期化？（未提供定義）

                luck->m_LUCKAlive[i] = 1;

                // 敵の向きを使って前方向へ
                Vector3 lockForward = Vector3(sin(enemyRot->y), 0.0f, cos(enemyRot->y));
                lockForward.Normalize();

                Vector3 lockPosition = *enemyPos + lockForward * -10.0f;
                luck->m_LUCKPos[i] = lockPosition;
                luck->m_LUCKPos[i].y = lockPosition.y + 5;  // 少し浮かせる
            }
        }
        else {
            // 敵が遠い場合、LUCK を非アクティブ化
            for (int i = 0; i < 100; i++) {
                luck->m_LUCKAlive[i] = 0;
            }
        }
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


//===============================
// LUCKシステム：敵に近づくと自動で演出をトリガー
// プレイヤー・銃・敵の位置関係に基づいて向きを調整
//===============================



void GUN::Luck()
{
    // 各種必要なポインタを取得
    GUN* gun = GetGUN();
    Enemy* enemy = GetEnemy();
    CPlayer* player = GetPlayer();

    Vector3* playerPos = player->GetPosition();
    Vector3* playerRot = player->GetRotation();
    Vector3* enemyPos = enemy->GetPosition();
    Vector3* enemyRot = enemy->GetRotation();
    Vector3* gunShootPos1 = gun->GetShootPosition();
    Vector3* gunShootPos2 = gun->GetShootPosition2();
    Vector3* gunPos2 = gun->GetPosition2();
    Vector3* gunPos1 = gun->GetPosition();
    Vector3* gunRot = gun->GetRotation();
    Vector3* gunRot2 = gun->GetRotation2();

    // 無効な状態（敵死亡、ポインタ無効）なら処理中断
    if (!enemy->IsAlive() || player == nullptr || enemy == nullptr) return;

    const float minLockDistance = 15.0f; // ロックを行わない最小距離

    // 敵との距離を測定
    float distanceToEnemy = (*enemyPos - *playerPos).Length();
    float distanceToEnemy2 = (*enemyPos - *gunPos1).Length();
    float distanceToEnemy3 = (*enemyPos - *gunPos2).Length();

    // 銃が敵に近すぎるときはロック演出を行わない
    if (distanceToEnemy2 < minLockDistance && distanceToEnemy3 < minLockDistance) {
        return;
    }

    // 右クリック中のみロックオン処理を実行
    if (CDirectInput::GetInstance().GetMouseRButtonCheck())
    {
        // 銃1 → 敵 方向ベクトル
        Vector3 vGUNDir = *enemyPos - *gunPos1;
        vGUNDir.Normalize();

        // 銃2 → 敵 方向ベクトル
        Vector3 vGUNDir2 = *enemyPos - *gunPos2;
        vGUNDir2.Normalize();

        // プレイヤー → 敵 方向ベクトル
        Vector3 vPEDir = *enemyPos - *playerPos;
        vPEDir.Normalize();

        // 各方向ベクトルからY軸とX軸の角度を計算
        float angleY = atan2(vGUNDir.x, vGUNDir.z);
        float angleX = atan2(vGUNDir.y, sqrt(vGUNDir.x * vGUNDir.x + vGUNDir.z * vGUNDir.z));

        float angleY2 = atan2(vGUNDir2.x, vGUNDir2.z);
        float angleX2 = atan2(vGUNDir2.y, sqrt(vGUNDir2.x * vGUNDir2.x + vGUNDir2.z * vGUNDir2.z));

        float angleY3 = atan2(vPEDir.x, vPEDir.z);
        float angleX3 = atan2(vPEDir.y, sqrt(vPEDir.x * vPEDir.x + vPEDir.z * vPEDir.z));

        // 敵の方向を向くように回転角度を設定（180度反転）
        float targetRotationY  = angleY  + DirectX::XM_PI;
        float targetRotationY2 = angleY2 + DirectX::XM_PI;
        float targetPlayerRotationY = angleY3 + DirectX::XM_PI;

        const float rotationSpeed = 0.1f;  // 補間速度

        // プレイヤーのY回転を敵方向に補間（スムーズ回転）
        playerRot->y = playerRot->y + (targetPlayerRotationY - playerRot->y) * rotationSpeed;

        // 銃のY回転を直接設定（即時方向補正）
        m_Rotation.y  = angleY  + DirectX::XM_PI;
        m_Rotation2.y = angleY2 + DirectX::XM_PI;

        // （未使用）弾を撃ったかどうかのフラグ変数
        static int bulletShot = 0;
    }
}

