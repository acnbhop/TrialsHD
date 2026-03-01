
float4x4 matrixWVP : WORLDVIEW;
//float4x4 matrixWV : WORLDVIEW;
//float4x4 matrixP : PROJECTION;

float4 VS(float3 inPos		: POSITION) : POSITION
{

	return mul(float4(inPos, 1), matrixWVP);
    //float3 pos = mul(inPos, (float4x3)matrixWV);
    //return mul(float4(pos, 1), matrixP);
//    float4 pos = mul(float4(inPos, 1), matrixWV);
//    return mul(pos, matrixP);
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
        PixelShader  = null;
        //PixelShader  = compile ps_2_0 PS();
    }
}


