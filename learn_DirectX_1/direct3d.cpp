/*==============================================================================

   Direct3D�̏������֘A [direct3d.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/12
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include "direct3d.h"
#include "debug_ostream.h"

#pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "dxgi.lib")

/* �e��C���^�[�t�F�[�X */
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;

/* �o�b�N�o�b�t�@�֘A */
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
static ID3D11Texture2D* g_pDepthStencilBuffer = nullptr;
static ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
static D3D11_TEXTURE2D_DESC g_BackBufferDesc{};

static bool configureBackBuffer(); // �o�b�N�o�b�t�@�̐ݒ�E����
static void releaseBackBuffer(); // �o�b�N�o�b�t�@�̉��


bool Direct3D_Initialize(HWND hWnd)
{
    /* �f�o�C�X�A�X���b�v�`�F�[���A�R���e�L�X�g���� */
    DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
    swap_chain_desc.Windowed = TRUE;
    swap_chain_desc.BufferCount = 2;
    // swap_chain_desc.BufferDesc.Width = 0;
    // swap_chain_desc.BufferDesc.Height = 0;
	// �� �E�B���h�E�T�C�Y�ɍ��킹�Ď����I�ɐݒ肳���
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swap_chain_desc.OutputWindow = hWnd;

	/*
	IDXGIFactory1* pFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	IDXGIAdapter1* pAdapter;
	pFactory->EnumAdapters1(1, &pAdapter); // �Z�J���_���A�_�v�^��擾
	pFactory->Release();
	DXGI_ADAPTER_DESC1 desc;
	pAdapter->GetDesc1(&desc); // �A�_�v�^�̏���擾���Ċm�F�������ꍇ
	pAdapter->Release(); // D3D11CreateDeviceAndSwapChain()�̑�P�����ɓn���ė��p���I�������������
	*/

	UINT device_flags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    
    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
 
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        device_flags,
        levels,
        ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        &swap_chain_desc,
        &g_pSwapChain,
        &g_pDevice,
        &feature_level,
        &g_pDeviceContext);

    if (FAILED(hr)) {
		MessageBox(hWnd, "Direct3D�̏������Ɏ��s���܂���", "�G���[", MB_OK);
        return false;
    }

	if (!configureBackBuffer()) {
		MessageBox(hWnd, "�o�b�N�o�b�t�@�̐ݒ�Ɏ��s���܂���", "�G���[", MB_OK);
		return false;
	}

    return true;
}

void Direct3D_Finalize()
{
	releaseBackBuffer();

	SAFE_RELEASE(g_pSwapChain);
	/*
	if (g_pSwapChain) {
		g_pSwapChain->Release();
		g_pSwapChain = nullptr;
	}
	*/

	SAFE_RELEASE(g_pDeviceContext);
	/*
	if (g_pDeviceContext) {
		g_pDeviceContext->Release();
		g_pDeviceContext = nullptr;
	}
	*/
	
	SAFE_RELEASE(g_pDevice);
	/*
    if (g_pDevice) {
		g_pDevice->Release();
		g_pDevice = nullptr;
	}
	*/
}

void Direct3D_Clear()
{
	float clear_color[4] = { 0.97f, 0.76f, 0.81f, 1.0f }; //0~1
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clear_color);
	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Direct3D_Present()
{
	// �X���b�v�`�F�[���̕\��
	g_pSwapChain->Present(1, 0);
}

bool configureBackBuffer()
{
    HRESULT hr;

    ID3D11Texture2D* back_buffer_pointer = nullptr;

	// �o�b�N�o�b�t�@�̎擾
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_pointer);

    if (FAILED(hr)) {
		hal::dout << "�o�b�N�o�b�t�@�̎擾�Ɏ��s���܂���" << std::endl;
        return false;
    }

	// �o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[�̐���
	hr = g_pDevice->CreateRenderTargetView(back_buffer_pointer, nullptr, &g_pRenderTargetView);

    if (FAILED(hr)) {
        back_buffer_pointer->Release();
        hal::dout << "�o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[�̐����Ɏ��s���܂���" << std::endl;
        return false;
    }

	// �o�b�N�o�b�t�@�̏�ԁi���j��擾
    back_buffer_pointer->GetDesc(&g_BackBufferDesc);

	back_buffer_pointer->Release(); // �o�b�N�o�b�t�@�̃|�C���^�͕s�v�Ȃ̂ŉ��

	// �f�v�X�X�e���V���o�b�t�@�̐���
	D3D11_TEXTURE2D_DESC depth_stencil_desc{};
	depth_stencil_desc.Width = g_BackBufferDesc.Width;
	depth_stencil_desc.Height = g_BackBufferDesc.Height;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.ArraySize = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_desc.SampleDesc.Count = 1;
	depth_stencil_desc.SampleDesc.Quality = 0;
	depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_desc.CPUAccessFlags = 0;
	depth_stencil_desc.MiscFlags = 0;
	hr = g_pDevice->CreateTexture2D(&depth_stencil_desc, nullptr, &g_pDepthStencilBuffer);

	if (FAILED(hr)) {
		hal::dout << "�f�v�X�X�e���V���o�b�t�@�̐����Ɏ��s���܂���" << std::endl;
		return false;
	}

	// �f�v�X�X�e���V���r���[�̐���
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = depth_stencil_desc.Format;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	depth_stencil_view_desc.Flags = 0;
	hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &depth_stencil_view_desc, &g_pDepthStencilView);

	if (FAILED(hr)) {
		hal::dout << "�f�v�X�X�e���V���r���[�̐����Ɏ��s���܂���" << std::endl;
		return false;
	}

    return true;
}

void releaseBackBuffer()
{
	SAFE_RELEASE(g_pRenderTargetView);
	/*
	if (g_pRenderTargetView) {
		g_pRenderTargetView->Release();
		g_pRenderTargetView = nullptr;
	}
	*/

	SAFE_RELEASE(g_pDepthStencilBuffer);
	/*
	if (g_pDepthStencilBuffer) {
		g_pDepthStencilBuffer->Release();
		g_pDepthStencilBuffer = nullptr;
	}
	*/

	SAFE_RELEASE(g_pDepthStencilView);
	/*
	if (g_pDepthStencilView) {
		g_pDepthStencilView->Release();
		g_pDepthStencilView = nullptr;
	}
	*/
}
