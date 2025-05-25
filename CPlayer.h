#pragma once

#include "CGameObject.h"
#include "CShader.h"
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "CDirectInput.h"



extern int playerscore;
extern int PLAYERHP;
extern bool hurt;

class CPlayer : public CGameObject {
private:
  
    CStaticMesh m_Mesh;
    CStaticMeshRenderer m_StaticMeshRenderer;
    CShader m_Shader;

   
    Vector3 m_vDir;

   
    float turnSpeed;
    float moveSpeed;
    float dashCooldown;
    float returnSpeed;
    
    float dashDistance;

    bool   dashActive = false;  
    float  dashDuration = 0.3f;   
    float  dashTimer = 0.0f;   
    float  dashSpeed = 10.0f;  
    Vector3 dashDirection = Vector3::Zero; 

    float lastTime;
    float currentTime;

   
    bool shootfire;
    bool missilefire;
    bool useshile;



    Vector3 m_vAABBMin;
    Vector3 m_vAABBMax;


public:
    CPlayer();
    virtual ~CPlayer();

  

   
    static CPlayer& GetInstance() {
        static CPlayer instance; 
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
    
    
    Vector3 GetDirection() const { return m_vDir; }

    
    void SetDirection(const Vector3& dir) { m_vDir = dir; }


    
    Matrix GetPlayerWorldMat() const { return GetWorldMatrix(); }

    Vector3 GetPlayerVDir() const { return m_vDir; }


    
    Vector3 GetPosition() const { return m_Position; }
	Vector3* GetPosition() { return &m_Position; }
	Vector3* GetRotation() { return &m_Rotation; }
   
    void SetPosition(const Vector3& pos) { m_Position = pos; }


    Vector3 GetPLAYERAABBMin() const { return m_vAABBMin; }
    Vector3 GetPLAYERAABBMax() const { return m_vAABBMax; }



};
CPlayer* GetPlayer();
