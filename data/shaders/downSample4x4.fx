
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
    float2 tex					: TEXCOORD0;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position;
    outStruct.tex = texcoords;
    return outStruct;
}


float4 PS( float2 tex : TEXCOORD0 ) : COLOR0
{
	// 4x4 downsample
    float4 sampledColor;
    float4 sampledColorACC;
    asm 
    {
		tfetch2D sampledColorACC, tex,
                 mapColorSampler, OffsetX = 1.0, OffsetY = 1.0
                 
		tfetch2D sampledColor, tex,
                 mapColorSampler, OffsetX = 3.0, OffsetY = 1.0
        add sampledColorACC, sampledColorACC, sampledColor
                 
		tfetch2D sampledColor, tex,
                 mapColorSampler, OffsetX = 1.0, OffsetY = 3.0
        add sampledColorACC, sampledColorACC, sampledColor
                 
		tfetch2D sampledColor, tex,
                 mapColorSampler, OffsetX = 3.0, OffsetY = 3.0
        add sampledColorACC, sampledColorACC, sampledColor
	};
	//return float4(sampledColorACC.xyz * 0.25, 1);
	return sampledColorACC * 0.25;
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


