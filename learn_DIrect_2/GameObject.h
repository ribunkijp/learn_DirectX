/**********************************************************************************
    GameObject.h

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

//state
enum class AnimationState {
    Idle,
    Walk
};
//direction
enum class Direction {
    Left,
    Right
};


// 頂点構造体
struct Vertex {
    DirectX::XMFLOAT3 position;  // 位置
    DirectX::XMFLOAT4 color;     // 色
    DirectX::XMFLOAT2 texCoord;  // テクスチャ座標 (U, V)
};


// 定数バッファ構造体
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
        float width, float height,
        bool animated,
        int totalFrames,
        int columns,
        int rows,
        float fps);

    void Update(float deltaTime);
    void UpdateConstantBuffer(ID3D11DeviceContext* context,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection);

    void Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection);

    void Release();

    float textureWidth = 0.0f;
    float textureHeight = 0.0f;

    

    float GetPosX() const;
    float GetPosY() const;
    float GetW() const;
    float GetH() const;

    void SetPos(float x, float y);

    float GetSpeed() const;
    void SetSpeed(float speed_new);
    AnimationState state = AnimationState::Idle;
    Direction direction = Direction::Right;

private:
    void InitVertexData(ID3D11Device* device, float width, float height);

    // GPUリソース
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* constantBuffer;
    ID3D11ShaderResourceView* walkLeft_textureSRV;
    ID3D11ShaderResourceView* walkRight_textureSRV;
    ID3D11ShaderResourceView* idelLeft_textureSRV;
    ID3D11ShaderResourceView* idelRight_textureSRV;

    // アニメーション制御
    float texOffset[2];
    float texScale[2];
    float fps;
    float animationTimer;
    int frameIndex;
    int totalFrames;
    int columns;
    int rows;
    bool isAnimated;
    float speed = 0.0f;
    float objW = 0.0f;
    float objH = 0.0f;
    // 描画
    UINT indexCount;

    // 
    DirectX::XMMATRIX modelMatrix;
};

#endif
