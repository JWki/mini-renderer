
#include <stdint.h>
#include <assert.h>


#include "Math/math_types.h"
#include "Math/math_functions.h"

#include <imgui/imgui.h>
#include <imgui/bindings/imgui_impl_dx12.h>
#include <imgui/bindings/imgui_impl_win32.h>

#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/fixed_function.h>


#include <Runtime/Renderer/rendergraph.h>
#include <Runtime/AssetLibraries/MeshLibrary.h>
#include <Runtime/Renderables/StaticMeshRenderer.h>

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

#include <fstream>

#define BACKBUFFER_COUNT 2

#include <Runtime/common.h>

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
            return static_cast<double>(currentTime.QuadPart - m_timestamp) / static_cast<double>(m_frequency);
        }
    };
}

/*
*/


/*

*/
IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return 0;
    }
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
    D3D12_CPU_DESCRIPTOR_HANDLE DSV = {};

    ID3D12GraphicsCommandList* cmdList = nullptr;
    ID3D12CommandAllocator* cmdAllocator = nullptr;
    ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
    ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
    ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;
   
    uint32_t backbufferIdx = 0;

    {   // DXGI Factory creation
        auto res = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create DXGI factory");
        if (FAILED(res)) {
            return -1;
        }
    }
    {   // Device Creation
        // @todo Adapter selection

        IDXGIAdapter* adapter = nullptr;
        for (uint32_t i = 0u; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC desc = {};
            auto res = adapter->GetDesc(&desc);
            MINI_ASSERT(SUCCEEDED(res), "Failed to get description for GPU adapter %lu", i);
        }

        auto minimumFeatureLevel = D3D_FEATURE_LEVEL_12_0;  // D3D11_FEATURE_LEVEL_12_0;
        auto res = D3D12CreateDevice(0, minimumFeatureLevel, IID_PPV_ARGS(&d3dDevice));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create D3D12 device");
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
        desc.NumDescriptors = 1024;     
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
        desc.NumDescriptors = 1024; 
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;
        auto res = d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvDescriptorHeap));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create DSV descriptor heap");
        if (FAILED(res)) {
            return -1;
        }
    }
    {   // SRV resource heap creation
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1024;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;
        auto res = d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvDescriptorHeap));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create SRV descriptor heap");
        if (FAILED(res)) {
            return -1;
        }
    }
    {   // create RTVs for swapchain backbuffers
        
        for (auto i = 0u; i < BACKBUFFER_COUNT; ++i) {

            auto res = swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i]));
            MINI_ASSERT(SUCCEEDED(res), "Failed to retrieve backbuffer at index #%i", i);
            backbuffers[i]->SetName(L"Swapchain Buffer");
            //d3dDevice->CreateRenderTargetView(backbuffers[i], nullptr, RTVs[i]);
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
        depthBuffer->SetName(L"Depth Buffer");
       /* DSV = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        d3dDevice->CreateDepthStencilView(depthBuffer, nullptr, DSV);*/
    }

    /*
        ***
    */

    ID3D12RootSignature* rootSig = nullptr;
    D3D12_SHADER_BYTECODE vertexShader;
    D3D12_SHADER_BYTECODE pixelShader;
    ID3D12PipelineState* pso = nullptr;

    {   // setup root signature 

        D3D12_ROOT_PARAMETER params[64];
           
        {   // Reserve space for one float4x4 matrix to be used as mvp transform and uploaded via root constant
            // @note    apparently, this isn't the most efficient way to deal with a transformation matrix but it'll do for now
            auto& param = params[0];
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            param.Constants.Num32BitValues = 32;    
            param.Constants.RegisterSpace = 0;
            param.Constants.ShaderRegister = 0;
        }
        {   // Vertex / index buffers for vertex pull;

            D3D12_DESCRIPTOR_RANGE ranges[2] = {};
            ranges[0].NumDescriptors = 2;
            ranges[0].RegisterSpace = 0;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

            auto& param = params[1];
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            param.DescriptorTable.NumDescriptorRanges = 1;
            param.DescriptorTable.pDescriptorRanges = ranges;
        }

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = 2;
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
    {   // load shaders from file
        std::ifstream fin("resource/shaders/base/Shader.shader", std::ios_base::binary);
        MINI_ASSERT(fin.is_open(), "Failed to open %s", "resources/shaders/base/Shader.shader");
        uint32_t pathLen = 0;
        fin >> pathLen;
        char* path = new char[pathLen + 1];
        memset(path, 0x0, pathLen + 1);
        fin.read(path, pathLen);
        {   // vertex shader
            uint32_t len = 0;
            fin >> len;
            vertexShader.BytecodeLength = static_cast<SIZE_T>(len);
            char* buf = new char[len];
            fin.read(buf, len);
            vertexShader.pShaderBytecode = buf;
        }
        {   // pixel shader
            uint32_t len = 0;
            fin >> len;
            pixelShader.BytecodeLength = static_cast<SIZE_T>(len);
            char* buf = new char[len];
            fin.read(buf, len);
            pixelShader.pShaderBytecode = buf;
        }
    }   
    {   // setup PSO

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = rootSig;
        desc.VS = vertexShader;
        desc.PS = pixelShader;
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
        desc.InputLayout.NumElements = 0; 
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
    mini::MeshLibrary meshLibrary;
    meshLibrary.Initialize(d3dDevice, 1024);

    char* meshDataBuf = nullptr;   // @note we share an allocation for vertices and indices
    mini::MeshData meshData;
    static_assert(sizeof(uint16_t) == sizeof(PAR_SHAPES_T));

    {   // create a procedural cube mesh, copy out positions and normals 
        auto cubeMesh = par_shapes_create_cube();
        par_shapes_translate(cubeMesh, -0.5f, -0.5f, -0.5f);
        par_shapes_unweld(cubeMesh, true);
        par_shapes_compute_normals(cubeMesh);
        
        meshData.vertexDataSize = sizeof(float) * 3 * (cubeMesh->npoints * 2);
        meshData.vertexStride = sizeof(float) * 6;
        
        meshData.indexDataSize = sizeof(uint16_t) * 3 * cubeMesh->ntriangles;

        meshData.vertexData = meshDataBuf = reinterpret_cast<char*>(malloc(meshData.vertexDataSize + meshData.indexDataSize));
        for (auto i = 0; i < cubeMesh->npoints; ++i) {

            auto writePtr = meshData.vertexData + meshData.vertexStride * i;
            memcpy(writePtr, cubeMesh->points + i * 3, sizeof(float) * 3);
            writePtr += sizeof(float) * 3;
            memcpy(writePtr, cubeMesh->normals + i * 3, sizeof(float) * 3);
        }
        meshData.indexData = meshDataBuf + meshData.vertexDataSize;
        memcpy(meshData.indexData, cubeMesh->triangles, meshData.indexDataSize);
    }

    auto cubeMesh = meshLibrary.AllocateWithData(meshData);
    free(meshDataBuf);  // @note we can free our mesh data here since we don't have a reason to keep it around any longer

    UINT64 frameFenceValue = 0;
    ID3D12Fence* frameFence = nullptr;
    HANDLE frameFenceEvent = NULL;
    {
        auto res = d3dDevice->CreateFence(frameFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frameFence));
        MINI_ASSERT(SUCCEEDED(res), "Failed to create frame fence");
        frameFenceEvent = CreateEvent(0, 0, FALSE, 0);
        MINI_ASSERT(frameFenceEvent != NULL, "Failed to create frame fence event");
    }
    
    /*
    */
    ImGui::CreateContext();

    ImGui_ImplDX12_Init(d3dDevice, 1, swapchainFormat, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    ImGui_ImplWin32_Init(hWnd);

    /*
    */
    mini::Timer timer;
    mini::rendergraph::RenderGraph rg;

    eastl::vector<mini::StaticMesh> meshes;
    for(auto i = -3; i < 6; ++i)
    {
        auto& mesh = meshes.emplace_back();
        mesh.resourceHandle = cubeMesh;
        mesh.transform.position = mini::math::vec3f_t(i * 1.5f, 0.0f, 0.0f);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE frameSRVOffsetCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE frameSRVOffsetGPU;
    const auto srvIncrement = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    do {
        auto const frameTime = timer.GetElapsedTime();
        timer.Reset();

        MSG msg = {};
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //
        frameSRVOffsetCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        frameSRVOffsetGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

        frameSRVOffsetCPU.ptr += srvIncrement;  // @note skip the first slot because we reserved that for imgui
        frameSRVOffsetGPU.ptr += srvIncrement;

        rg.StartFrame();

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            //
            ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
            auto const windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
            if (ImGui::Begin("#info", nullptr, windowFlags)) {
                ImGui::Text("Frame Time : %fms", frameTime * 1000.0);
            } ImGui::End();

            //
            //
            {
                using namespace mini;
                auto PrintPasses = [](eastl::vector<rendergraph::Pass> const& passes) {
                    for (auto const& pass : passes) {
                        ImGui::Text("%s\n", pass.name);
                    }
                };

                auto finalTarget = rg.ImportResource(backbuffers[backbufferIdx], rendergraph::Resource::RenderTarget, D3D12_RESOURCE_STATE_PRESENT);     // import swapchain backbuffer as a non-cullable resource
                auto depth = rg.ImportResource(depthBuffer, rendergraph::Resource::DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE); 
                
                D3D12_RESOURCE_DESC desc = {};
                desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                desc.DepthOrArraySize = 1;
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.MipLevels = 1;
                desc.SampleDesc.Count = 1;
                desc.Width = backbuffers[0]->GetDesc().Width;
                desc.Height = backbuffers[0]->GetDesc().Height;
                desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                auto tempTarget = rg.DeclareResource(desc, rendergraph::Resource::RenderTarget);

                rg
                .AddPass("Scene Pass", [&](rendergraph::RenderGraph * renderGraph, rendergraph::Pass& pass) {

                    pass.clear = true;
                    pass.clearColor[0] = 0.2f;
                    pass.clearColor[1] = 0.2f;
                    pass.clearColor[2] = 0.2f;
                    pass.clearColor[3] = 1.0f;

                    finalTarget = renderGraph->Write(pass, finalTarget);
                    depth = renderGraph->Write(pass, depth);

                    return [&](rendergraph::RenderGraph* renderGraph, rendergraph::Pass const& pass) {
                        
                        auto backbuffer = backbuffers[backbufferIdx];

                        D3D12_VIEWPORT viewport = {};
                        viewport.Width = (float)backbuffer->GetDesc().Width;
                        viewport.Height = (float)backbuffer->GetDesc().Height;
                        viewport.MinDepth = 0.0f;
                        viewport.MaxDepth = 1.0f;
                        D3D12_RECT scissorRect = {};
                        scissorRect.right = (LONG)backbuffer->GetDesc().Width;
                        scissorRect.bottom = (LONG)backbuffer->GetDesc().Height;

                        cmdList->SetDescriptorHeaps(1, &srvDescriptorHeap);

                        cmdList->SetGraphicsRootSignature(rootSig);
                        cmdList->RSSetViewports(1, &viewport);
                        cmdList->RSSetScissorRects(1, &scissorRect);

                        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                        const auto proj = mini::math::make_perspective_proj(mini::math::DegToRad(60.0f), viewport.Width / viewport.Height, 0.1f, 100.0f);
                        const auto view = mini::math::inverse(mini::math::make_lookat(mini::math::vec3f_t(0.0f, 1.5f, -8.0f), mini::math::vec3f_t(), mini::math::vec3f_t(0.0f, 1.0f, 0.0f)));


                        static float rot = 0.0f;
                        rot += 0.5f * static_cast<float>(frameTime);
                        for (auto const& mesh : meshes) {

                            auto meshResource = meshLibrary.Lookup(mesh.resourceHandle);
                            // @note allocate two SRVs from the frame ringbuffer heap, copy resource descriptors over then bind them for rendering

                            d3dDevice->CopyDescriptorsSimple(2, frameSRVOffsetCPU, { meshResource->vertexBufferView }, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                            cmdList->SetGraphicsRootDescriptorTable(1, frameSRVOffsetGPU);
                            frameSRVOffsetCPU.ptr += srvIncrement * 2;
                            frameSRVOffsetGPU.ptr += srvIncrement * 2;

                            const auto model = mini::math::make_translation(mesh.transform.position) * mini::math::quat_to_mat(mesh.transform.rotation);
                            const auto mvp = proj * view * model;

                            struct Constants
                            {
                                math::mat4x4f_t model;
                                math::mat4x4f_t mvp;
                            } constants { model, mvp };

                            cmdList->SetGraphicsRoot32BitConstants(0, 32, &constants, 0);

                            cmdList->DrawInstanced(meshResource->numIndices, 1, 0, 0);
                        }
                    };
                })
                .AddPass("UI Pass", [&](rendergraph::RenderGraph* renderGraph, rendergraph::Pass& pass) {
                    finalTarget = renderGraph->Write(pass, finalTarget);
                    pass.clear = false;
                    pass.clearColor[0] = 0.3f;
                    pass.clearColor[1] = 0.3f;
                    pass.clearColor[2] = 0.3f;
                    pass.clearColor[3] = 1.0f;
                    return [&](rendergraph::RenderGraph * renderGraph, rendergraph::Pass const& pass) {
                        cmdList->SetDescriptorHeaps(1, &srvDescriptorHeap);
                        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
                    };
                });
                
                PrintPasses(rg.GetFinalPasses());
            }
        }
        ImGui::EndFrame();
        ImGui::Render();

        //
        cmdAllocator->Reset();
        cmdList->Reset(cmdAllocator, pso);

        rg.Execute(d3dDevice, cmdList, rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        //
        {   // transition backbuffer to present state from render target state
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = backbuffers[backbufferIdx];
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

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();

    return 0;
}

