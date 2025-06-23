/*
    BufferUtils.cpp

*/

#include "BufferUtils.h"







ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device, Vertex* vertices, unsigned vertices_count) {



    D3D11_BUFFER_DESC bd = {};// Direct3D 11 用来描述缓冲区属性的结构体
    bd.Usage = D3D11_USAGE_DEFAULT;              // 指示缓冲区将由 GPU 读取和写入。创建后不允许 CPU 直接访问。
    bd.ByteWidth = sizeof(Vertex) * vertices_count;             // 缓冲区大小 = 顶点总大小
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;     // 用作顶点缓冲区

    D3D11_SUBRESOURCE_DATA initData = {};//D3D11_SUBRESOURCE_DATA 是用来初始化缓冲区数据的结构体。
    initData.pSysMem = vertices;// 提供用于在 GPU 上填充缓冲区的初始数据指针

    //定义一个指向 Direct3D 11 顶点缓冲区的指针，初始值设为 nullptr（空指针）
    ID3D11Buffer* vertexBuffer = nullptr;
    //创建一个 GPU 上的缓冲区。
    HRESULT hr = device->CreateBuffer(&bd, &initData, &vertexBuffer);
    if (FAILED(hr)) {
        // 错误处理
        return nullptr;
    }
    return vertexBuffer;
}
ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device)
{
    // 定义矩形的索引数组，共6个索引，绘制两个三角形组成一个矩形
    // 三角形1顶点索引: 0, 1, 2
    // 三角形2顶点索引: 0, 2, 3
    UINT indices[6] = { 0, 1, 2, 0, 2, 3 };

    // 初始化缓冲区描述结构体
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;                // 默认使用方式，GPU可读写，CPU不可访问
    bd.ByteWidth = sizeof(UINT) * 6;               // 缓冲区大小，6个索引，每个索引是UINT类型
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;        // 绑定标志，表示该缓冲区是索引缓冲区

    // 定义初始化数据结构，告诉D3D缓冲区初始化时使用哪个内存的数据
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;                     // 指向索引数组的指针

    // 创建索引缓冲区指针，初始值为空
    ID3D11Buffer* indexBuffer = nullptr;

    // 调用设备接口，创建索引缓冲区
    HRESULT hr = device->CreateBuffer(&bd, &initData, &indexBuffer);
    if (FAILED(hr))
    {
        // 创建失败，返回nullptr，可以在调用处检测失败并处理
        return nullptr;
    }

    // 创建成功，返回索引缓冲区指针
    return indexBuffer;
}

// 更新常量缓冲区的函数
void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, const ConstantBuffer& cbData)
{
  



    //将 GPU 的 constantBuffer 映射到 CPU-accessible memory, 可以写入新数据
    D3D11_MAPPED_SUBRESOURCE mappedResource;//Direct3D 映射（Map）之后，提供给你的 一块可以写入的内存地址，类型是 void*（无类型指针)
    HRESULT hr = context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        // 失败处理
        return;
    }
    

    //取得映射后的内存指针，类型转换为定义的 ConstantBuffer 结构体
    ConstantBuffer* pCb = (ConstantBuffer*)mappedResource.pData;

    //
    pCb->worldMatrix = DirectX::XMMatrixTranspose(cbData.worldMatrix);//  // 先转置矩阵（DirectX一般用行主序，HLSL一般列主序）  
    
    
    
    // 对象的变换矩阵
    pCb->screenSize[0] = cbData.screenSize[0];        // 当前窗口宽度
    pCb->screenSize[1] = cbData.screenSize[1];      // 当前窗口高度
    
    pCb->texOffset[0] = cbData.texOffset[0];
    pCb->texOffset[1] = cbData.texOffset[1];
    pCb->texScale[0] = cbData.texScale[0];
    pCb->texScale[1] = cbData.texScale[1];

    ////取消映射 buffer，使更新后的数据可供 GPU 使用。
    context->Unmap(constantBuffer, 0);
}
