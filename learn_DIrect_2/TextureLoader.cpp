/*
    TextureLoader.cpp

*/
#include "TextureLoader.h"






HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** srv) {
    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratchImage;
    HRESULT hr;

    // 尝试从 DDS 文件加载纹理
    hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, &metadata, scratchImage);

    if (FAILED(hr)) {
        // 如果加载失败，通常是文件不存在或格式不正确
        // 可以在这里添加 MessageBox 或日志输出，便于调试
        // MessageBox(nullptr, L"Failed to load DDS texture.", filename, MB_OK);

        return hr;
    }

    // 从加载的图片数据创建 Shader Resource View
    // 这个函数会自动处理 mipmaps 和纹理格式转换
    hr = DirectX::CreateShaderResourceView(device, scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, srv);
    return hr;
}