Texture2D map[10] : register(t0);
SamplerState samp : register(s0);

struct TransformMatrix
{
    float4x4 transform;
};
StructuredBuffer<TransformMatrix> TM : register(t10);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 LightDir;
}

cbuffer PixelBuffer : register(b1)
{
    float4 ambient;
    float4 diffuse;
    float3 specular;
    float power;
    float4 emmisive;
}

struct VertexInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 weight : BLENDWEIGHT;
    uint4 bones : BLENDBONE;
    uint material : TEXCOORD1;
    uint ctrlPtNum : TEXCOORD2;
};

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint material : TEXCOORD1;
    uint ctrlPtNum : TEXCOORD2;
};

VertexOutput VertexMain(VertexInput input)
{
    float4x4 Trans = TM[input.ctrlPtNum].transform;
    float weight = Trans._41;
    Trans._41 = 0.0;
    float4 srcVertex;
    float4 dstVertex;
    float4 translation;
    if (weight != 0.0)
    {
        translation.x = Trans._14;
        translation.y = Trans._24;
        translation.z = Trans._34;
        translation.w = 0;
        srcVertex = input.pos * (1.0 - weight);
        //dstVertex = input.pos + translation;
        dstVertex = mul(input.pos, transpose(Trans));
        dstVertex += srcVertex;
    }
    else
    {
        //dstVertex = input.pos;
    }

    VertexOutput output = (VertexOutput) 0;

    output.pos = mul(dstVertex, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.normal = mul(input.normal, (float3x3) World);
    output.binormal = input.binormal;
    output.tangent = input.tangent;
    output.material = input.material;
    output.uv = input.uv;
    output.ctrlPtNum = input.ctrlPtNum;
    return output;

    //VertexOutput output = (VertexOutput) 0;
    //output.pos = mul(input.pos, World);
    //output.pos = mul(output.pos, View);
    //output.pos = mul(output.pos, Projection);
    //output.uv = input.uv;
    //output.normal = mul(input.normal, (float3x3) World);
    //output.binormal = input.binormal;
    //output.tangent = input.tangent;
    //output.material = input.material;
    //return output;
}


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

float4 PixelMain(VertexOutput input) : SV_Target
{
    float4 albedo;
    switch (input.material)
    {
        case (0):
            albedo = map[0].Sample(samp, input.uv);
            break;
        case (1):
            albedo = map[1].Sample(samp, input.uv);
            break;
        case (2):
            albedo = map[2].Sample(samp, input.uv);
            break;
        case (3):
            albedo = map[3].Sample(samp, input.uv);
            break;
        case (4):
            albedo = map[4].Sample(samp, input.uv);
            break;
        case (5):
            albedo = map[5].Sample(samp, input.uv);
            break;
        case (6):
            albedo = map[6].Sample(samp, input.uv);
            break;
        case (7):
            albedo = map[7].Sample(samp, input.uv);
            break;
        case (8):
            albedo = map[8].Sample(samp, input.uv);
            break;
        case (9):
            albedo = map[9].Sample(samp, input.uv);
            break;
    }
    float3 ld = normalize(float3(0, 0, 1));

    float diffuse = dot((float3) -normalize(LightDir), input.normal);
    float3 ambient = float3(0.5f, 0.5f, 0.5f) * albedo.rgb;

    //albedo.rgb *= diffuse;
    //albedo.rgb += ambient;

    return albedo;
}