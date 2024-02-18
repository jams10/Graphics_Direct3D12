
// 쉐이더 입력.
struct VSInput
{
    float3 Position : POSITION; // 정점 위치.
    float4 Color : COLOR;       // 정점 색상.
};

// 쉐이더 출력.
struct VSOutput
{
    float4 Position : SV_POSITION; // 정점 위치.
    float4 Color : COLOR;          // 정점 색상.
};


cbuffer Transform : register(b0)
{
    float4x4 World : packoffset(c0); // 월드 변환 행렬.
    float4x4 View : packoffset(c4);  // 뷰 변환 행렬.
    float4x4 Proj : packoffset(c8);  // 투영 변환 행렬.
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

    float4 localPos = float4(input.Position, 1.0f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    output.Position = projPos;
    output.Color = input.Color;

    return output;
}