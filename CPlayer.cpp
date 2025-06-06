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

Vector3 velocity = Vector3::Zero; // 現在の移動速度（慣性シミュレーション用）
const float inertiaFactor = 0.01f; //慣性係数
const float friction = 0.95f;     // 摩擦
const float dashInertiaFactor = 0.3f; // 
static Polygon2D glass;
int px, py;
CPlayer::CPlayer() : 
    
    
    m_vDir(0.0f, 0.0f, 1.0f),
    turnSpeed(0.02f),
    moveSpeed(1.0f),
    dashCooldown(0.3f), // ダッシュの再使用までの待機時間
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

    // カメラの参照を取得
    Camera* cmaera = GetCamera();
    Vector3 HURTP = cmaera->GetLook();

    // ガラスエフェクト管理用ポインタ取得
    Glass* glass = Getglass();

    // プレイヤーの移動処理
    Move();

    // ダッシュ処理
    Dashing();

    /// プレイヤーの行動可能範囲制限（半径450以内）
    float radius = 450.0f;
    float distanceSquared = m_Position.x * m_Position.x + m_Position.z * m_Position.z;

    // 距離が範囲外の場合、円周上に位置を補正
    if (distanceSquared > radius * radius) {
        float distance = sqrt(distanceSquared); // √(x^2 + z^2)
        m_Position.x = (m_Position.x / distance) * radius;
        m_Position.z = (m_Position.z / distance) * radius;
    }

    // AABB（当たり判定ボックス）設定
    float halfW = 40.0f * 0.5f;
    float halfH = 100.0f * 0.5f;
    float halfD = 40.0f * 0.5f;

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

    // ワールド行列の更新
    UpdateWorldMatrix();

    // プレイヤーのHPが50以下なら、ガラス破片エフェクトを100個生成
    for (int i = 0; i < 100; i++) {

        if (PLAYERHP <= 50) {
            // プレイヤーの前方向ベクトルを計算（Y軸回転から）
            Vector3 playerForward = Vector3(sin(m_Rotation.y), 0.0f, cos(m_Rotation.y));
            playerForward.Normalize();

            // プレイヤーの背後35の位置に破片を発生
            Vector3 explosionPosition = m_Position + playerForward * -35.0f;

            // 破片エフェクトの初期化
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


//------------------------------------------------------------
// プレイヤーの移動処理（慣性付き）
//------------------------------------------------------------


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

    velocity += moveDir * inertiaFactor; // 移動方向に応じて速度を加える（慣性効果）
    velocity *= friction;                // 摩擦係数により速度を減衰させる
    m_Position += velocity; //
}

//======================
// ダッシュ処理の流れ
//======================
// 1. キー入力確認（SPACE）
// 2. ダッシュ可能チェックと距離制限
// 3. 方向決定（WASD or 前方）
// 4. 一定距離までダッシュ移動
// 5. 垂直ジャンプ処理（ジャンプ中のみY軸加算）
// 6. 地面に戻ったらジャンプ解除
void CPlayer::Dashing() {

    // 累積ダッシュ距離（静的変数：呼び出し間で保持される）
    static float dashDistanceCovered = 0.0f; 
    const float maxDashDistance = 50.0f;     // ダッシュ最大距離

    static float verticalVelocity = 0.0f;    // 垂直方向の速度
    const float jumpHeight = 8.0f;           // ジャンプ高さ

    const float gravity = -9.8f;             // 重力加速度（負方向）

    static float baseHeight = m_Position.y;  // 地面の高さ基準（Y座標）
    static bool jump = false;                // ジャンプ中フラグ
    static bool canDash = true;              // ダッシュ可能フラグ
    static bool canjump = true;              // ジャンプ可能フラグ（未使用）

    

    // スペースキー押下時、かつダッシュ可能
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE) && canDash /*&& canjump*/) {

        // 既に最大距離まで移動していたら処理終了
        if (dashDistanceCovered >= maxDashDistance) {
            dashDistanceCovered = 0.0f;
            return;
        }

        // ジャンプ開始初期化（1回だけ実行）
        if (!jump) {
            baseHeight = m_Position.y;
            verticalVelocity = sqrt(2.0f * -gravity * jumpHeight); // 重力から初速計算
            jump = true;
        }

        // プレイヤーの向き（Y軸回転）から前後左右ベクトルを計算
        float angle = m_Rotation.y;

        Vector3 forwardDir = Vector3(sin(angle), 0.0f, cos(angle));      // 前方向
        Vector3 rightDir = Vector3(cos(angle), 0.0f, -sin(angle));       // 右方向

        Vector3 dashDir = Vector3::Zero;  // 実際のダッシュ方向

        // 入力キーによって dashDir に方向ベクトルを加算
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
            dashDir += rightDir;   // 左移動
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D)) {
            dashDir -= rightDir;   // 右移動
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
            dashDir -= forwardDir; // 前進
        }
        if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
            dashDir += forwardDir; // 後退
        }

        // 入力が無ければ前進をデフォルトとする
        if (dashDir == Vector3::Zero) {
            dashDir = forwardDir;
        }

        // 正規化して方向ベクトルに変換
        dashDir.Normalize();

        // 一歩分の移動量を計算（速度を掛ける）
        Vector3 dashStep = dashDir * dashSpeed;
        float stepDistance = dashStep.Length();

        // 残りのダッシュ距離を超えないよう調整
        if (dashDistanceCovered + stepDistance > maxDashDistance) {
            float remainingDistance = maxDashDistance - dashDistanceCovered;
            dashStep = dashDir * remainingDistance;
            stepDistance = remainingDistance;
        }

        // 実際に移動する
        m_Position += dashStep;
        dashDistanceCovered += stepDistance;

        // ダッシュによる慣性を加える
        velocity += dashDir * dashInertiaFactor;
    }
    else {
        // スペースキーを離したら距離カウントをリセット
        dashDistanceCovered = 0.0f;
    }

    // ジャンプ処理（Y方向移動と重力）
    if (jump) {
        m_Position.y += verticalVelocity * 0.05f;  // 垂直移動
        verticalVelocity += gravity * 0.05f;       // 重力適用

        // 地面まで戻ったら着地
        if (m_Position.y <= baseHeight) {
            m_Position.y = baseHeight;
            verticalVelocity = 0.0f;

            jump = false;
            canjump = false;
        }
    }
}


