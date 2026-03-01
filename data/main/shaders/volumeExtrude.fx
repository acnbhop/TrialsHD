
float4x4 matrixWVP : WORLDVIEW;

float4 shadowDepthScale;

struct VS_OUT
{
	float4 pos : POSITION;
	float linearDepth : TEXCOORD0;
};


VS_OUT VS(float3 inPos		: POSITION)
{
	VS_OUT vsOut;
	vsOut.pos = mul(float4(inPos, 1), matrixWVP);
	vsOut.linearDepth = shadowDepthScale.x * vsOut.pos.z + shadowDepthScale.y;		// multiplied to [0,32] range and added depth bias
	return vsOut;
}


float4 PS(VS_OUT vsIn) : COLOR
{
	return float4(vsIn.linearDepth, 1, 0, 0);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


