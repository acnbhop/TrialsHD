
float4x4 matrixWVP : WORLDVIEW;

texture mapColor;
sampler mapColorSampler = 
sampler_state
{
    Texture = <mapColor>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


struct VS_OUTPUT
{
    float4 pos			: POSITION;
    float2 tex			: TEXCOORD0;
};


VS_OUTPUT VS(
    float3 inPos		: POSITION,
    float2 inTex		: TEXCOORD0)
{
    VS_OUTPUT outStruct;
	outStruct.pos = mul(float4(inPos, 1), matrixWVP);
    outStruct.tex  = inTex;
    return outStruct;
}


float4 PS(VS_OUTPUT psIn) : COLOR
{
	return tex2D(mapColorSampler, psIn.tex);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}

