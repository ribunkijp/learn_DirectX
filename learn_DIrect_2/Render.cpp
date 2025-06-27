/*
    Render.cpp

*/


#include "Render.h"








void RenderFrame(HWND hwnd, StateInfo* pState) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    (void)hdc;






    // 绑定渲染目标视图和深度/模板视图
    pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);


    // 清除背景色
    float clearColor[4] = { 1.0f, 1.0f, 0.88f, 1.0f };
    pState->context->ClearRenderTargetView(pState->rtv, clearColor);

    //清除深度和模板缓冲区。1.0f是深度的默认最远值。
    pState->context->ClearDepthStencilView(pState->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


    // 设置输入布局和着色器
    /*
        告诉GPU顶点数据长啥样，怎么读；

        告诉GPU顶点数据如何被顶点着色器处理；

        告诉GPU像素怎么被像素着色器处理。
    */
    //设置输入布局（Input Layout）
    pState->context->IASetInputLayout(pState->inputLayout);
    //绑定顶点着色器（Vertex Shader）到管线
    pState->context->VSSetShader(pState->vertexShader, nullptr, 0);
    //绑定像素着色器（Pixel Shader）到管线
    pState->context->PSSetShader(pState->pixelShader, nullptr, 0);

    // 作用是告诉 GPU 如何把顶点组织成图元来绘制。设置图元类型为三角形列表:D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    pState->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // --- 绑定混合状态 ---
   // 第二个参数是一个常数数组，用于那些在混合描述中设置为 D3D11_BLEND_BLEND_FACTOR 或 D3D11_BLEND_INV_BLEND_FACTOR 的混合模式。
   // 如果你的混合因子是基于源 Alpha 的，这里通常用 nullptr 或全为1的数组。
   // 第三个参数是样本遮罩，通常为 0xffffffff。
    pState->context->OMSetBlendState(pState->blendState, nullptr, 0xffffffff);

    //设置用于透明的深度模板状态！这是关键！
    // 第二个参数(StencilRef)在这里不重要，设为0即可。
    pState->context->OMSetDepthStencilState(pState->depthStencilStateTransparent, 0);



    // PSSetSamplers(起始槽位, 采样器数量, 采样器数组指针)
        // s0 寄存器对应起始槽位 0
    pState->context->PSSetSamplers(0, 1, &pState->samplerState);

    //pState->background.UpdateConstantBuffer(pState->context, pState->view, pState->projection);
    //pState->background.Render(pState->context);


    for (auto& obj : pState->sceneObjects)
    {
        obj->Render(pState->context);
    
    }



    // 将后备缓冲区 (已完成渲染的缓冲区) 与前台缓冲区 (当前显示在屏幕上的缓冲区) 交换
    pState->swapChain->Present(1, 0);

    EndPaint(hwnd, &ps);
}