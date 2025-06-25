/*
    GameObject.h

*/
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

//
struct Vertex {
    DirectX::XMFLOAT3 position;  // 位置
    DirectX::XMFLOAT4 color;     // 颜色
    DirectX::XMFLOAT2 texCoord;  // 纹理坐标 (U, V)
};


struct ConstantBuffer {
    DirectX::XMMATRIX model;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    float texOffset[2];
    float texScale[2];
    float padding[4];
};

class GameObject {
public:
    GameObject();
    ~GameObject();


    bool Load(
        ID3D11Device* device, 
        const std::wstring& texturePath,
        float left, float top, float right, float bottom,
        bool animated, 
        int totalFrames, 
        int columns, 
        int rows, 
        float fps);

    void Update(float deltaTime);
    void UpdateConstantBuffer(ID3D11DeviceContext* context,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection);

    void Render(ID3D11DeviceContext* context);

    void Release();
    
    void UpdateAsFullscreenBackground(float clientWidth, float clientHeight);

    float textureWidth = 0.0f;
    float textureHeight = 0.0f;


private:
    
    void InitVertexData(ID3D11Device* device, float left, float top, float right, float bottom);
    
    // GPU 资源
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* constantBuffer;
    ID3D11ShaderResourceView* textureSRV;

    // 动画控制
    float texOffset[2];
    float texScale[2];
    float fps;
    float animationTimer;
    int frameIndex;
    int totalFrames;
    int columns;
    int rows;
    bool isAnimated;

    // 渲染
    UINT indexCount;

    // 
    DirectX::XMMATRIX modelMatrix;
};

#endif
