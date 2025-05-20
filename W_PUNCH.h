#pragma once

#include "CGameObject.h"
#include "CShader.h"
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
using namespace DirectX::SimpleMath;

static bool isPUNCH = false;
static bool isPUNCH2 = false;


static int currentPUNCH = 1;



class PUNCH : public CGameObject {
private:
    // 渲染相關
    CStaticMesh m_Mesh;
    CStaticMeshRenderer m_StaticMeshRenderer;


    Vector3 m_vAABBMin;
    Vector3 m_vAABBMax;

	// 第二把槍

	Vector3 m_vAABBMin2;
	Vector3 m_vAABBMax2;



    Vector3 m_Rotation2 = Vector3(DirectX::XM_PI, 0.0f, 0.0f);
    Vector3 m_Position2 = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_Scale2 = Vector3(4.0f, 4.0f, 6.0f);

	Vector3 PUNCHRotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 PUNCHPosition = Vector3(0.0f, 0.0f, 0.0f);
	

 
	Vector3 PUNCHRotation2 = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 PUNCHPosition2 = Vector3(0.0f, 0.0f, 0.0f);
	



    CStaticMesh m_Mesh2;
    CStaticMeshRenderer m_StaticMeshRenderer2;

    CShader m_Shader;
public:
    // 單例模式
    static PUNCH& GetInstance() {
        static PUNCH instance; // 保證只有一個實例
        return instance;
    }

    // 基本方法
    void Init() override;
    void Update() override;
    void Draw() override;
    void Dispose() override;
    void UpdatePUNCH();
    void UpdatePUNCH2();
    void PUNCHLOCK();

    Vector3* GetPosition2() { return &m_Position2; }
    Vector3* GetRotation2() { return &m_Rotation2; }

	Vector3* GetPUNCHPosition() { return &PUNCHPosition; }
	Vector3* GetPUNCHRotation() { return &PUNCHRotation; }

	Vector3* GetPUNCHPosition2() { return &PUNCHPosition2; }
	Vector3* GetPUNCHRotation2() { return &PUNCHRotation2; }


    Vector3 GetPUNCHAABBMin() const { return m_vAABBMin; }
    Vector3 GetPUNCHAABBMax() const { return m_vAABBMax; }

	Vector3 GetPUNCHAABBMin2() const { return m_vAABBMin2; }
	Vector3 GetPUNCHAABBMax2() const { return m_vAABBMax2; }

    void SetPosition2(const Vector3& position) { m_Position2 = position; }

};
PUNCH* GetPUNCH(); // 全局訪問方法