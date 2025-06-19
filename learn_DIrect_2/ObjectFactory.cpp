/*
    ObjectFactory.cpp

*/

#include "ObjectFactory.h"
#include "BufferUtils.h"








GameObject CreateTexture(ID3D11Device* device, const wchar_t* texturePath,float left, float top, float right, float bottom) {
    
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
    // worldMatrix 2D平移矩阵，Z轴一般为0
    obj.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);


    // 加载纹理文件，请确保 'bg.dds' 存在于你的可执行文件同级目录
    HRESULT hr = LoadTextureAndCreateSRV(device, texturePath, &obj.textureSRV);
    if (FAILED(hr)) {

        MessageBox(nullptr, L"Failed to load texture1.dds. Please check if the file exists and is a valid DDS.", L"Error", MB_OK);
    }

    return obj;
}
