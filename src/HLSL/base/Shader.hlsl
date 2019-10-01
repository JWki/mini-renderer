@vs_main { VSMain }
@ps_main { PSMain }
#line 3

struct VS_Out
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

struct MVP
{
    float4x4 mvp;
};
ConstantBuffer<MVP> constants : register(b0);

struct Vertex
{
    float3 position;
    float3 normal;
};

StructuredBuffer<Vertex> vertices : register(t0);
Buffer<uint> indices : register(t1); 

VS_Out VSMain(uint id: SV_VertexID)
{
    Vertex vertex = vertices[indices[id]];
    
    VS_Out output;
    output.color = vertex.normal * 0.5f + 0.5f;
    output.position = mul(constants.mvp, float4(vertex.position, 1.0f));
    return output;
}

float4 PSMain(VS_Out input) : SV_TARGET
{
    return float4(input.color, 1.0);
}