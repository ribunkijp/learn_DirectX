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
bool InitD3D(HWND hwnd, StateInfo* state, float clientWidth, float clientHeight);


//
HRESULT LoadTextureAndCreateSRV(
    ID3D11Device* device,                       // 用来创建资源的 D3D 设备
    const wchar_t* filename,                   // 纹理文件路径（通常是 DDS、PNG、JPG）
    ID3D11ShaderResourceView** srv             // 输出：创建好的 SRV 指针 返回给调用者

);

//
void CleanupD3D(StateInfo* state);


//
void OnResize(HWND hwnd, StateInfo* state, UINT width, UINT height);


#endif 