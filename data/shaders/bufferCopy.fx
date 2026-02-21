
texture2D postProcessInput;

sampler2D mapColorSampler = 
sampler_state
{
    Texture = <postProcessInput>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


float4 PS( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	return tex2D(mapColorSampler, tex);
}


technique Basic
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PS();
    }
}


