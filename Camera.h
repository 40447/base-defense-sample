#pragma once

#include "CGameObject.h"
#include "CShader.h"
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "CDirectInput.h"
#include "CPlayer.h"





class Camera : public CGameObject {
private:  
    CStaticMesh m_Mesh;
    CStaticMeshRenderer m_StaticMeshRenderer;
    CShader m_Shader;

  
    Vector3 m_vDir;

    Vector3	m_vEye = Vector3(0.0f, 0.0f, 0.0f);
    Vector3	m_vLook{};
    Matrix		m_ViewMatrix{};

public:
    Camera();
    virtual ~Camera();

    static Camera& GetInstance() {
        static Camera instance; 
        return instance;
    }
    
   
    void Init() override;

   
    void Update() override;

   
    void Draw() override;

    void Dispose() override;

   

  
    Vector3 GetDirection() const { return m_vDir; }

    
    void SetDirection(const Vector3& dir) { m_vDir = dir; }


    
    Matrix GetPlayerWorldMat() const { return GetWorldMatrix(); }

    Vector3 GetPlayerVDir() const { return m_vDir; }

	Vector3 GetLook() const { return m_vLook; }
    
    Vector3 GetPosition() const { return m_Position; }

    void SetPosition(const Vector3& pos) { m_Position = pos; }

};
Camera* GetCamera();