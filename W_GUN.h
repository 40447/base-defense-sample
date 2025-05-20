#pragma once

#include "CGameObject.h"
#include "CShader.h"
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
using namespace DirectX::SimpleMath;

class GUN : public CGameObject {
private:
    // 
    CStaticMesh m_Mesh;
    CStaticMeshRenderer m_StaticMeshRenderer;
   
    // 
    Vector3 m_ShootPosition = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 m_ShootRotation = Vector3(0.0f, 0.0f, 0.0f);

    CStaticMesh m_Mesh2;
    CStaticMeshRenderer m_StaticMeshRenderer2;

    Vector3 m_ShootPosition2 = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 m_ShootRotation2 = Vector3(0.0f, 0.0f, 0.0f);

    Vector3 m_Rotation2 = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 m_Position2 = Vector3(0.0f, 0.0f, 0.0f);

    
    CShader m_Shader;
public:
    // 
    static GUN& GetInstance() {
        static GUN instance; // 
        return instance;
    }

    // 
    void Init() override;
    void Update() override;
    void Draw() override;
    void Dispose() override;
    void Luck();

    // Getter 和 Setter
  
    bool GUNUSE = false;
 
    Vector3* GetShootPosition() { return &m_ShootPosition; }
    Vector3* GetShootRotation() { return &m_ShootRotation; }

    Vector3* GetPosition() { return &m_Position; }
    Vector3* GetRotation() { return &m_Rotation; }


    Vector3* GetShootPosition2() { return &m_ShootPosition2; }
    Vector3* GetShootRotation2() { return &m_ShootRotation2; }

    Vector3* GetPosition2() { return &m_Position2; }
    Vector3* GetRotation2() { return &m_Rotation2; }

    void SetPosition2(const Vector3& position) { m_Position2 = position; }
	
};
GUN* GetGUN(); // 