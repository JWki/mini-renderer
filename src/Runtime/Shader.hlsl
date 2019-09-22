struct vs_in
{
    float2 P : POSITION;
    float3 Col : COLOR;
};

struct vs_out
{
    float4 SV_P : SV_POSITION;
    float3 Col : COLOR;
};

vs_out VSMain(vs_in In)
{
    vs_out Out;

    Out.SV_P = float4(In.P, 0.0, 1.0);
    Out.Col = In.Col;

    return Out;
}

float4 PSMain(vs_out In) : SV_TARGET
{
    return float4(sqrt(In.Col), 1.0);
}