
float2 zConversion;
float4 dofParameters;	// focalPlaneDepth, focalPlaneWidth, focusMultiplierNear, focusMultiplierFar

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


texture postProcessInput4;
sampler postProcessInput4Sampler = 
sampler_state
{
	Texture = <postProcessInput4>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
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
    float4 sampledColorDepth;
    
    tex *= userInput.xy;
    tex += userInput.zw;
    
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

		tfetch2D sampledColorDepth, tex,
                 postProcessInput4Sampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5
	};
	
	/*
	float4 pixelColor = sampledColor + sampledColorACC;
	return userInput3.rrrr * (pixelColor * (1 - sampledColorParticles.a) + float4(sampledColorParticles.rgb, 1));
	*/
	
	// Convert texDepth to view space
	float pixelDepthVS = zConversion.y / ((1.0f - sampledColorDepth.x) - zConversion.x);

	float focusBlur;
	float focalDistance = pixelDepthVS - dofParameters.x;
	if (focalDistance <= 0)
	{
		focusBlur = 1.0f - (pixelDepthVS / dofParameters.x);
		focusBlur *= dofParameters.z;
	}
	else
	{
		focusBlur = 1.0f - (dofParameters.x / pixelDepthVS);
		focusBlur *= dofParameters.w;
	}
	
	float4 pixelColor = lerp(sampledColor, sampledColorACC * 2.0f, saturate(focusBlur));
	//float4 pixelColor = lerp(sampledColorACC, sampledColor, focusBlur);
	pixelColor += sampledColorACC*sampledColorACC*0.8f;

	//float4 pixelColor = sampledColor * 2.0 + sampledColorACC;
	return userInput3.rrrr * (pixelColor * (1 - sampledColorParticles.a) + float4(sampledColorParticles.rgb, 1));		
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


