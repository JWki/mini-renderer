
#include <stdint.h>
#include <assert.h>

#include "Math/math_types.h"
#include "Math/math_functions.h"


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

#define PAR_SHAPES_IMPLEMENTATION
#pragma warning(push, 0)
#include "par_shapes-h.h"
#pragma warning(pop)

#define ARRAY_SIZE(Array) (sizeof(Array)/sizeof((Array)[0]))
#define BACKBUFFER_COUNT 2

#define MINI_ASSERT(condition, msgFmt, ...) \
assert(condition)

static bool g_exitFlag = false;

namespace mini {
    /*
        *   Win32 High Precision Timer
    */
    class Timer
    {
        uint64_t m_frequency;
        uint64_t m_timestamp;
    public:
        Timer()
        {
            LARGE_INTEGER frequency, timestamp;
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&timestamp);

            m_frequency = frequency.QuadPart;
            m_timestamp = timestamp.QuadPart;
        }

        void Reset() {
            LARGE_INTEGER timestamp;
            QueryPerformanceCounter(&timestamp);
            m_timestamp = timestamp.QuadPart;
        }

        double GetElapsedTime() {
            LARGE_INTEGER currentTime = {};
            QueryPerformanceCounter(&currentTime);
            return static_cast<double>(currentTime.QuadPart - m_timestamp) * 1000.0f / static_cast<double>(m_frequency);
        }
    };
}

/*

*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = 0;
    switch (msg) {
        case WM_QUIT:
        case WM_CLOSE: 
            g_exitFlag = true;
        break;
        default: 
            res = DefWindowProc(hWnd, msg, wParam, lParam);
        break;
    }
    return res;
} 

/*

*/
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    FILE* file = nullptr;
    freopen_s(&file, "CON", "w", stdout);

    // Window Setup
    WNDCLASS windowClass = {};
    windowClass.lpfnWndProc = WndProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.lpszClassName = L"MiniWindow";
    RegisterClass(&windowClass);

    HWND hWnd = CreateWindowEx(0, windowClass.lpszClassName, L"Mini [D3D12] : Hello World", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
    
    // D3D12 Setup

    ID3D12Debug* d3dDebugController = nullptr;
    {
        auto res = D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDebugController));
        MINI_ASSERT(SUCCEEDED(res), "Failed to retrieve D3D12 debug interface");
        if (FAILED(res)) {
            return -1;
        }
    }
    d3dDebugController->EnableDebugLayer();

    ID3D12Device* d3dDevice = nullptr;
    IDXGIFactory2* dxgiFactory = nullptr;
    ID3D12CommandQueue* graphicsQueue = nullptr;

    // @todo pull these together into a structure
    IDXGISwapChain3* swapchain = nullptr;
    ID3D12Resource* backbuffers[BACKBUFFER_COUNT] = {};
    ID3D12Resource* depthBuffer = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RTVs[BACKBUFFER_COUNT] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE DSV = {};

    ID3D12GraphicsCommandList* cmdList = nullptr;
    ID3D12CommandAllocator* cmdAllocator = nullptr;
    ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
    ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
   
    uint32_t backbufferIdx = 0;

    {   // Device Creation
        // @todo Adapter selection
        auto minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;  // D3D11_FEATURE_LEVEL_12_0;
        auto res = D3D12CreateDevice(0, minimumFeatureLevel, IID_PPV_ARGS(&d3dDevice));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create D3D12 device");
        if (FAILED(res)) {
            return -1;
        }
    }
    {   // DXGI Factory creation
        auto res = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create DXGI factory");
        if (FAILED(res)) {
            return -1;
        }
    }
    {   // Graphics command queue creation
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        auto res = d3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&graphicsQueue));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create graphics command queue");
        if (FAILED(res)) {
            return -1;
        }
    }
    auto swapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;  // @note we need this later for PSO creation
    auto depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    {   // Swapchain creation
        IDXGISwapChain1* tempSwapchain = nullptr;   // create a basic swapchain first, then retrieve IDXGISwapChain3 interface via QueryInterface
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width = 0;
        desc.Height = 0;
        desc.Format = swapchainFormat;
        desc.Stereo = FALSE;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = BACKBUFFER_COUNT;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        desc.Flags = 0;
        // @note for a d3d12 compatible swap chain, instead of the device we need to pass the presenting command queue here
        auto res = dxgiFactory->CreateSwapChainForHwnd(graphicsQueue, hWnd, &desc, 0, 0, &tempSwapchain);
        MINI_ASSERT(SUCCEEDED(res), "Failed to create basic swapchain");
        if (FAILED(res)) {
            return -1;
        }
        // query advanced interface 
        res = tempSwapchain->QueryInterface(IID_PPV_ARGS(&swapchain));
        MINI_ASSERT(SUCCEEDED(res), "Failed to query advanced swapchain interface");
        if (FAILED(res)) {
            return -1;
        }
        backbufferIdx = swapchain->GetCurrentBackBufferIndex();
    }
    {   // command allocator and list creation
        auto res = d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create command allocator");
        if (FAILED(res)) {
            return -1;
        }
        res = d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, 0, IID_PPV_ARGS(&cmdList));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create command list");
        if (FAILED(res)) {
            return -1;
        }
        cmdList->Close();
    }
    
    {   // RTV resource heap creation
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = BACKBUFFER_COUNT;     // @note allocate only the RTVs needed for the swapchain RTVs
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;
        auto res = d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create RTV descriptor heap");
        if (FAILED(res)) {
            return -1;
        }
    }
    {   // DSV resource heap creation
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.NumDescriptors = 1; 
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;
        auto res = d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvDescriptorHeap));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create DSV descriptor heap");
        if (FAILED(res)) {
            return -1;
        }
    }
    {   // create RTVs for swapchain backbuffers
        
        UINT descriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < BACKBUFFER_COUNT; ++i) {
            RTVs[i] = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            RTVs[i].ptr += i * descriptorSize;

            auto res = swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i]));
            MINI_ASSERT(SUCCEEDED(res), "Failed to retrieve backbuffer at index #%i", i);
            d3dDevice->CreateRenderTargetView(backbuffers[i], nullptr, RTVs[i]);
        }
    }
    {   // Create main depth buffer as well as a DSV 
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.DepthOrArraySize = 1;
        desc.Format = depthFormat;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Width = backbuffers[0]->GetDesc().Width;
        desc.Height = backbuffers[0]->GetDesc().Height;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    

        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;


        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = depthFormat;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;
        auto res = d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthBuffer));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create depth buffer");
        
        DSV = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        d3dDevice->CreateDepthStencilView(depthBuffer, nullptr, DSV);
    }

    /*
        ***
    */

    ID3D12RootSignature* rootSig = nullptr;
    ID3DBlob* vsCodeBlob = nullptr;
    ID3DBlob* psCodeBlob = nullptr;
    ID3D12PipelineState* pso = nullptr;

    {   // setup root signature 

        D3D12_ROOT_PARAMETER params[64];
           
        {   // Reserve space for one float4x4 matrix to be used as mvp transform and uploaded via root constant
            // @note    apparently, this isn't the most efficient way to deal with a transformation matrix but it'll do for now
            auto& param = params[0];
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            param.Constants.Num32BitValues = 16;    
            param.Constants.RegisterSpace = 0;
            param.Constants.ShaderRegister = 0;
        }

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = 1;
        desc.pParameters = params;
        desc.NumStaticSamplers = 0;
        desc.pStaticSamplers = nullptr;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        
        ID3DBlob* serializedSignature = nullptr;
        auto res = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &serializedSignature, 0);
        MINI_ASSERT(SUCCEEDED(res), "Failed to serialize root signature");

        res = d3dDevice->CreateRootSignature(0, serializedSignature->GetBufferPointer(), serializedSignature->GetBufferSize(), IID_PPV_ARGS(&rootSig));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create root signature");
    }
    {   // load and compile shaders from file
        auto res = D3DCompileFromFile(L"src/Runtime/Shader.hlsl", 0, 0, "VSMain", "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vsCodeBlob, 0);
        MINI_ASSERT(SUCCEEDED(res), "Failed to compile vertex shader");
        res = D3DCompileFromFile(L"src/Runtime/Shader.hlsl", 0, 0, "PSMain", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &psCodeBlob, 0);
        MINI_ASSERT(SUCCEEDED(res), "Failed to compile pixel shader");
    }   
    {   // setup PSO

        D3D12_INPUT_ELEMENT_DESC inputElements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = rootSig;
        desc.VS = { vsCodeBlob->GetBufferPointer(), vsCodeBlob->GetBufferSize() };
        desc.PS = { psCodeBlob->GetBufferPointer(), psCodeBlob->GetBufferSize() };
        desc.BlendState.AlphaToCoverageEnable = FALSE;
        desc.BlendState.IndependentBlendEnable = FALSE;
        for (auto i = 0; i < ARRAY_SIZE(desc.BlendState.RenderTarget); ++i) {
            desc.BlendState.RenderTarget[i].BlendEnable = FALSE;
            desc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
            desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }
        desc.SampleMask = UINT_MAX;
        desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        desc.RasterizerState.FrontCounterClockwise = FALSE;
        desc.DepthStencilState.DepthEnable = TRUE;
        desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        desc.DepthStencilState.StencilEnable = FALSE;
        desc.InputLayout.pInputElementDescs = inputElements;
        desc.InputLayout.NumElements = ARRAY_SIZE(inputElements);
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = swapchainFormat;
        desc.DSVFormat = depthFormat;     
        desc.SampleDesc.Count = 1;
        desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        auto res = d3dDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create graphics pipeline state object");
    }

    /*
        ***
    */

    char* meshData = nullptr;   // @note we share an allocation for vertices and indices
    char* vertices = nullptr;   
    char* indices = nullptr;
    uint32_t vertexBufferSize = 0;
    uint32_t vertexBufferStride = 0;
    uint32_t indexBufferSize = 0;
    static_assert(sizeof(uint16_t) == sizeof(PAR_SHAPES_T));

    {   // create a procedural cube mesh, copy out positions and normals 
        auto cubeMesh = par_shapes_create_cube();
        par_shapes_translate(cubeMesh, -0.5f, -0.5f, -0.5f);
        par_shapes_unweld(cubeMesh, true);
        par_shapes_compute_normals(cubeMesh);
        
        vertexBufferSize = sizeof(float) * 3 * (cubeMesh->npoints * 2);
        vertexBufferStride = sizeof(float) * 6;
        
        indexBufferSize = sizeof(uint16_t) * 3 * cubeMesh->ntriangles;

        vertices = meshData = reinterpret_cast<char*>(malloc(vertexBufferSize + indexBufferSize));
        for (auto i = 0; i < cubeMesh->npoints; ++i) {

            auto writePtr = vertices + vertexBufferStride * i;
            memcpy(writePtr, cubeMesh->points + i * 3, sizeof(float) * 3);
            writePtr += sizeof(float) * 3;
            memcpy(writePtr, cubeMesh->normals + i * 3, sizeof(float) * 3);
        }
        indices = meshData + vertexBufferSize;
        memcpy(indices, cubeMesh->triangles, indexBufferSize);
    }


    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* indexBuffer = nullptr;

    {   // Vertex Buffer
        // @todo    Use a staging buffer for uploading and issue a GPU copy into GPU exclusive memory 
        //          instead of using an upload heap for the vertex data 
        
        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = vertexBufferSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        // @note we allow the runtime to manage our memory allocation here
        auto res = d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, 0, IID_PPV_ARGS(&vertexBuffer));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create commited resource for vertex buffer");
        
     
        D3D12_RANGE readRange = {};
        void* map = nullptr;
        res = vertexBuffer->Map(0, &readRange, &map);
        MINI_ASSERT(SUCCEEDED(res), "Failed to map vertex buffer");
        memcpy(map, vertices, vertexBufferSize);
        vertexBuffer->Unmap(0, 0);
    }

    {   // Vertex Buffer
        // @todo    Use a staging buffer for uploading and issue a GPU copy into GPU exclusive memory 
        //          instead of using an upload heap for the vertex data 

        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = indexBufferSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        // @note we allow the runtime to manage our memory allocation here
        auto res = d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, 0, IID_PPV_ARGS(&indexBuffer));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create commited resource for index buffer");


        D3D12_RANGE readRange = {};
        void* map = nullptr;
        res = indexBuffer->Map(0, &readRange, &map);
        MINI_ASSERT(SUCCEEDED(res), "Failed to map index buffer");
        memcpy(map, indices, indexBufferSize);
        indexBuffer->Unmap(0, 0);
    }

    // @note we can free our mesh data here since we don't have a reason to keep it around any longer
    free(meshData);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = vertexBufferSize;
    vertexBufferView.StrideInBytes = vertexBufferStride;

    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = indexBufferSize;
    indexBufferView.Format = DXGI_FORMAT_R16_UINT;


    UINT64 frameFenceValue = 0;
    ID3D12Fence* frameFence = nullptr;
    HANDLE frameFenceEvent = NULL;
    {
        auto res = d3dDevice->CreateFence(frameFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frameFence));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create frame fence");
        frameFenceEvent = CreateEvent(0, 0, FALSE, 0);
        MINI_ASSERT(frameFenceEvent != NULL, "Failed to create frame fence event");
    }
    
    mini::Timer timer;
    do {

        auto frameTime = timer.GetElapsedTime();
        timer.Reset();
        printf("\rFrame Time : %fms", frameTime);

        MSG msg = {};
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //
        cmdAllocator->Reset();
        cmdList->Reset(cmdAllocator, pso);

        auto backbuffer = backbuffers[backbufferIdx];
        auto backbufferRTV = RTVs[backbufferIdx];

        D3D12_VIEWPORT viewport = {};
        viewport.Width = (float)backbuffer->GetDesc().Width;
        viewport.Height = (float)backbuffer->GetDesc().Height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        D3D12_RECT scissorRect = {};
        scissorRect.right = (LONG)backbuffer->GetDesc().Width;
        scissorRect.bottom = (LONG)backbuffer->GetDesc().Height;

        cmdList->SetGraphicsRootSignature(rootSig);
        cmdList->RSSetViewports(1, &viewport);
        cmdList->RSSetScissorRects(1, &scissorRect);

        {   // transition backbuffer to render target state from present state
            // @note    check if this is the correct transition for the first frame
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = backbuffer;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            cmdList->ResourceBarrier(1, &barrier);
        }

        const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        
        cmdList->OMSetRenderTargets(1, &backbufferRTV, FALSE, &DSV);
        cmdList->ClearRenderTargetView(backbufferRTV, clearColor, 0, nullptr);
        cmdList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
        cmdList->IASetIndexBuffer(&indexBufferView);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const auto proj = mini::math::make_perspective_proj(mini::math::DegToRad(60.0f), viewport.Width / viewport.Height, 0.1f, 100.0f);
        const auto view = mini::math::inverse(mini::math::make_lookat(mini::math::vec3f_t(0.0f, 1.5f, -2.0f), mini::math::vec3f_t(), mini::math::vec3f_t(0.0f, 1.0f, 0.0f)));
        static float rot = 0.0f;
        rot += 0.005f;
        const auto model = mini::math::make_rotation(mini::math::vec3f_t(0.0f, 1.0f, 0.0f), rot);
        const auto mvp = proj * view * model;
        cmdList->SetGraphicsRoot32BitConstants(0, 16, mvp.elements, 0);
        cmdList->DrawIndexedInstanced(indexBufferSize / sizeof(uint16_t), 1, 0, 0, 0);

        {   // transition backbuffer to present state from render target state
            // @note    check if this is the correct transition for the first frame
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = backbuffer;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            cmdList->ResourceBarrier(1, &barrier);
        }

        cmdList->Close();

        ID3D12CommandList* submits[] = { cmdList };
        graphicsQueue->ExecuteCommandLists(1, submits);

        swapchain->Present(1, 0);

        // @note    we wait for the entire frame to finish executing before we continue with the next frame
        //          this is not a good thing long term 
        frameFenceValue++;
        graphicsQueue->Signal(frameFence, frameFenceValue);
        if (frameFence->GetCompletedValue() < frameFenceValue) {
            frameFence->SetEventOnCompletion(frameFenceValue, frameFenceEvent);
            WaitForSingleObject(frameFenceEvent, INFINITE);
        }
        backbufferIdx = swapchain->GetCurrentBackBufferIndex();

    } while (!g_exitFlag);

    return 0;
}

