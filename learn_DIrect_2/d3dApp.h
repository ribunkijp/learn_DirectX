/*
    d3dApp.h

*/
#ifndef D3DAPP_H
#define D3DAPP_H


#include <new>
#include <d3d11.h>//ID3D11Device
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")//链接Direct3D 11 的静态链接库
#pragma comment(lib, "d3dcompiler.lib")//链接D3DCompiler API 的静态链接库
#include <DirectXMath.h>
#include <vector>
#include <DirectXTex.h>
#include "StateInfo.h"


//
struct alignas(16) GameObject {
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();  // 世界矩阵，表示物体的位置、旋转、缩放（默认单位矩阵）
    ID3D11Buffer* vertexBuffer = nullptr;  // 顶点缓冲区指针，存储物体的顶点数据（位置、颜色等）
    ID3D11Buffer* indexBuffer = nullptr;   // 索引缓冲区指针，用于顶点的索引绘制，减少顶点重复
    UINT vertexCount = 0;                   // 顶点数量，用于绘制调用时指定绘制的顶点个数
    UINT indexCount = 0;                    // 索引数量，用于绘制调用时指定绘制的索引个数
    ID3D11ShaderResourceView* textureSRV = nullptr;//用于存储此物体纹理的资源视图
};


//定义了将数据作为常量缓冲区传递给 GPU shaders 的布局
struct ConstantBuffer {
    //世界变换矩阵。这将按物体进行更新并发送到顶点着色器
    DirectX::XMMATRIX worldMatrix;  // 64 字节 (4x4 float)
    //一个数组，用于存储屏幕的宽度和高度。这在 shader 中用于将像素坐标转换为 Normalized Device Coordinates (NDC)。
    float screenSize[2]; // 屏幕宽高
    float padding[2];    // 保持16字节对齐
};
//
struct Vertex {
    DirectX::XMFLOAT3 position;  // 位置
    DirectX::XMFLOAT4 color;     // 颜色
    DirectX::XMFLOAT2 texCoord;  // 纹理坐标 (U, V)
};

// 获取屏幕的宽度和高度（单位：像素）
extern unsigned screenWidth;
extern unsigned screenHeight;



bool InitD3D(HWND hwnd, StateInfo* state);

ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device, Vertex* vertices, unsigned vertices_count);

ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device);

void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX& worldMatrix, float screenWidth, float screenHeight);

extern std::vector<GameObject> sceneObjects;

HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** srv);

void CleanupD3D(StateInfo* state);


#endif 