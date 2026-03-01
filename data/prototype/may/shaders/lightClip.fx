
float4x4 matrixWVP : WORLDVIEW;

float4 VS(float3 inPos		: POSITION) : POSITION
{
	return mul(float4(inPos, 1), matrixWVP);
}

technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = null;
    }
}


