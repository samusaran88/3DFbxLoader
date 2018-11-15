cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

struct VertexOutput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

VertexOutput VertexMain(float4 Pos : POSITION, float4 Color : COLOR) 
{
    VertexOutput output = (VertexOutput) 0;
    output.Pos = mul(Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Color = Color;
    return output;
}

float4 PixelMain(VertexOutput input) : SV_Target
{
    return input.Color;
}

//float4 VertexMain(float4 pos : POSITION) : SV_Position
//{
//    return pos;
//}
//
//float4 PixelMain(float4 pos : SV_position) : SV_Target
//{
//    return float4(1.0f, 1.0f, 0.0f, 1.0f);
//}