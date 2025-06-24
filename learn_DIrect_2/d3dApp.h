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




//定义了将数据作为常量缓冲区传递给 GPU shaders 的布局
struct ConstantBuffer {
    DirectX::XMMATRIX model;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;

    float texOffset[2] = { 0.0f, 0.0f };   // 表示从纹理的哪里开始采样 ,[0.0, 1.0] 范围内, {0.25, 0.0} 表示从纹理的横向 25% 开始采样
    float texScale[2] = { 1.0f, 1.0f };    // 表示采样区域的大小。{0.125, 1.0} 表示只采整张图的 1/8 宽度，高度全用

    float padding[2] = { 0.0f, 0.0f };    // 保持16字节对齐
};

//
struct alignas(16) GameObject {
    ID3D11Buffer* vertexBuffer = nullptr;  // 顶点缓冲区指针，存储物体的顶点数据（位置、颜色等）
    ID3D11Buffer* indexBuffer = nullptr;   // 索引缓冲区指针，用于顶点的索引绘制，减少顶点重复
    UINT vertexCount = 0;                   // 顶点数量，用于绘制调用时指定绘制的顶点个数
    UINT indexCount = 0;                    // 索引数量，用于绘制调用时指定绘制的索引个数
    ID3D11ShaderResourceView* textureSRV = nullptr;//用于存储此物体纹理的资源视图
    ID3D11Buffer* constantBuffer = nullptr;
    ConstantBuffer constantBufferData;  // 新增：用于存储每个物体的动画帧、屏幕信息等
    int totalFrames = 1;                // 动画帧数（比如雪碧图是8帧）
    bool isAnimated = false;

    //每个对象单独存储动画状态
    float animationTimer = 0.0f;
    int frameIndex = 0;

    //
    int columns = 1;
    int rows = 1;
    //
    float fps = 8.0f;
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

//
extern float clientWidth;
extern float clientHeight;

//
bool InitD3D(HWND hwnd, StateInfo* state, float clientWidth, float clientHeight);

//
extern std::vector<GameObject> sceneObjects;

//
HRESULT LoadTextureAndCreateSRV(
    ID3D11Device* device,                       // 用来创建资源的 D3D 设备
    const wchar_t* filename,                   // 纹理文件路径（通常是 DDS、PNG、JPG）
    ID3D11ShaderResourceView** srv             // 输出：创建好的 SRV 指针 返回给调用者

);

//
void CleanupD3D(StateInfo* state);


#endif 