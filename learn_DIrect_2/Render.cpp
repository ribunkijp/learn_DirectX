#include "Render.h"









void RenderFrame(HWND hwnd, StateInfo* pState) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    (void)hdc;



    /*
        这段代码是为了实现动态更新GPU端的常量缓冲区，使着色器能够获取当前窗口大小，
        保证渲染效果能正确适配窗口尺寸变化。
    */
    // 取得最新窗口大小（防止窗口大小变化不更新）
    RECT rect;
    //检索窗口客户区 (可绘制区域，不包括边框和标题栏) 的尺寸
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);





    // 绑定渲染目标视图和深度/模板视图
    pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);


    // 清除背景色
    float clearColor[4] = { 0.0f, 0.3f, 0.0f, 1.0f };
    pState->context->ClearRenderTargetView(pState->rtv, clearColor);

    //清除深度和模板缓冲区。1.0f是深度的默认最远值。
    pState->context->ClearDepthStencilView(pState->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // 绑定渲染目标
    pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);

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
    //
    pState->context->VSSetConstantBuffers(0, 1, &pState->constantBuffer);
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



    // 更新顶点缓冲区步幅 (stride)
    // 确保这里的 stride 与你的 Vertex 结构体大小一致
    UINT stride = sizeof(Vertex);
    //offset（偏移量）：从顶点缓冲区开始处偏移多少字节读取数据，这里是0，表示从缓冲区头开始。
    UINT offset = 0;



    for (auto& obj : sceneObjects)
    {
        ID3D11Buffer* vertexBuffers[] = { obj.vertexBuffer };
        // 设置顶点缓冲区
        pState->context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
        // 设置索引缓冲区 指定每个索引是一个 32 位无符号整数
        pState->context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);


        // --- 绑定纹理和采样器到像素着色器 ---
        // PSSetShaderResources(起始槽位, 视图数量, SRV数组指针)
        // t0 寄存器对应起始槽位 0
        pState->context->PSSetShaderResources(0, 1, &obj.textureSRV);
        // PSSetSamplers(起始槽位, 采样器数量, 采样器数组指针)
        // s0 寄存器对应起始槽位 0
        pState->context->PSSetSamplers(0, 1, &pState->samplerState);
        // --- 绑定结束 ---


        // 设置常量缓冲区，传入 obj.worldMatrix
        UpdateConstantBuffer(pState->context, pState->constantBuffer, obj.worldMatrix, width, height);

        // 绘制调用
        pState->context->DrawIndexed(obj.indexCount, 0, 0);
    }



    // 将后备缓冲区 (已完成渲染的缓冲区) 与前台缓冲区 (当前显示在屏幕上的缓冲区) 交换
    pState->swapChain->Present(1, 0);

    EndPaint(hwnd, &ps);
}