/*
    ObjectFactory.h

*/
#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <DirectXMath.h>

struct ID3D11Device;
struct alignas(16) GameObject;


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
    const DirectX::XMMATRIX& projection);




#endif