
texture2D mapColor;
texture2D gbufferDepth;
float2 gbufferSize;

sampler2D mapColorSampler = 
sampler_state
{
    Texture = <mapColor>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};

sampler2D gbufferDepthSampler = 
sampler_state
{
    Texture = <gbufferDepth>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    float4 color				: TEXCOORD0;
    float3 tex					: TEXCOORD1;
};


VS_OUTPUT VS( float3 position : POSITION0, float4 color : COLOR0_center, float2 texcoords : TEXCOORD0 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = float4(position / float3(gbufferSize.x/2, -gbufferSize.y/2, 1) + float3(-1,1,0), 1);
    outStruct.color  = color;
    outStruct.tex  = float3(texcoords, outStruct.pos.z);
    return outStruct;
}


float4 PS( float4 color : TEXCOORD0, float3 tex : TEXCOORD1, float2 pixelPos : VPOS ) : COLOR0
{
	float4 texDepth;
    asm 
    {
		tfetch2D texDepth, pixelPos, gbufferDepthSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5
	};
	
	float4 texCol = tex2D(mapColorSampler, tex.xy);
	
	if ((1.0f-texDepth.x) < tex.z) texCol.a *= 0.25f;
	
	return texCol * color;
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


