/*
    ObjectFactory.h

*/
#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

struct ID3D11Device;
struct alignas(16) GameObject;

GameObject CreateTexture(ID3D11Device* device, const wchar_t* texturePath, float left, float top, float right, float bottom);




#endif