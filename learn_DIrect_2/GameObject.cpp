/*
    GameObject.cpp

*/
#include <windows.h>

#include "GameObject.h"
#include "BufferUtils.h"
#include "TextureLoader.h"
#include <DirectXMath.h>


using namespace DirectX;

GameObject::GameObject()
    : vertexBuffer(nullptr),
    indexBuffer(nullptr),
    constantBuffer(nullptr),
    textureSRV(nullptr),
    texOffset{ 0.0f, 0.0f },
    texScale{ 1.0f, 1.0f },
    fps(8.0f),
    animationTimer(0.0f),
    frameIndex(0),
    totalFrames(1),
    columns(1),
    rows(1),
    isAnimated(false),
    indexCount(0),
    modelMatrix(XMMatrixIdentity()) 
{
    texOffset[0] = texOffset[1] = 0.0f;
    texScale[0] = texScale[1] = 1.0f;
}

GameObject::~GameObject() {
    Release();
}

void GameObject::Release() {
    if (vertexBuffer) { vertexBuffer->Release(); vertexBuffer = nullptr; }
    if (indexBuffer) { indexBuffer->Release(); indexBuffer = nullptr; }
    if (constantBuffer) { constantBuffer->Release(); constantBuffer = nullptr; }
    if (textureSRV) { textureSRV->Release(); textureSRV = nullptr; }
}

bool GameObject::Load(
    ID3D11Device* device, 
    const std::wstring& texturePath,
    float left, float top, float right, float bottom,
    bool animated, 
    int totalFrames_, 
    int columns_, 
    int rows_, 
    float fps_
    ) {
    
    InitVertexData(device, left, top, right, bottom);

    isAnimated = animated;
    totalFrames = totalFrames_;
    columns = columns_;
    rows = rows_;
    fps = fps_;

    // 创建常量缓冲区
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&cbd, nullptr, &constantBuffer);

    // 加载纹理
    if (FAILED(LoadTextureAndCreateSRV(device, texturePath.c_str(), &textureSRV, &textureWidth, &textureHeight))) {
        return false;
    }
   

    // 设置非动画图的纹理偏移和缩放
    if (!isAnimated) {
        texOffset[0] = 0.0f;
        texOffset[1] = 0.0f;
        texScale[0] = 1.0f;
        texScale[1] = 1.0f;
    }


    return true;
}

void GameObject::InitVertexData(ID3D11Device* device, float left, float top, float right, float bottom) {
    Vertex vertices[] = {
       { { left,  top,    0.0f }, { 1, 1, 1, 1 }, { 0.0f, 0.0f } },
        { { right, top,    0.0f }, { 1, 1, 1, 1 }, { 1.0f, 0.0f } },
        { { right, bottom, 0.0f }, { 1, 1, 1, 1 }, { 1.0f, 1.0f } },
        { { left,  bottom, 0.0f }, { 1, 1, 1, 1 }, { 0.0f, 1.0f } }
    };
    vertexBuffer = CreateQuadVertexBuffer(device, vertices, 4);
    indexBuffer = CreateQuadIndexBuffer(device);
    indexCount = 6;
}

void GameObject::Update(float deltaTime) {
    if (!isAnimated) return;

    animationTimer += deltaTime;
    if (animationTimer >= (1.0f / fps)) {
        frameIndex = (frameIndex + 1) % totalFrames;
        animationTimer = 0.0f;
    }

    float frameW = 1.0f / static_cast<float>(columns);
    float frameH = 1.0f / static_cast<float>(rows);
    int col = frameIndex % columns;
    int row = frameIndex / columns;

    texOffset[0] = static_cast<float>(col) * frameW;
    texOffset[1] = static_cast<float>(row) * frameH;
    texScale[0] = frameW;
    texScale[1] = frameH;
}

void GameObject::UpdateConstantBuffer(ID3D11DeviceContext* context,
    const XMMATRIX& view, const XMMATRIX& projection) {
    
    //将 GPU 的 constantBuffer 映射到 CPU-accessible memory, 可以写入新数据
    //Direct3D 映射（Map）之后，提供给你的 一块可以写入的内存地址，类型是 void*（无类型指针)
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    
    if (SUCCEEDED(context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        ConstantBuffer* cb = (ConstantBuffer*)mappedResource.pData;
        cb->model = DirectX::XMMatrixTranspose(modelMatrix);
        cb->view = DirectX::XMMatrixTranspose(view);
        cb->projection = DirectX::XMMatrixTranspose(projection);
        cb->texOffset[0] = texOffset[0];
        cb->texOffset[1] = texOffset[1];
        cb->texScale[0] = texScale[0];
        cb->texScale[1] = texScale[1];
        context->Unmap(constantBuffer, 0);
    }
}

void GameObject::Render(ID3D11DeviceContext* context) {
    // 更新顶点缓冲区步幅 (stride)
   // 确保这里的 stride 与你的 Vertex 结构体大小一致
    UINT stride = sizeof(Vertex);
    //offset（偏移量）：从顶点缓冲区开始处偏移多少字节读取数据，这里是0，表示从缓冲区头开始。
    UINT offset = 0;
    // 设置顶点缓冲区
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    // 设置索引缓冲区 指定每个索引是一个 32 位无符号整数
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    // 之后HLSL 的 PSMain 中用到 cbuffer ConstantBuffer时，再用
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    // PSSetShaderResources(起始槽位, 视图数量, SRV数组指针)
    // t0 寄存器对应起始槽位 0
    context->PSSetShaderResources(0, 1, &textureSRV);
    //
    context->DrawIndexed(indexCount, 0, 0);
}
