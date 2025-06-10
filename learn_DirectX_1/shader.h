/*==============================================================================

   シェーダー [shader.h]
                                                          Author : Youhei Sato
                                                          Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_H
#define	SHADER_H

#include <d3d11.h>
#include <DirectXMath.h>

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Shader_Finalize();

void Shader_SetMatrix(const DirectX::XMMATRIX& matrix);

void Shader_Begin();

// InputLayout作成のために、コンパイル済みの頂点シェーダーの
// バイナリデータ(Blob)を取得する関数を追加します。
ID3DBlob* Shader_GetVertexShaderBlob(void);

#endif // SHADER_H