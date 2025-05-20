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
    // 描繪相關物件
    CStaticMesh m_Mesh;
    CStaticMeshRenderer m_StaticMeshRenderer;
    CShader m_Shader;

    // 方向向量
    Vector3 m_vDir;

    // 衝刺相關變數
    float turnSpeed;
    float moveSpeed;
    float dashCooldown;
    float returnSpeed;
    
    float dashDistance;

    bool   dashActive = false;  // 是否正在dash
    float  dashDuration = 0.3f;   // dash 持續時間(秒)
    float  dashTimer = 0.0f;   // 用來累計已經dash了多少秒
    float  dashSpeed = 10.0f;  // dash 速度(可自行調整)
    Vector3 dashDirection = Vector3::Zero; // dash時的朝向/方向

    float lastTime;
    float currentTime;

    // 射擊相關變數
    bool shootfire;
    bool missilefire;
    bool useshile;



    Vector3 m_vAABBMin;
    Vector3 m_vAABBMax;


public:
    CPlayer();
    virtual ~CPlayer();

  

    // 提供一個靜態方法來獲取單例實例
    static CPlayer& GetInstance() {
        static CPlayer instance; // 保證只有一個實例
        return instance;
    }

    // 繼承自基類的初始化
    void Init() override;

    // 繼承自基類的更新
    void Update() override;

    // 繼承自基類的繪製
    void Draw() override;

    // 繼承自基類的釋放資源
    void Dispose() override;

    // 行為方法
    void Move();
    void Fly();
    void Luck();
    void Dashing();
    
    // 取得方向向量
    Vector3 GetDirection() const { return m_vDir; }

    // 設定方向向量
    void SetDirection(const Vector3& dir) { m_vDir = dir; }


    // 新增方法以供 Camera 使用
    Matrix GetPlayerWorldMat() const { return GetWorldMatrix(); }

    Vector3 GetPlayerVDir() const { return m_vDir; }


    // 取得玩家位置
    Vector3 GetPosition() const { return m_Position; }
	Vector3* GetPosition() { return &m_Position; }
	Vector3* GetRotation() { return &m_Rotation; }
    // 設定玩家位置
    void SetPosition(const Vector3& pos) { m_Position = pos; }


    Vector3 GetPLAYERAABBMin() const { return m_vAABBMin; }
    Vector3 GetPLAYERAABBMax() const { return m_vAABBMax; }



};
CPlayer* GetPlayer();