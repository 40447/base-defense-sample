#include "CPlayer.h"
#include "Renderer.h"
#include <cmath>
#include"W_GUN.h"
#include <iostream>
#include "Polygon2D.h"
#include"main.h"
#include"Camera.h"
#include"glass.h"



extern bool GUNUSE;

Vector3 velocity = Vector3::Zero; //
const float inertiaFactor = 0.01f; // 慣性因子
const float friction = 0.95f;     // 摩擦力
const float dashInertiaFactor = 0.3f; // 衝刺慣性因子
static Polygon2D glass;
int px, py;
CPlayer::CPlayer() : 
    
    
    m_vDir(0.0f, 0.0f, 1.0f),
    turnSpeed(0.02f),
    moveSpeed(1.0f),
    dashCooldown(0.3f),
    returnSpeed(2.0f),
    dashSpeed(2.0f),
    dashDistance(10.0f),
    lastTime(0.0f),
    currentTime(0.0f),
    shootfire(false),
    missilefire(false),
    useshile(false)

{
    // 初始化成員
}

CPlayer::~CPlayer() {
    Dispose();
}

void CPlayer::Init() {
    // SRT
    m_Position = Vector3(0.0f, 12.0f, 0.0f);
    m_Rotation = Vector3(DirectX::XM_PI / 2, 0.0f, 0.0f);
    m_Scale = Vector3(2.0f, 4.0f, 5.0f);

    UpdateWorldMatrix();

    m_Mesh.Load(
        "assets/model/0005.fbx",    // 
        "assets/model/"            //
    );

    // 渲染器初始化
    m_StaticMeshRenderer.Init(m_Mesh);

    // シェーダーの初期化
    m_Shader.Create(
        "shader/vertexLightingVS.hlsl", // 頂點シェーダー
        "shader/vertexLightingPS.hlsl"  // ピクセルシェーダー
    );

    // 調整旋轉角度
    m_Rotation.y += DirectX::XM_PI;

    //std::cout << "玩家初始化完成。" << std::endl;

    float halfW = 20.0f * 0.5f;
    float halfH = 20.0f * 0.5f;
    float halfD = 20.0f * 0.5f;

    m_vAABBMin = Vector3(m_Position.x - halfW,
        m_Position.y - halfH,
        m_Position.z - halfD);
    m_vAABBMax = Vector3(m_Position.x + halfW,
        m_Position.y + halfH,
        m_Position.z + halfD);
    int screenw = SCREEN_WIDTH;
    int screenh = SCREEN_HEIGHT;
   

   glass.Init((char*)"assets/GLASS.png", px, py, screenw, screenh);
}

void CPlayer::Update() {


    Camera* cmaera = GetCamera();
    Vector3 HURTP = cmaera->GetLook();
    Glass* glass = Getglass();
    Move();
	
    Dashing();

    ///position<450
    float radius = 450.0f;
    float distanceSquared = m_Position.x * m_Position.x + m_Position.z * m_Position.z;

    if (distanceSquared > radius * radius) {
        float distance = sqrt(distanceSquared);//平方根
        m_Position.x = (m_Position.x / distance) * radius;
        m_Position.z = (m_Position.z / distance) * radius;
    }
 

    float halfW = 40.0f * 0.5f;
    float halfH =100.0f * 0.5f;
    float halfD = 40.0f * 0.5f;
    m_vAABBMin = Vector3

    (   m_Position.x - halfW,
        m_Position.y - halfH,
        m_Position.z - halfD);
    m_vAABBMax = Vector3
    
    (   m_Position.x + halfW,
        m_Position.y + halfH,
        m_Position.z + halfD);

  
    UpdateWorldMatrix();

  
    
    for (int i = 0; i < 100; i++) {

        if (PLAYERHP<=50) {

            Vector3 playerForward = Vector3(sin(m_Rotation.y), 0.0f, cos(m_Rotation.y)); // 根據玩家的旋轉角度計算前方向
            playerForward.Normalize(); // 方向

            Vector3 explosionPosition = m_Position + playerForward * -35.0f; // 玩家前方 10 單位的位置

            glass->glassisAlive[i] = 1;
            glass->glassPos[i] = explosionPosition;
            glass->glassFlag[i] = 0;

        }
       
    }
}

void CPlayer::Draw() {


 
    // 創建世界矩陣
    Matrix r = Matrix::CreateFromYawPitchRoll(m_Rotation.y, m_Rotation.x, m_Rotation.z);
    Matrix t = Matrix::CreateTranslation(m_Position);
    Matrix s = Matrix::CreateScale(m_Scale);
    Matrix worldmtx = s * r * t;

    // 更新世界矩陣
    m_WorldMatrix = worldmtx;

    // 更新方向向量
    m_vDir = Vector3(worldmtx._31, worldmtx._32, worldmtx._33);

    // 設定世界矩陣到渲染器
    Renderer::SetWorldMatrix(&worldmtx); // 假設 Renderer 是一個全域物件或單例

    // 設定シェーダー到 GPU
    m_Shader.SetGPU();

    // 繪製模型
    m_StaticMeshRenderer.Draw();

    //wstd::cout << "玩家繪製完成。" << std::endl;
}

void CPlayer::Dispose() {
  
}

void CPlayer::Move() {
    //
    float angle = m_Rotation.y;

    // 計算移動方向
    Vector3 forwardDir = Vector3(sin(angle), 0.0f, cos(angle)); // 前進方向
    Vector3 rightDir = Vector3(cos(angle), 0.0f, -sin(angle));  // 右移方向

    // 初始化方向向量
    Vector3 moveDir = Vector3::Zero;

    // 移動控制
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
        moveDir += rightDir * moveSpeed;  // 向右移動
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D)) {
        moveDir -= rightDir * moveSpeed;  // 向左移動
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
        moveDir -= forwardDir * moveSpeed;  // 向前移動
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
        moveDir += forwardDir * moveSpeed;  // 向後移動
    }

    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_Q)) {
        m_Rotation.y -= turnSpeed; // 左轉
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_E)) {
        m_Rotation.y += turnSpeed; // 右轉
    }
    // 更新位置
    m_Position += moveDir;

    // 添加慣性效果（逐步減速）
    velocity += moveDir * inertiaFactor; // 根據移動方向增加速度
    velocity *= friction; // 根據摩擦力減速
    m_Position += velocity; // 更新位置
}


void CPlayer::Dashing() {


    static float dashDistanceCovered = 0.0f; // 當前已衝刺的距離
    const float maxDashDistance = 50.0f;    // 衝刺距離上限

    static float verticalVelocity = 0.0f; // 垂直速度
    const float jumpHeight = 8.0f;        // 跳躍的最大高度

    const float gravity = -9.8f;          // 重力加速度

    
    static float baseHeight = m_Position.y; // 初始高度（基準高度）
	static bool jump = false; // 跳躍標誌
	static bool canDash = true; // 是否可以衝刺
	static bool canjump = true;

  //  if (!CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE)) {
  //     
		//canjump = true;
  //  }

    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE)&&canDash/*&&canjump*/) {
        // 如果已達到衝刺上限，退出

     
		

        if (dashDistanceCovered >= maxDashDistance) {
            //m_Position.y = baseHeight; // 將角色高度設置為基準高度
            //jump = false;              // 結束跳躍狀態
            dashDistanceCovered = 0.0f; // 重置衝刺距離
            return;
            
        }

        if (!jump) {

            baseHeight = m_Position.y; // 記錄角色的基準高度
            verticalVelocity = sqrt(2.0f * -gravity * jumpHeight); // 根據物理公式計算初速度
            jump = true; // 標記為跳躍狀態
        }

        // 獲取玩家的當前旋轉角度
        float angle = m_Rotation.y;

        // 計算方向向量
        Vector3 forwardDir = Vector3(sin(angle), 0.0f, cos(angle)); // 前進方向
        Vector3 rightDir = Vector3(cos(angle), 0.0f, -sin(angle));  // 右移方向

        // 初始化衝刺方向
        Vector3 dashDir = Vector3::Zero;

        // 檢測輸入方向
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
            dashDir += rightDir;  // 向右衝刺
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D)) {
            dashDir -= rightDir;  // 向左衝刺
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
            dashDir -= forwardDir;  // 向前衝刺
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
            dashDir += forwardDir;  // 向後衝刺
        }

        // 如果沒有明確方向輸入，使用當前前進方向
        if (dashDir == Vector3::Zero) {
            dashDir = forwardDir;
        }

		dashDir.Normalize(); // 正規化
        // 計算當次衝刺的移動量
		
        Vector3 dashStep = dashDir * dashSpeed;
        float stepDistance = dashStep.Length();

        // 如果超過剩餘距離，限制移動量
        if (dashDistanceCovered + stepDistance > maxDashDistance) {
            float remainingDistance = maxDashDistance - dashDistanceCovered;
            dashStep = dashDir * remainingDistance;
            stepDistance = remainingDistance;
        }

        // 更新水平位置
        m_Position += dashStep;
        dashDistanceCovered += stepDistance;
       
        // 添加慣性效果
        velocity += dashDir * dashInertiaFactor;
    }
    else {
        // 如果不在衝刺中，重置距離計數器
        dashDistanceCovered = 0.0f;
    }

     //垂直運動更新
    if (jump) {
        m_Position.y += verticalVelocity * 0.05f; // 垂直位移 (假設幀時間為 0.016 秒)
        verticalVelocity += gravity * 0.05f;     // 更新垂直速度

        // 檢查是否回到基準高度
        if (m_Position.y <= baseHeight) {
            m_Position.y = baseHeight; // 恢復到基準高度
            verticalVelocity = 0.0f;   // 重置垂直速度


        
                jump = false;
                canjump = false;
        }
    }
}


