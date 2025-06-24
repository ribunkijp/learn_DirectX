/*
    ObjectFactory.cpp

*/

#include "ObjectFactory.h"
#include "BufferUtils.h"








GameObject CreateTexture(ID3D11Device* device,
    const wchar_t* texturePath,
    float left, 
    float top, 
    float right, 
    float bottom,
    bool isAnimated, 
    int totalFrames,
    float texOffset[2], 
    float texScale[2],
    int columns, 
    int rows, 
    float fps,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection) {
    
    GameObject obj;

    const UINT quadIndexCount = 6;   // 6个索引（三角形2个，每个3个顶点
    
    Vertex bg_vertices[] = {
        { { left, top, 0.0f }, {1, 0, 0, 1}, {0.0f, 0.0f} }, // 左上角
        { { right, top, 0.0f }, {0, 1, 0, 1}, {1.0f, 0.0f} }, // 右上角
        { { right, bottom, 0.0f }, {0, 0, 1, 1}, {1.0f, 1.0f} }, // 右下角
        { { left, bottom, 0.0f }, {1, 1, 0, 1}, {0.0f, 1.0f} }  // 左下角
    };

    // 假设你有一个函数用来创建2D矩形顶点缓冲区
    obj.vertexBuffer = CreateQuadVertexBuffer(device, bg_vertices, 4);
    // 创建索引缓冲区
    obj.indexBuffer = CreateQuadIndexBuffer(device);
    obj.indexCount = quadIndexCount;  // 6，两个三角形的索引数量
    
    obj.constantBufferData.model = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f); // 初始位置
    obj.constantBufferData.view = view;
    obj.constantBufferData.projection = projection;


    //初始化 texOffset / texScale
    obj.constantBufferData.texOffset[0] = texOffset[0];
    obj.constantBufferData.texOffset[1] = texOffset[1];
    obj.constantBufferData.texScale[0] = texScale[0];
    obj.constantBufferData.texScale[1] = texScale[1];
    obj.isAnimated = isAnimated;
    obj.totalFrames = totalFrames;
    obj.columns = columns;
    obj.rows = rows;
    obj.fps = fps;


    //创建每个物体自己的 constant buffer
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateBuffer(&cbd, nullptr, &obj.constantBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create constant buffer.", L"Error", MB_OK);
    }


    // 加载纹理文件，请确保 'bg.dds' 存在于你的可执行文件同级目录
    hr = LoadTextureAndCreateSRV(device, texturePath, &obj.textureSRV);
    if (FAILED(hr)) {

        MessageBox(nullptr, L"Failed to load texture1.dds. Please check if the file exists and is a valid DDS.", L"Error", MB_OK);
    }

    return obj;
}
