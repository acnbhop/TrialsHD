
float EDRAM_TILE_WIDTH = 80;


texture mapColor;
sampler mapColorSampler = 
sampler_state
{
	Texture = <mapColor>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


float4 VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 ) : POSITION
{
    return position;
}


void PS( float2 ScreenPos : VPOS,
		 out float4 oColor : COLOR,
         out float oDepth : DEPTH  )
{
    float2 TexCoord = ScreenPos * 2;
    float4 DepthData;
    asm 
    {
        tfetch2D DepthData, 
                 TexCoord, 
                 mapColorSampler, 
                 UnnormalizedTextureCoords = true
    };
    
    oDepth = DepthData.x;
    oColor = 0;
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


