
float4x4 matrixW[30] : WORLD;
float4x4 matrixVP : PROJECTION;

float4 shadowDepthScale;

struct VS_OUT
{
	float4 pos : POSITION;
	float linearDepth : TEXCOORD0;
};


VS_OUT VS(
    float3 inPos	: POSITION0,
    float4 inWeights	: BLENDWEIGHT,
    uint4  inIndices	: BLENDINDICES)
{
	VS_OUT vsOut;
	float4 posVS = mul(float4(inPos, 1), matrixW[inIndices.x]) * inWeights.x;
	posVS += mul(float4(inPos, 1), matrixW[inIndices.y]) * inWeights.y;
	posVS += mul(float4(inPos, 1), matrixW[inIndices.z]) * inWeights.z;
	posVS += mul(float4(inPos, 1), matrixW[inIndices.w]) * inWeights.w;
    vsOut.pos = mul(posVS, matrixVP);
	vsOut.linearDepth = shadowDepthScale.x * vsOut.pos.z + shadowDepthScale.y;		// multiplied to [0,32] range and added depth bias
	return vsOut;
}


float4 PS(float linearDepth : TEXCOORD0) : COLOR
{
	return float4(linearDepth, 1, 0, 0);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}














