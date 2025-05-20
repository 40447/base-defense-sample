
#pragma once

#include "CGameObject.h"
#include "CShader.h"
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "CDirectInput.h"

extern bool enemyattack;
extern bool enemydied;
extern int score ;
class Enemy : public CGameObject 

{

public:
    bool IsAlive() const { return isAlive; }
    void SetAlive(bool state) { isAlive = state; }

    
    static Enemy& GetInstance() {
        static Enemy instance; 
        return instance;
    }

    
    void Init() override;

 
    void Update() override;

    void Draw() override;

    void Dispose() override;

  
    void Move();
    void Fly();
    void Luck();
    void Dashing();
    void Respawn();

    ///////////////////////////

    enum class EnemyState
    {
        Idle,       // 
        Patrol,     // 
        Chase,      // 
        Attack,     // 
        Retreat,    // 
    };
    ///////////////////////
    //  // 
    //void SetState(EnemyState newState) { m_State = newState; }
    //EnemyState GetState() const { return m_State; }
    //// 
    //// 
    //// 
    //// 
    // 
    //
    Vector3 GetDirection() const { return m_vDir; }

    
    void SetDirection(const Vector3& dir) { m_vDir = dir; }


    
    Matrix GetEnemyWorldMat() const { return GetWorldMatrix(); }

    Vector3 GetEnemyVDir() const { return m_vDir; }


    

    Vector3* GetPosition() { return &m_Position; }
    Vector3* GetRotation() { return &m_Rotation; }



    Vector3 GetENEMYAABBMin() const { return m_vAABBMin; }
    Vector3 GetENEMYAABBMax() const { return m_vAABBMax; }





private:
    
    CStaticMesh m_Mesh;
    CStaticMeshRenderer m_StaticMeshRenderer;
    CShader m_Shader;

    
    Vector3 m_vDir;

    
    bool isAlive = true;

    Vector3 m_vAABBMin;
    Vector3 m_vAABBMax;

    float respawnTimer = 0.0f;
    const float respawnDelay = 1.0f; 
    //std::vector<Enemy> m_enemies;



    //////////////////////////////////7


    EnemyState m_State = EnemyState::Idle;  

 
    float engageDistance = 400.0f;   
    float attackDistance = 60.0f;    
	float SHOOTDistance = 200.0f;    

    float distanceBuffer = 50.0f;   

    float moveSpeed = 70.0f;     
    float retreatSpeed = 50.0f;    

    float attackInterval = 1.0f;    
    float attackTimer = 0.0f;      

    DirectX::SimpleMath::Vector3 m_PatrolTarget;
    bool m_HasPatrolTarget = false; 

  
    float m_PatrolRange = 400.0f;  

  
    float m_PatrolSpeed = 50.0f;





};
Enemy* GetEnemy();//2024-11-01