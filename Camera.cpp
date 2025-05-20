#include "renderer.h"
#include "Camera.h"
#include "Application.h"
#include "CPlayer.h"//2024-10-21 Monday
#include "CGameScene.h"
#include "CDirectInput.h"//2024-10-21 Monday



//2024-11-18
#define FPS


using namespace DirectX::SimpleMath;

Camera::Camera() {
	
}

Camera::~Camera() {
	
}

void Camera::Init()
{
	m_vEye = Vector3(0.0f, 10.0f, -50.0f);
	m_vLook = Vector3(0.0f, 10.0f, 0.0f);
	
}

void Camera::Dispose()
{

}


void Camera::Update()
{

	
	CPlayer* player = CGame::GetInstance().GetPlayer();

	if (!player) return;


	Vector3 playerPos = *player->GetPosition();

	Vector3 vPlayerDir = player->GetDirection();
	vPlayerDir.Normalize();

	
	float cameraDistance = 10.0f; 
	float cameraHeight = 10.0f;    

	
	m_vEye= playerPos  - vPlayerDir * cameraDistance;
	m_vEye.y = playerPos.y + cameraHeight;
	

	Vector3 horizontalDir = Vector3(vPlayerDir.x, 0.0f, vPlayerDir.z);
	horizontalDir.Normalize();

	
	m_vLook = playerPos + horizontalDir * 5.0f;
	m_vLook.y = playerPos.y + cameraHeight;	

	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	m_ViewMatrix = Matrix::CreateLookAt(m_vEye, m_vLook, up);



}


void Camera::Draw()
{
	// ビュー変換後列作成
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	m_ViewMatrix =
		DirectX::XMMatrixLookAtLH(
			m_vEye, //視点
			m_vLook,//注視点
			up);				
				

	Renderer::SetViewMatrix(&m_ViewMatrix);

	//プロジェクション行列の生成
	constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角

	float aspectRatio = static_cast<float>(Application::GetWidth()) / static_cast<float>(Application::GetHeight());	// アスペクト比	
	float nearPlane = 1.0f;       // ニアクリップ
	float farPlane = 1000.0f;      // ファークリップ

	//プロジェクション行列の生成
	Matrix projectionMatrix;
	projectionMatrix =
		DirectX::XMMatrixPerspectiveFovLH(
			fieldOfView,
			aspectRatio,
			nearPlane,
			farPlane);	// 左手系にした　20230511 by suzuki.tomoki


	Renderer::SetProjectionMatrix(&projectionMatrix);
}



