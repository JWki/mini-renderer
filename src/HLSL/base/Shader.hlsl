@vs_main { VSMain }
@ps_main { PSMain }
#line 3

struct VS_Out
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
};

struct Matrices
{
    float4x4 object;
    float4x4 mvp;
};

ConstantBuffer<Matrices> constants : register(b0, space0);

struct Vertex
{
    float3 position;
    float3 normal;
};

StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<uint> indices : register(t1); 


VS_Out VSMain(uint id: SV_VertexID)
{
    Vertex vertex = vertices[indices[id]];
    
    VS_Out output;
    output.normal = mul(constants.object, vertex.normal);
    output.position = mul(constants.mvp, float4(vertex.position, 1.0f));
    return output;
}

float4 PSMain(VS_Out input) : SV_TARGET
{
    const float3 L = normalize(-float3(-1, -1, 1));

    float lambert = dot(L, input.normal);

    return float4(lambert, lambert, lambert, 1.0);
}