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
#include "StateInfo.h"


//
bool InitD3D(HWND hwnd, StateInfo* state, float clientWidth, float clientHeight);


//
void CleanupD3D(StateInfo* state);


//
void OnResize(HWND hwnd, StateInfo* state, UINT width, UINT height);


#endif 