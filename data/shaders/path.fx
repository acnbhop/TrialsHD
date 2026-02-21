
float4 lightColor;


struct VS_OUTPUT
{
    float4 pos					: POSITION;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position;
    return outStruct;
}


struct PS_OUTPUT
{
	float4 rtColorEmi	: COLOR0;
	float4 rtNormalSpec	: COLOR1;
};


PS_OUTPUT PS()
{
	PS_OUTPUT outStruct;
	//outStruct.rtColorEmi = float4(lightColor, 1);
    //outStruct.rtNormalSpec = float4(0.5f, 0.5f, 0.0f, 0.0f);
	outStruct.rtColorEmi = float4(lightColor);
	outStruct.rtNormalSpec = float4(0, 0, 0, 0);
    return outStruct;
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


