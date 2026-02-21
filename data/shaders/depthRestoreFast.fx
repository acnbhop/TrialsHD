
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


float4 PS( float2 ScreenPos : VPOS ) : COLOR0
{
    float ColumnIndex = ScreenPos.x / EDRAM_TILE_WIDTH;
    float HalfColumn = frac( ColumnIndex );
    float2 TexCoord = ScreenPos;
    if( HalfColumn >= 0.5 )
        TexCoord.x -= ( EDRAM_TILE_WIDTH / 2 );
    else
        TexCoord.x += ( EDRAM_TILE_WIDTH / 2 );

    float4 DepthData;
    asm 
    {
        tfetch2D DepthData, 
                 TexCoord, 
                 depth_alias_texture, 
                 UnnormalizedTextureCoords = true
    };
    return DepthData.zyxw;
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


