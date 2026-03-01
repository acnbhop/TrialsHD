
float4x4 matrixP;
float4x4 matrixWV;
float4x4 matrixUser;		// light texture projection matrix

float2 zConversion;

float4 lightPositionRangeCS;
float4 lightColor;



texture lightMapTexture;
sampler lightMapTextureSampler = 
sampler_state
{
	Texture = <lightMapTexture>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
};


texture gbufferDepth;
sampler gbufferDepthSampler = 
sampler_state
{
	Texture = <gbufferDepth>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


texture gbufferColor;
sampler gbufferColorSampler = 
sampler_state
{
	Texture = <gbufferColor>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


texture gbufferNormal;
sampler gbufferNormalSampler = 
sampler_state
{
	Texture = <gbufferNormal>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};



struct VS_OUTPUT
{
    float4 pos					: POSITION;
    float3 directionVS			: TEXCOORD0;
};

VS_OUTPUT VS(float3 inPos : POSITION)
{
	VS_OUTPUT outStruct;
	outStruct.directionVS = mul(float4(inPos, 1), matrixWV);
	outStruct.pos = mul(float4(outStruct.directionVS, 1), matrixP);
	return outStruct;
}



float4 PS( float2 tex : VPOS, float3 directionVS : TEXCOORD0 ) : COLOR0
{
	// Deferred spot light shader
    float4 texDepth;					// Depth
    float4 texColor;					// color.rgb, emissive
    float4 texNormalSpec;				// normal.xy, specular, glossiness    
    asm 
    {
		tfetch2D texDepth, tex,
                 gbufferDepthSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D texColor, tex,
                 gbufferColorSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D texNormalSpec, tex,
                 gbufferNormalSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5
	};
	
	// Convert texDepth to view space
	float pixelDepthVS = zConversion.y / ((1.0f - texDepth.x) - zConversion.x);
	float3 pixelPositionVS = (directionVS.xyz / directionVS.z) * pixelDepthVS;
	
	// Calculate normal vector from two channel input
	float3 texNormal2;
	texNormal2.xy = (texNormalSpec.xy * 2) - 1;
	texNormal2.z = -sqrt(1 - texNormal2.x*texNormal2.x - texNormal2.y*texNormal2.y);
	float3 pixelDirection = normalize(directionVS);
	
	// Stored specularity and glossiness in [0,2] range. Multiply back
	texNormalSpec.zw *= 2.0f;	
	
	float4 finalColor = float4(0,0,0,1);
	
	// Calculate spotlight diffuse and specular intensity for pixel
	{	
		float4 posLMap = mul(float4(pixelPositionVS, 1), matrixUser);
		float3 texLmap = posLMap.xyz;
		posLMap.xyz /= posLMap.w;

		if (!any(floor(posLMap.xyz)))
		{
			float4 lightPosRan = lightPositionRangeCS;
		
			float3 lightDirection = lightPosRan.xyz - pixelPositionVS;
			float range = length(lightDirection);
			float lrange = lightPosRan.w - range;
			lightDirection = lightDirection / range;
			float invRange = 1.0f / lightPosRan.w;
			float brightness = saturate(lrange * invRange);
			
			// Sample projectors and depth map simultaneously (optimized version)
			texLmap.xy = posLMap.xy;
			texLmap.z *= invRange;
			float2 shadBuffer = tex2D(lightMapTextureSampler, texLmap.xy).xy;

			brightness *= shadBuffer.y * 32 * saturate(exp((5.0f + 80.0f * brightness) * ( shadBuffer.x - texLmap.z )));		// ESM with fake distance falloff soft shadows
					 
			float3 specHalfVector = normalize(lightDirection - pixelDirection);

			float diffuse = saturate(dot(texNormal2, lightDirection));
			float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
			float3 lightMul = lightColor.rgb * brightness;
			
			float3 lightAccDiff = lightMul * diffuse;
			float3 lightAccSpec = lightMul * specular;
			finalColor = float4(lightAccDiff * texColor.rgb + lightAccSpec*3, 1);
		}
	}

	return finalColor;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


