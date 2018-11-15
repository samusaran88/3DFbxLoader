Texture2D map : register(t0);
SamplerState samp : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

struct VertexInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexOutput VertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput) 0;
    output.pos = mul(input.pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.uv = input.uv;
    return output;
}

float4 PixelMain(VertexOutput input) : SV_Target
{
    return map.Sample(samp, input.uv);
}
