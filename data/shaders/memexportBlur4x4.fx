

float4 exportAddress;
static float4 const01= {0, 1, 0, 0};


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


float4 PS( float2 tex : TEXCOORD0, float2 screenPos : VPOS  ) : COLOR0
{
	// 4x4 fast blur filter
	// TODO: Implement proper gaussian blur
    float4 sampledColor;
    //float4 sampledColor2;
    //float4 sampledColor3;
    //float4 sampledColor4;
    asm 
    {
		tfetch2D sampledColor, tex,
                 mapColorSampler, OffsetX = 0.5, OffsetY = 0.5

		/*    
		tfetch2D sampledColor, tex,
                 mapColorSampler, OffsetX = -1.0, OffsetY = -1.0
                 
		tfetch2D sampledColor2, tex,
                 mapColorSampler, OffsetX = 1.0, OffsetY = -1.0
                 
		tfetch2D sampledColor3, tex,
                 mapColorSampler, OffsetX = -1.0, OffsetY = 1.0
                 
		tfetch2D sampledColor4, tex,
                 mapColorSampler, OffsetX = 1.0, OffsetY = 1.0
                 */
	};

	int writeIndex = screenPos.y * 1280 + screenPos.x;
	asm 
	{
		alloc export=1
		mad eA, writeIndex, const01, exportAddress
		mov eM0, sampledColor;
	};
	
	sampledColor.r = 0.5;
	return sampledColor;		// MUST BE DONE in export shaders too... color write is masked out
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


