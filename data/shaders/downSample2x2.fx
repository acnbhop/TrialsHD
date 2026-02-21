
texture mapColor;
sampler mapColorSampler = 
sampler_state
{
	Texture = <mapColor>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    //float2 tex					: TEXCOORD0;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position;
    //outStruct.tex = texcoords;
    return outStruct;
}


float4 PS( float2 tex : VPOS ) : COLOR0
{
	// 2x2 downsample
	tex *= 2;
    float4 sampledColorACC;
    asm 
    {
		tfetch2D sampledColorACC, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = 1.0, OffsetY = 1.0
	};
	return float4(sampledColorACC.xyz, 1);
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


