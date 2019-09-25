#include "rendergraph.h"

#include <Runtime/common.h>
#include <EASTL/fixed_vector.h>

//
//
//

bool mini::rendergraph::Resource::Realize(ID3D12Device* device)
{
    if (d3dResource != nullptr) { return true; }
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    currentState = type == RenderTarget ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_DEPTH_WRITE;
    auto res = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, &desc, currentState, nullptr, IID_PPV_ARGS(&d3dResource));
    MINI_ASSERT(SUCCEEDED(res), "Failed to realize render graph resource");
    return SUCCEEDED(res);
}


//
//
//

eastl::vector<mini::rendergraph::Pass> mini::rendergraph::RenderGraph::GetPassDependencies(Pass const& pass)
{
    eastl::vector<Pass> dependencies;
    dependencies.data();
    for (auto const& p : m_passes) {
        bool dependsOn = false;
        for (auto const& read : pass.reads) {
            for (auto const& write : p.writes) {
                if (read == write) { dependsOn = true; break; }
            }
            if (dependsOn) { break; }
        }
        if (dependsOn) {
            dependencies.push_back(p);
        }
    }
    return dependencies;
}

void mini::rendergraph::RenderGraph::InsertRecursively(eastl::vector<Pass>& sortedPasses, Pass const& pass)
{
    for (const auto& p : sortedPasses) {
        if (pass == p) { return; }
    }
    const auto dependencies = GetPassDependencies(pass);
    for (const auto& dependency : dependencies) {
        InsertRecursively(sortedPasses, dependency);
    }
    sortedPasses.push_back(pass);
}

eastl::vector<mini::rendergraph::Pass> mini::rendergraph::RenderGraph::SortPasses()
{
    eastl::vector<Pass> sortedPasses;
    sortedPasses.reserve(m_passes.size());
    for (auto const& pass : m_passes) {
        InsertRecursively(sortedPasses, pass);
    }
    return sortedPasses;
}

eastl::vector<mini::rendergraph::Pass> const& mini::rendergraph::RenderGraph::GetFinalPasses()
{
    m_passes = SortPasses();
    return m_passes;
}

void mini::rendergraph::RenderGraph::Execute(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart, D3D12_CPU_DESCRIPTOR_HANDLE dsvHeapStart)
{
    GetFinalPasses();

    auto nextRtv = rtvHeapStart;
    auto nextDsv = dsvHeapStart;

    auto rtvIncrSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    auto dsvIncrSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    for (auto& pass : m_passes) {
        for (auto& write : pass.writes) {
            auto res = write.Realize(device);
            MINI_ASSERT(res, "Failed to realize render pass write resource");
        }
    }

    for (auto& pass : m_passes) {

        eastl::fixed_vector<D3D12_CPU_DESCRIPTOR_HANDLE, 12> rtvs;
        eastl::fixed_vector<D3D12_RESOURCE_BARRIER, 12> barriers;
        D3D12_CPU_DESCRIPTOR_HANDLE const* dsv = nullptr;

        for (auto& write : pass.writes) {
            if (write.type == Resource::DepthTarget) {

                device->CreateDepthStencilView(write.d3dResource, nullptr, nextDsv);
                write.cpuDescriptorHandle.ptr = nextDsv.ptr;
                nextDsv.ptr += dsvIncrSize;
                dsv = &write.cpuDescriptorHandle;

                if (write.currentState != D3D12_RESOURCE_STATE_DEPTH_WRITE) {
                    D3D12_RESOURCE_BARRIER barrier = {};
                    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                    barrier.Transition.pResource = write.d3dResource;
                    barrier.Transition.StateBefore = write.currentState;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                    barriers.push_back(barrier);

                    write.currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                }
            }
            else {
                device->CreateRenderTargetView(write.d3dResource, nullptr, nextRtv);
                write.cpuDescriptorHandle.ptr = nextRtv.ptr;
                nextRtv.ptr += rtvIncrSize;
                rtvs.push_back(write.cpuDescriptorHandle);

                if (write.currentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
                    D3D12_RESOURCE_BARRIER barrier = {};
                    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                    barrier.Transition.pResource = write.d3dResource;
                    barrier.Transition.StateBefore = write.currentState;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    barriers.push_back(barrier);

                    write.currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
                }
            }
        }

        if (!barriers.empty()) {    // handle resource transition with a single call to ResourceBarrier
            cmdList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
        }

        if (pass.clear) {
            for (auto const& rtv : rtvs) {
                cmdList->ClearRenderTargetView(rtv, pass.clearColor, 0, nullptr);
            }
            if (dsv != nullptr) {
                cmdList->ClearDepthStencilView(*dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
            }
        }
        cmdList->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), FALSE, dsv);
        pass.execute(this, pass);
    }

    m_passes.clear();
}