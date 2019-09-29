#pragma once

#include <stdint.h>
#include <eastl/vector.h>
#include <EASTL/fixed_function.h>


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>

namespace mini
{
    namespace rendergraph
    {
        struct Resource
        {
            int32_t             id = -1;
            bool                isRootResource = false;
            D3D12_RESOURCE_DESC desc = {};

            enum Type {
                RenderTarget,
                DepthTarget
            } type = RenderTarget;

            ID3D12Resource* d3dResource = nullptr;
            /**/
            D3D12_RESOURCE_STATES currentState;

            struct {
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
                D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
            };

            bool operator == (Resource const& other) const { return other.id == id; }

            bool Realize(ID3D12Device* device);

            void Cleanup()
            {
                if (d3dResource != nullptr && !isRootResource) {
                    d3dResource->Release();
                }
            }
        };

        class RenderGraph;
        struct Pass;

        using PassExecuteFunc = eastl::fixed_function<1024, void(RenderGraph*, Pass const&)>;
        using PassInitFunc = eastl::fixed_function<1024, PassExecuteFunc(RenderGraph*, Pass&)>;


        struct Pass
        {
            char const* name = "Generic Pass";
            int32_t id = -1;

            eastl::vector<Resource> reads;
            eastl::vector<Resource> writes;
            bool clear = false;
            float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            Pass() = default;
            Pass(char const* n) : name(n) {}

            bool operator == (Pass const& other) const { return other.id == id; }

            PassExecuteFunc execute;
        };

        class RenderGraph
        {
            int32_t             m_nextPassId = 0;
            int32_t             m_nextResId = 0;
            eastl::vector<Pass> m_passes;

            eastl::vector<Pass> GetPassDependencies(Pass const& pass);
            void InsertRecursively(eastl::vector<Pass>& sortedPasses, Pass const& pass);
            eastl::vector<Pass> SortPasses();
        public:

            RenderGraph& AddPass(char const* name, PassInitFunc&& init) {
                Pass pass(name);
                pass.id = m_nextPassId++;
                pass.execute = init(this, pass);
                m_passes.push_back(pass);
                return *this;
            }

            Resource DeclareResource(D3D12_RESOURCE_DESC const& desc, Resource::Type type) { return { m_nextResId++, false, desc, type }; }
            Resource ImportResource(ID3D12Resource* resource, Resource::Type type, D3D12_RESOURCE_STATES state) { return { m_nextResId++, true, resource->GetDesc(), type, resource, state }; }
            Resource IncrementResourceVersion(Resource res) { auto ret = res; ret.id = m_nextResId++; return ret; }

            Resource Read(Pass& pass, Resource const& res) { pass.reads.push_back(res); return IncrementResourceVersion(res); }
            Resource Write(Pass& pass, Resource const& res) { pass.writes.push_back(IncrementResourceVersion(res)); return pass.writes.back(); }

            void StartFrame();
            void Execute(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart, D3D12_CPU_DESCRIPTOR_HANDLE dsvHeapStart);
            eastl::vector<Pass> const& GetFinalPasses();
            
        };
    }
}
