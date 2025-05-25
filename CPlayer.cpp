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
const float inertiaFactor = 0.01f; //慣性係数
const float friction = 0.95f;     // 摩擦
const float dashInertiaFactor = 0.3f; // 
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

            Vector3 playerForward = Vector3(sin(m_Rotation.y), 0.0f, cos(m_Rotation.y)); 
            playerForward.Normalize(); 

            Vector3 explosionPosition = m_Position + playerForward * -35.0f; 

            glass->glassisAlive[i] = 1;
            glass->glassPos[i] = explosionPosition;
            glass->glassFlag[i] = 0;

        }
       
    }
}

void CPlayer::Draw() {


 
  
    Matrix r = Matrix::CreateFromYawPitchRoll(m_Rotation.y, m_Rotation.x, m_Rotation.z);
    Matrix t = Matrix::CreateTranslation(m_Position);
    Matrix s = Matrix::CreateScale(m_Scale);
    Matrix worldmtx = s * r * t;

 
    m_WorldMatrix = worldmtx;

    
    m_vDir = Vector3(worldmtx._31, worldmtx._32, worldmtx._33);

    
    Renderer::SetWorldMatrix(&worldmtx); 
    
    m_Shader.SetGPU();

    
    m_StaticMeshRenderer.Draw();

}

void CPlayer::Dispose() {
  
}

void CPlayer::Move() {
    //
    float angle = m_Rotation.y;

    // 移動
    Vector3 forwardDir = Vector3(sin(angle), 0.0f, cos(angle)); // 前進
    Vector3 rightDir = Vector3(cos(angle), 0.0f, -sin(angle));  // 右移

    //
    Vector3 moveDir = Vector3::Zero;

    // 移動
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
        moveDir += rightDir * moveSpeed;  // 右
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D)) {
        moveDir -= rightDir * moveSpeed;  // 左
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
        moveDir -= forwardDir * moveSpeed;  // 前
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
        moveDir += forwardDir * moveSpeed;  // 後
    }

    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_Q)) {
        m_Rotation.y -= turnSpeed; // 
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_E)) {
        m_Rotation.y += turnSpeed; // 
    }
    
    m_Position += moveDir;

    velocity += moveDir * inertiaFactor; //方向に応じて速度を加える
    velocity *= friction; //摩擦力に応じて減速する
    m_Position += velocity; //
}


void CPlayer::Dashing() {


    static float dashDistanceCovered = 0.0f; 
    const float maxDashDistance = 50.0f;   

    static float verticalVelocity = 0.0f; 
    const float jumpHeight = 8.0f;        

    const float gravity = -9.8f;          

    
    static float baseHeight = m_Position.y; 
	static bool jump = false; 
	static bool canDash = true; 
	static bool canjump = true;

  //  if (!CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE)) {
  //     
		//canjump = true;
  //  }

    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE)&&canDash/*&&canjump*/) {
        // 上限に達した場合は、終了する。

     
		

        if (dashDistanceCovered >= maxDashDistance) {
            //m_Position.y = baseHeight; // キャラクターの高さを基準高さに設定する
            //jump = false;              
            dashDistanceCovered = 0.0f; 
            return;
            
        }

        if (!jump) {

            baseHeight = m_Position.y; 
            verticalVelocity = sqrt(2.0f * -gravity * jumpHeight); 
            jump = true; 
        }

        
        float angle = m_Rotation.y;

        
        Vector3 forwardDir = Vector3(sin(angle), 0.0f, cos(angle)); 
        Vector3 rightDir = Vector3(cos(angle), 0.0f, -sin(angle));  
        
        Vector3 dashDir = Vector3::Zero;

        
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
            dashDir += rightDir;  
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D)) {
            dashDir -= rightDir;  
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
            dashDir -= forwardDir;  
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
            dashDir += forwardDir;  
        }

        
        if (dashDir == Vector3::Zero) {
            dashDir = forwardDir;
        }

		dashDir.Normalize(); 
       
		
        Vector3 dashStep = dashDir * dashSpeed;
        float stepDistance = dashStep.Length();

        
        if (dashDistanceCovered + stepDistance > maxDashDistance) {
            float remainingDistance = maxDashDistance - dashDistanceCovered;
            dashStep = dashDir * remainingDistance;
            stepDistance = remainingDistance;
        }

        
        m_Position += dashStep;
        dashDistanceCovered += stepDistance;
       
       
        velocity += dashDir * dashInertiaFactor;
    }
    else {
        
        dashDistanceCovered = 0.0f;
    }

     
    if (jump) {
        m_Position.y += verticalVelocity * 0.05f; 
        verticalVelocity += gravity * 0.05f;     

    
        if (m_Position.y <= baseHeight) {
            m_Position.y = baseHeight; 
            verticalVelocity = 0.0f;  


        
                jump = false;
                canjump = false;
        }
    }
}


