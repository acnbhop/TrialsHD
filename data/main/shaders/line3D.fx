float4x4 matrixWVP : WORLDVIEWPROJECTION;

struct VS_OUTPUT
{
    float4 pos			: POSITION;
    float4 color			: COLOR0;
};

VS_OUTPUT VS(
    float3 inPos			: POSITION,
    float4 inColor		: COLOR0)
{
	VS_OUTPUT outStruct;
	outStruct.pos=mul(float4(inPos,1), matrixWVP);		
	outStruct.color=inColor;
	return outStruct;
}

/*
float4 PS(VS_OUTPUT psIn) : COLOR0
{
	return psIn.color;
}
*/


struct PS_OUTPUT
{
	float4 rtColorEmi	: COLOR0;
	float4 rtNormalSpec	: COLOR1;
};


PS_OUTPUT PS(VS_OUTPUT psIn)
{
	PS_OUTPUT outStruct;
	outStruct.rtColorEmi = float4(psIn.color.xyz, 1);
    outStruct.rtNormalSpec = float4(0.5f, 0.5f, 0.0f, 0.0f);
    return outStruct;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


