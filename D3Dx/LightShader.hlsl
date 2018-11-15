Texture2D map : register(t0);
SamplerState samp : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 LightDir;
    //float4 lightColor;
    //float4 outputColor;
}

struct VertexInput
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD0;
};

VertexOutput VertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput) 0;
    output.pos = mul(input.pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3) World);
    return output;
}


struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD0;
};

//float4 PixelMain(PS_INPUT input) : SV_Target
//{
//    return map.Sample(samp, input.Tex);
//}

float4 PixelMain(VertexOutput input) : SV_Target
{
    float4 albedo = map.Sample(samp, input.uv);
    float3 ld = normalize(float3(0, 0, 1));

    float diffuse = dot((float3) -normalize(LightDir), input.normal);
    float3 ambient = float3(0.5f, 0.5f, 0.5f) * albedo.rgb;

    albedo.rgb *= diffuse;
    albedo.rgb += ambient;

    return map.Sample(samp, input.uv);
}


/*
struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float Pad;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Position;
    float Range;
    float3 Attenuation;
    float Pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Position;
    float Range;
    float3 Direction;
    float Spot;
    float3 Attenuation;
    float Pad;
};

cbuffer cbPerFrame
{
    DirectionalLight    dLight;
    PointLight          pLight;
    SpotLight           sLight;
    float3              EyePos;
};
*/