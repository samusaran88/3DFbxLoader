Texture2D map[10] : register(t0);
SamplerState samp : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 LightDir;
}

cbuffer BoneBuffer : register(b1)
{
    float4 boneTransforms[255];
    matrix boneInvTransforms[255];
}

cbuffer PixelBuffer : register(b2)
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
    uint material : TEXCOORD4;
    uint ctrlPtNum : TEXCOORD2;
};

VertexOutput VertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput) 0;

    //float3 bonedPosition = float3(0, 0, 0);
    //float3 bonedNormal = float3(0, 0, 0);
    //for (int i = 0; i < 4; i++)
    //{
    //    int currentBoneIndex = input.bones[i];
    //    float4x4 boneTransform = boneTransforms[currentBoneIndex];
    //    float4x4 boneInverseTransform = boneInvTransforms[currentBoneIndex];
    //    float currentWeight = input.weight[i];
    //
    //    bonedPosition += currentWeight * mul(input.pos, boneTransform).xyz;
    //    bonedNormal += currentWeight * mul(input.normal, (float3x3) transpose(boneInverseTransform)).xyz;
    //}
    //
    //float4 bonedPosition4 = float4(bonedPosition, 1);
    //float4 bonedNormal4 = float4(bonedNormal, 0);

    output.pos = mul(input.pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.normal = mul(input.normal, (float3x3) World);
    output.binormal = input.binormal;
    output.tangent = input.tangent;
    output.material = input.material;
    output.uv = input.uv;
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
    }
    float3 ld = normalize(float3(0, 0, 1));

    float diffuse = dot((float3) -normalize(LightDir), input.normal);
    float3 ambient = float3(0.5f, 0.5f, 0.5f) * albedo.rgb;

    //albedo.rgb *= diffuse;
    //albedo.rgb += ambient;

    return albedo;
}