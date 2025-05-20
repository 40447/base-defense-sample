#include "CollisionManager.h"
#include "Shoot.h"
#include "Enemy.h"
#include <iostream>
//#include"Shoot2.h"
#include "Explosion.h"
#include"W_PUNCH.h"
#include"EnemyShoot.h"
#include "CPlayer.h"
#include"Camera.h"
#include"glass.h"
using namespace DirectX::SimpleMath;


int score = 0;
int enemyscore = 0;
int enemyhurt = 0;
int playerscore = 0;
int PLAYERHP = 100;

bool m_bPunchCollideLastFrame = false;   // 上幀是否拳擊中
bool m_bPunchCollideLastFrame2 = false;  // 第二個拳頭

bool hurt = false;
bool enemydied = false;


// 先定義一個「AABB 與 AABB」重疊檢查的輔助函式
static bool OverlapAABB
(
    const Vector3& minA, const Vector3& maxA,
    const Vector3& minB, const Vector3& maxB)
{
    // 在 X 軸若無重疊 => 無碰撞
    if (maxA.x < minB.x || minA.x > maxB.x) return false;
    // 在 Y 軸若無重疊 => 無碰撞
    if (maxA.y < minB.y || minA.y > maxB.y) return false;
    // 在 Z 軸若無重疊 => 無碰撞
    if (maxA.z < minB.z || minA.z > maxB.z) return false;

    // 三軸都有重疊 => AABB 重疊
    return true;
}

void CollisionManager::CheckCollision()
{
    // 1) 取得 Shoot & Enemy 單例
	Shoot* shoot = GetShoot();
	Enemy* enemy = GetEnemy();
	CPlayer* player = GetPlayer();
    Explosion* explosion = GetExplosion();
	PUNCH* punch = GetPUNCH();
	EnemyShoot* enemyshoot = GetEnemyShoot();
    Glass* glass = Getglass();
   

	
    Vector3 expos = *explosion->GetExPositions();
    Vector3 enemypos = *enemy->GetPosition();
	Vector3 enemyRot = *enemy->GetRotation();
	Vector3 playerpos = *player->GetPosition();
	Vector3 playerRotation = *player->GetRotation();

    // 2) 從 Shoot 取得子彈資料
    //    假設您在 Shoot.h / Shoot.cpp 有對外提供 Getter 
    //    或直接存取 public 成員 m_bulletAABBMin[], m_bulletAABBMax[]。
    int bulletCount = shoot->GetMaxBullets();
	int bulletCount2 = shoot->GetMaxBullets2();

    const Vector3* bulletAABBMin = shoot->m_bulletAABBMin;
    const Vector3* bulletAABBMax = shoot->m_bulletAABBMax;

	const Vector3* bulletAABBMin2 = shoot->m_bulletAABBMin2;
	const Vector3* bulletAABBMax2 = shoot->m_bulletAABBMax2;

    const int* bulletLife = shoot->GetBulletAliveFlags();
	const int* bulletLife2 = shoot->GetBulletAliveFlags2();
	const int* enemybulletLife2 = enemyshoot->GetEnemyShootAliveFlags();

    // 3) 取得敵人的 AABB
    Vector3 enemyMin = enemy->GetENEMYAABBMin();
    Vector3 enemyMax = enemy->GetENEMYAABBMax();

	Vector3 playerMin = player->GetPLAYERAABBMin();
	Vector3 playerMax = player->GetPLAYERAABBMax();
    ///////////////////////////////
	Vector3 PUNCHMin = punch->GetPUNCHAABBMin();
	Vector3 PUNCHMax = punch->GetPUNCHAABBMax();
    /////////////////////////////////
	Vector3 PUNCHMin2 = punch->GetPUNCHAABBMin2();
	Vector3 PUNCHMax2 = punch->GetPUNCHAABBMax2();
////////////////////////////////////////////////
	Vector3 *enemyshootMin = enemyshoot->m_EnemyShootAABBMin;
	Vector3 *enemyshootMax = enemyshoot->m_EnemyShootAABBMin;

    Vector3 enemyForward = Vector3(sin(enemyRot.y), 0.0f, cos(enemyRot.y)); // 根據玩家的旋轉角度計算前方向
    enemyForward.Normalize(); // 確保方向向量為單位向量


    // 4) 逐一檢查每顆子彈與敵人 AABB
    for (int i = 0; i < bulletCount; i++)
    {
        if (enemybulletLife2[i]==1) {
            if (OverlapAABB(enemyshootMin[i], enemyshootMax[i], playerMin, playerMax)) {
            
                enemyshoot->m_EnemyShootAlive[i] = 0;


                Vector3 playerForward = Vector3(sin(playerRotation.y), 0.0f, cos(playerRotation.y)); // 根據玩家的旋轉角度計算前方向
                playerForward.Normalize(); // 確保方向向量為單位向量

                Vector3 explosionPosition = playerpos + playerForward * -30.0f; // 玩家前方 10 單位的位置

				hurt = true;
            

                explosion->g_Ex_isAlive[i] = 1;
                explosion->g_ExPos[i] = explosionPosition; // 將爆炸位置設為計算後的位置
                explosion->g_ExFlag[i] = 0;
            
            
            
            
            }	
        }
        if (bulletLife[i] == 1)  // 子彈活著
        {
            // 檢查 AABB vs. AABB
            if (OverlapAABB(bulletAABBMin[i], bulletAABBMax[i],
                enemyMin, enemyMax))
            {
                // 碰撞成立 -> 做後續處理
               enemyhurt=true;
                // a) 子彈消失
                shoot->m_shootAlive[i] = 0;
               

                

                Vector3 exPosition = enemypos + enemyForward * -15.0f; // 玩家前方 10 單位的位置

                explosion->g_Ex_isAlive[i] = 1;
                explosion->g_ExPos[i] = exPosition;
               // explosion->g_ExPos[i].y = exPosition.y + 10;
                explosion->g_ExFlag[i] = 0;
               // std::cout << "explosion" << std::endl;
               
               //  如果只需一發子彈就能擊中 -> 結束
                break;
            }
        }
        if (bulletLife2[i] == 1) {
            if (OverlapAABB(bulletAABBMin2[i], bulletAABBMax2[i],
                enemyMin, enemyMax))
            {
                // 碰撞成立 -> 做後續處理
                enemyhurt = true;
                // a) 子彈消失
                shoot->m_shootAlive2[i] = 0;
               
              
                Vector3 exPosition = enemypos + enemyForward * -15.0f; // 玩家前方 10 單位的位置

                explosion->g_Ex_isAlive[i] = 1;
                explosion->g_ExPos[i] = exPosition;
               // explosion->g_ExPos[i].y = exPosition.y+10;
                explosion->g_ExFlag[i] = 0;
                break;
            }

        }
      
    }
   
    bool isPunchColliding = OverlapAABB(PUNCHMin, PUNCHMax, enemyMin, enemyMax);
    bool isPunch2Colliding = OverlapAABB(PUNCHMin2, PUNCHMax2, enemyMin, enemyMax);
    // 拳頭1：
    if (isPunchColliding)
    {
        // 如果上一幀沒撞，這一幀才剛撞 => 加分
        if (!m_bPunchCollideLastFrame)
        {
            enemy->SetAlive(false);
            score = 0;
            Vector3 exPosition = enemypos + enemyForward * -15.0f;
            explosion->g_Ex_isAlive[1] = 1; // 根據需求選擇索引
            explosion->g_ExPos[1] = exPosition;
          //  explosion->g_ExPos[1].y = exPosition.y + 10;
            explosion->g_ExFlag[1] = 0;
        }
        // 不管剛撞或持續撞，都要設「這幀是碰撞中」
        m_bPunchCollideLastFrame = true;
    }
    else
    {
        // 沒撞就把旗標改回 false
        m_bPunchCollideLastFrame = false;
    }

    // 拳頭2：
    if (isPunch2Colliding)
    {
        if (!m_bPunchCollideLastFrame2)
        {
            enemy->SetAlive(false);
            Vector3 exPosition = enemypos + enemyForward * -15.0f;
            score = 0;
            explosion->g_Ex_isAlive[1] = 1; // 根據需求選擇索引
            explosion->g_ExPos[1] = exPosition;
         //  explosion->g_ExPos[1].y = exPosition.y + 10;
            explosion->g_ExFlag[1] = 0;
        }
        m_bPunchCollideLastFrame2 = true;
    }
    else
    {
        m_bPunchCollideLastFrame2 = false;
    }

  //  std::cout << "score = " << score << std::endl;
    if (enemyhurt) {
        score += 1;


 
        enemyhurt = false;
    }
   
    if (score >= 6) {
        enemy->SetAlive(false);
        Vector3 enemyForward = Vector3(sin(enemyRot.y), 0.0f, cos(enemyRot.y)); // 根據玩家的旋轉角度計算前方向
        enemyForward.Normalize(); // 確保方向向量為單位向量

        Vector3 exPosition = enemypos + enemyForward * -15.0f; // 玩家前方 10 單位的位置

        for (int i = 0; i < 100; i++) {
            explosion->g_Ex_isAlive[i] = 1;
            explosion->g_ExPos[i] = exPosition;
           // explosion->g_ExPos[i].y = exPosition.y + 10;
            explosion->g_ExFlag[i] = 0;
        }
        enemyscore += 1;
        std::cout << "enemyscore" << enemyscore << std::endl;
        score = 0;
        
    }
    if (hurt) {
        PLAYERHP -= 10;
        std::cout << "HP" << PLAYERHP << std::endl;

        //Vector3 playerForward = Vector3(sin(m_Rotation.y), 0.0f, cos(m_Rotation.y)); // 根據玩家的旋轉角度計算前方向
        //playerForward.Normalize(); // 確保方向向量為單位向量

        //Vector3 explosionPosition = m_Position + playerForward * -30.0f; // 玩家前方 10 單位的位置
        //glass->glassisAlive[i] = 1;
        //glass->glassPos[i] = explosionPosition;
        //glass->glassFlag[i] = 0;
        hurt = false;
    }
    if (enemyscore>10) {

        enemydied = true;

    }
}
