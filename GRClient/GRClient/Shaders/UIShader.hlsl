#include "Common.hlsl"



struct VertexOut
{
    float4 Pos : SV_POSITION;
    //float4 Color : COLOR;
};

struct VertexIn
{
    float Pos : SV_Position;
};

VertexOut VS(uint VertexID : SV_VertexID, VertexIn vin)
{
    VertexOut vout;

    

    float4 posW = mul(float4(vin.Pos, 1.0f), gWorld); //기존 좌표계를 월드좌표계로?
    
    posW = mul(posW, 2 / vi);
    posW = posW - 1;
   
    vout.Pos = mul(posW, gViewProj); // 클립스페이스로

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}

