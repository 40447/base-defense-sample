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

bool m_bPunchCollideLastFrame = false;   
bool m_bPunchCollideLastFrame2 = false;  

bool hurt = false;
bool enemydied = false;



static bool OverlapAABB
(
    const Vector3& minA, const Vector3& maxA,
    const Vector3& minB, const Vector3& maxB)
{
    
    if (maxA.x < minB.x || minA.x > maxB.x) return false;
   
    if (maxA.y < minB.y || minA.y > maxB.y) return false;
   
    if (maxA.z < minB.z || minA.z > maxB.z) return false;


    return true;
}
//------------------------------------------------------------
// 弾・近接・敵弾の当たり判定（プレイヤー／敵）
//------------------------------------------------------------
void CollisionManager::CheckCollision()
{
    
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

   
    int bulletCount = shoot->GetMaxBullets();
	int bulletCount2 = shoot->GetMaxBullets2();

    const Vector3* bulletAABBMin = shoot->m_bulletAABBMin;
    const Vector3* bulletAABBMax = shoot->m_bulletAABBMax;

	const Vector3* bulletAABBMin2 = shoot->m_bulletAABBMin2;
	const Vector3* bulletAABBMax2 = shoot->m_bulletAABBMax2;

    const int* bulletLife = shoot->GetBulletAliveFlags();
	const int* bulletLife2 = shoot->GetBulletAliveFlags2();
	const int* enemybulletLife2 = enemyshoot->GetEnemyShootAliveFlags();

 
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

    Vector3 enemyForward = Vector3(sin(enemyRot.y), 0.0f, cos(enemyRot.y)); 
    enemyForward.Normalize(); 


  
    for (int i = 0; i < bulletCount; i++)
    {
        if (enemybulletLife2[i]==1) {
            if (OverlapAABB(enemyshootMin[i], enemyshootMax[i], playerMin, playerMax)) {
              // 通常弾が敵に命中
                enemyshoot->m_EnemyShootAlive[i] = 0;


                Vector3 playerForward = Vector3(sin(playerRotation.y), 0.0f, cos(playerRotation.y)); 
                playerForward.Normalize(); 
              
                Vector3 explosionPosition = playerpos + playerForward * -30.0f; 

				hurt = true;
            

                explosion->g_Ex_isAlive[i] = 1;
                explosion->g_ExPos[i] = explosionPosition; 
                explosion->g_ExFlag[i] = 0;
            
            
            
            
            }	
        }
        if (bulletLife[i] == 1)  // 子彈活著
        {
            
            if (OverlapAABB(bulletAABBMin[i], bulletAABBMax[i],
                enemyMin, enemyMax))
            {
                
               enemyhurt=true;
               
                shoot->m_shootAlive[i] = 0;
               

                
             // 敵の向きに基づいて爆発位置を少し後ろに調整
                Vector3 exPosition = enemypos + enemyForward * -15.0f; 
                explosion->g_Ex_isAlive[i] = 1;
                explosion->g_ExPos[i] = exPosition;
               // explosion->g_ExPos[i].y = exPosition.y + 10;
                explosion->g_ExFlag[i] = 0;
               // std::cout << "explosion" << std::endl;
               
              
                break;// 命中後、ループ打ち切り
            }
        }
        if (bulletLife2[i] == 1) {
            if (OverlapAABB(bulletAABBMin2[i], bulletAABBMax2[i],
                enemyMin, enemyMax))
            {
              
                enemyhurt = true;
             
                shoot->m_shootAlive2[i] = 0;
               
              
                Vector3 exPosition = enemypos + enemyForward * -15.0f;

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
   
    if (isPunchColliding)
    {
        
        if (!m_bPunchCollideLastFrame)
        {

		 // 前のフレームで当たっていなければ命中処理
            enemy->SetAlive(false);
            score = 0;
            Vector3 exPosition = enemypos + enemyForward * -15.0f;
            explosion->g_Ex_isAlive[1] = 1; 
            explosion->g_ExPos[1] = exPosition;
          //  explosion->g_ExPos[1].y = exPosition.y + 10;
            explosion->g_ExFlag[1] = 0;
        }
       
        m_bPunchCollideLastFrame = true;
    }
    else
    {
      
        m_bPunchCollideLastFrame = false;
    }

  
    if (isPunch2Colliding)
    {
        if (!m_bPunchCollideLastFrame2)
        {
            enemy->SetAlive(false);
            Vector3 exPosition = enemypos + enemyForward * -15.0f;
            score = 0;
            explosion->g_Ex_isAlive[1] = 1; 
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
	    
        enemy->SetAlive(false); // 敵に一定ダメージを与えたら撃破
        Vector3 enemyForward = Vector3(sin(enemyRot.y), 0.0f, cos(enemyRot.y)); 
        enemyForward.Normalize(); 

        Vector3 exPosition = enemypos + enemyForward * -15.0f; 

        for (int i = 0; i < 100; i++) { // 大規模爆発エフェクト
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

        //Vector3 playerForward = Vector3(sin(m_Rotation.y), 0.0f, cos(m_Rotation.y));
        //playerForward.Normalize(); 

        //Vector3 explosionPosition = m_Position + playerForward * -30.0f; 
        //glass->glassisAlive[i] = 1;
        //glass->glassPos[i] = explosionPosition;
        //glass->glassFlag[i] = 0;
        hurt = false;
    }
    if (enemyscore>10) {

        enemydied = true;

    }
}
