
float4x4 matrixWVP : WORLDVIEW;


float4 VS(float3 inPos		: POSITION) : POSITION
{
    return mul(float4(inPos, 1), matrixWVP);
}


/*
float4 PS() : COLOR
{
	return float4(0,0,0,0);
}
*/


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        //PixelShader  = compile ps_2_0 PS();
        PixelShader  = null;
    }
}


