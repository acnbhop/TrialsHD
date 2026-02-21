
texture postProcessInput;
sampler postProcessInputSampler = 
sampler_state
{
	Texture = <postProcessInput>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
};

texture postProcessInput2;
sampler postProcessInput2Sampler = 
sampler_state
{
	Texture = <postProcessInput2>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	//MinFilter = POINT;
	//MagFilter = POINT;
	MipFilter = POINT;
};

texture postProcessInput3;
sampler postProcessInput3Sampler = 
sampler_state
{
	Texture = <postProcessInput3>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	//MinFilter = POINT;
	//MagFilter = POINT;
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
	// HDR bloom + screen buffer combine
    float4 sampledColor;
    float4 sampledColorParticles;
    float4 sampledColorACC;
    
    tex.y -= 24;					// NOTE: Uncomment this line to enable upper+lower 24 pixel menu areas
    //tex.y -= 48;	//720-672;		// NOTE: Uncomment this line to enable upper 48 pixel menu area
    //tex.y*=0.9333333;				// NOTE: Uncomment this line to enable streching
    
    float2 tex2 = tex/2;
    float2 tex4 = tex/4;
    
    asm 
    {
		/*
		tfetch2D sampledColorACC, tex,
                 postProcessInput2Sampler, OffsetX = -1.0, OffsetY = -1.0
                 
		tfetch2D sampledColor, tex,
                 postProcessInput2Sampler, OffsetX = 2.0, OffsetY = -1.0
        add sampledColorACC, sampledColorACC, sampledColor
                 
		tfetch2D sampledColor, tex,
                 postProcessInput2Sampler, OffsetX = -1.0, OffsetY = 2.0
        add sampledColorACC, sampledColorACC, sampledColor
                 
		tfetch2D sampledColor, tex,
                 postProcessInput2Sampler, OffsetX = 2.0, OffsetY = 2.0
        add sampledColorACC, sampledColorACC, sampledColor
        */

		tfetch2D sampledColorACC, tex4,
                 postProcessInput2Sampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D sampledColor, tex,
                 postProcessInputSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D sampledColorParticles, tex2,
                 postProcessInput3Sampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5
	};
	
	//float4 pixelColor = sampledColor * 2.0 + (sampledColorACC * sampledColorACC * sampledColorACC * 2);		// HDR color is squared to filter out low colors
	//float4 pixelColor = (sampledColorACC * sampledColorACC * sampledColorACC * 2);		// HDR color is squared to filter out low colors
	//return sampledColor * 2.0 + (sampledColorACC * sampledColorACC * 2);		// HDR color is squared to filter out low colors
	
	float4 pixelColor = sampledColor + sampledColorACC;
	//float4 pixelColor = sampledColor * 2.0 + sampledColorACC;
	//return pixelColor * (1 - sampledColorParticles.a) + float4(sampledColorParticles.rgb, 1);
	return pixelColor;
	
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


