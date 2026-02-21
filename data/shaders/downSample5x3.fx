
float2 gbufferSize;

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

float4 multipliers = float4(0.5, 0.25, 0.266666, 0);

float4 PS( float2 tex : TEXCOORD0 ) : COLOR0
{
	// 5x3 downsample
    float4 sampledColor;
    float4 sampledColorACC;
    asm 
    {
		tfetch2D sampledColorACC.xxxx, tex,
                 mapColorSampler, OffsetX = 0.0, OffsetY = 0.0
                 
		tfetch2D sampledColor.xxxx, tex,
                 mapColorSampler, OffsetX = 2.0, OffsetY = 0.0
        add sampledColorACC, sampledColorACC, sampledColor

		tfetch2D sampledColor.xxxx, tex,
                 mapColorSampler, OffsetX = 3.5, OffsetY = 0.0
        mad sampledColorACC, sampledColor, multipliers.x, sampledColorACC
        
		tfetch2D sampledColor.xxxx, tex,
                 mapColorSampler, OffsetX = 0.0, OffsetY = 0.0
        mad sampledColorACC, sampledColor, multipliers.x, sampledColorACC
                 
		tfetch2D sampledColor.xxxx, tex,
                 mapColorSampler, OffsetX = 2.0, OffsetY = 0.0
        mad sampledColorACC, sampledColor, multipliers.x, sampledColorACC

		tfetch2D sampledColor.xxxx, tex,
                 mapColorSampler, OffsetX = 3.5, OffsetY = 0.0
        mad sampledColorACC, sampledColor, multipliers.y, sampledColorACC
	};
	return sampledColorACC * multipliers.z;
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


