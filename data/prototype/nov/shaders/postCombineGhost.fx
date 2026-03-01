
float4 userInput;
float4 userInput3;

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
	MipFilter = POINT;
};

texture postProcessInput4;
sampler postProcessInput4Sampler = 
sampler_state
{
	Texture = <postProcessInput4>;
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
	// HDR bloom + screen buffer combine
    float4 sampledColor;
    float4 sampledColorParticles;
    float4 sampledColorACC;
    float4 sampledColorLastFrame;

    float2 tex01 = tex / float2(1280, 720);
	float2 expandTex = tex01 - normalize(tex01 - 0.5f) * 0.002f;
	//float percentage = length(tex01 - 0.5f) * 1.2f;
    
    tex.y -= 24;					// NOTE: Uncomment this line to enable upper+lower 24 pixel menu areas
    //tex.y -= 48;	//720-672;		// NOTE: Uncomment this line to enable upper 48 pixel menu area
    //tex.y*=0.9333333;				// NOTE: Uncomment this line to enable streching
    
    float2 tex2 = tex/2;
    float2 tex4 = tex/4;
        
    asm 
    {
		tfetch2D sampledColorACC, tex4,
                 postProcessInput2Sampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D sampledColor, tex,
                 postProcessInputSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D sampledColorParticles, tex2,
                 postProcessInput3Sampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D sampledColorLastFrame, expandTex,
                 postProcessInput4Sampler, OffsetX = 0.5, OffsetY = 0.5
	};
	
	float4 pixelColor = sampledColor + sampledColorACC*sampledColorACC;
	
	/*
	percentage = sqrt(percentage);
	percentage -= 0.05f;
	percentage *= percentage;
	percentage = saturate(percentage);
	*/
	float percentage = 1.0f - userInput.x;
	
	return userInput3.rrrr * ((1.0f - percentage) * (pixelColor * (1 - sampledColorParticles.a) + float4(sampledColorParticles.rgb, 1)) + percentage * sampledColorLastFrame);
	
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}














