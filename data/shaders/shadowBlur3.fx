
//float3 lightColor;

texture mapColor;
sampler mapColorSampler = 
sampler_state
{
	Texture = <mapColor>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
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
	//float4 texCol = tex2D(mapColorSampler, tex);
	//return float4(texCol.xy*32, 0, 0.003921568627450980392156862745098);
	
    float4 texCol;
    float4 texCol2;
    asm 
    {
		tfetch2D texCol, tex,
                 mapColorSampler, OffsetX = -0.5, OffsetY = -0.5

		tfetch2D texCol2, tex,
                 mapColorSampler, OffsetX = 0.5, OffsetY = 0.5
	};	
	//return float4(texCol.xy*32, 0, 0.5);
	//return float4((texCol.xy+texCol2.xy)*16, 0, 0.03);
	return float4(texCol.xy*32, 0, 0.03);
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


