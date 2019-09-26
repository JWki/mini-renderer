struct VS_In
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VS_Out
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

cbuffer Constants : register(b0)
{
    float4x4 mvp;
};


VS_Out VSMain(VS_In input)
{
    VS_Out output;

    output.color = input.normal * 0.5f + 0.5f;
    output.position = mul(mvp, float4(input.position, 1.0f));

    return output;
}

float4 PSMain(VS_Out input) : SV_TARGET
{
    return float4(input.color, 1.0);
}