#ifndef STATEINFO_H
#define STATEINFO_H

#include <d3d11.h>//ID3D11Device
#include <DirectXMath.h>
#include <vector>
#include "GameObject.h"
#include <memory>

// 管理 Direct3D11 渲染状态的结构体
struct StateInfo {
    // 设备对象：用于创建资源和着色器
    ID3D11Device* device = nullptr; //ID3D11Device（显卡接口）

    // 设备上下文：用于发出绘制命令和绑定资源 //D3D11DeviceContext（命令接口）
    ID3D11DeviceContext* context = nullptr;//渲染管线

    
    /*
        交换链：管理后台缓冲区与前台缓冲区的交换（用于屏幕显示）通常包含一个或多个后备缓冲区 (Back Buffers)，
        渲染时画面首先绘制到这些缓冲区，然后调用 Present() 将当前缓冲区“交换”到前台显示。
    */
    IDXGISwapChain* swapChain = nullptr;

    // 渲染目标视图：指定绘制目标缓冲区的视图
    ID3D11RenderTargetView* rtv = nullptr;

    // 输入布局：将顶点数据的结构传达给 GPU 的设置
    ID3D11InputLayout* inputLayout = nullptr;

    // 顶点着色器：对每个顶点进行处理的着色器
    ID3D11VertexShader* vertexShader = nullptr;

    // 像素着色器：用于决定每个像素颜色的着色器
    ID3D11PixelShader* pixelShader = nullptr;


    //用于纹理采样的采样器状态
    ID3D11SamplerState* samplerState = nullptr;

    // 用于透明混合
    ID3D11BlendState* blendState = nullptr;
    // 用于透明物体的深度状态
    ID3D11DepthStencilState* depthStencilStateTransparent = nullptr;
    //深度/模板缓冲区视图 的接口指针  GPU 用来访问和操作深度/模板缓冲区的接口
    ID3D11DepthStencilView* depthStencilView = nullptr;
    //
    ID3D11RenderTargetView* renderTargetView = nullptr;
    //
    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX projection = DirectX::XMMatrixIdentity();
    //
    float logicalWidth = 1888.0f;
    float logicalHeight = 1062.0f;
    //
    std::vector<std::unique_ptr<GameObject>> sceneObjects;
  

    


};










#endif