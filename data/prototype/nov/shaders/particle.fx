
texture mapColor;


sampler mapColorSampler = 
sampler_state
{
    Texture = <mapColor>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
};



struct VS_OUTPUT
{
    float4 pos  : POSITION;
    float2 tex  : TEXCOORD0;
    float4 color : COLOR0;
};


struct PS_OUTPUT
{
	float4 color		: COLOR0;
};


VS_OUTPUT VS(
    float4 inPos    : POSITION,
    float3 inTex    : TEXCOORD0,
    float4 inColor  : COLOR0)
{
    VS_OUTPUT outStruct;
    outStruct.pos = float4(inPos.xyz, 1);
    outStruct.tex = inTex.xy;
    outStruct.color = inColor;
    return outStruct;
}


PS_OUTPUT PS(VS_OUTPUT psIn) : COLOR
{
    PS_OUTPUT outStruct;
	float4 texMapColor = tex2D(mapColorSampler, psIn.tex);
	outStruct.color = texMapColor * psIn.color * 4.0;
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


