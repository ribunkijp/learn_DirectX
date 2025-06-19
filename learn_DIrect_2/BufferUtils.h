/*
    BufferUtils.h

*/


#include "d3dApp.h"




ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device, Vertex* vertices, unsigned vertices_count);

ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device);

void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX& worldMatrix, float screen_width, float screen_height);