
float fogStart;
float fogLengthInv;
float4 fogColor;

float2 zConversion;

float3 directionPSSM;
float4x4 matrixPSSM[3];
float3 colorPSSM;
float4 ambientMultiplier;


texture gbufferMotionVectors;
sampler gbufferMotionVectorsSampler = 
sampler_state
{
	Texture = <gbufferMotionVectors>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
};


textureCUBE lightIndexTexture;
samplerCUBE lightIndexTextureSampler = 
sampler_state
{
    Texture = <lightIndexTexture>;
    AddressU = Wrap;
    AddressV = Wrap;
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
    float3 tex2					: TEXCOORD0;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position;
    outStruct.tex2 = texcoords2;
    return outStruct;
}


float4 PS( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{

	// Deferred lighting shader
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
	float3 pixelPositionVS = tex2 * pixelDepthVS;
	
	// Directional sunlight
	float3 lightDirection = directionPSSM;
	float brightness = 1.0f;

	// Calculate normal vector from two channel input
	float3 texNormal2;
	texNormal2.xy = (texNormalSpec.xy * 2) - 1;
	texNormal2.z = -sqrt(1 - texNormal2.x*texNormal2.x - texNormal2.y*texNormal2.y);
	
	texNormalSpec.zw *= 2.0f;	// Stored specularity and glossiness in [0,2] range. Multiply back
	
	float3 pixelDirection = normalize(tex2);
	
	float3 lightAccDiff = texColor.aaa*2;																		// Add emissive to diffuse (DOES NOT WORK ON MULTIPASS!!)
	lightAccDiff += texCUBE(lightIndexTextureSampler, texNormal2).rgb * ambientMultiplier.rgb;					// Add ambient (DOES NOT WORK ON MULTIPASS!!)
	float3 lightAccSpec = float3(0, 0, 0);
	
	
	// Directional sun light (PSSM)
	{
		// Transform pixel to PSSM space (TODO: Optimize)
		float4 posPSSM[3];
		posPSSM[0] = mul(float4(pixelPositionVS, 1), matrixPSSM[0]);
		posPSSM[1] = mul(float4(pixelPositionVS, 1), matrixPSSM[1]);
		posPSSM[2] = mul(float4(pixelPositionVS, 1), matrixPSSM[2]);
	
		float3 texLmap1 = posPSSM[0].xyz / posPSSM[0].w;
		float pssmIndex = any(floor(texLmap1));
		float3 texLmap2 = posPSSM[1].xyz / posPSSM[1].w;
		pssmIndex += pssmIndex * any(floor(texLmap2));
		float4 posLMap = posPSSM[pssmIndex];
		//float3 texLmap = posLMap.xyz / posLMap.w;		// TODO: Replace with tex2DProj
		float3 texLmap = posLMap.xyz;
		texLmap.xy /= posLMap.w;
		texLmap.y *= 1.0f/3.0f;
		texLmap.y += (1.0f/3.0f)*pssmIndex;
		
		// Sample projectors and depth map simultaneously (optimized version)
		float2 shadBuffer = tex2D(gbufferMotionVectorsSampler, texLmap.xy).xy;

		float lZC = (0.5f + shadBuffer.x * 0.5f);
		float lZR = texLmap.z;
		float3 lightMul = shadBuffer.y * 32 * saturate(exp(120.0f * ( lZC*lZC - lZR*lZR )));		// ESM with squared depth range
		
		lightMul += ambientMultiplier.aaa;	// Hack: See a little bit though to give ambient light specular highlights
		
		float3 specHalfVector = normalize(directionPSSM - pixelDirection);

		float diffuse = saturate(dot(texNormal2, directionPSSM));
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
		lightMul *= colorPSSM;
		lightAccDiff += lightMul * diffuse;
		lightAccSpec += lightMul * specular;
	}


	float3 lightAcc = lightAccDiff * texColor.rgb + lightAccSpec*3;

	/*
	// NOTE: Fog moved to combine shader
	// Fog
	float fogFactor = saturate((length(pixelPositionVS) - fogStart) * fogLengthInv);
	//return float4(lerp(lightAcc*2, fogColor.rgb, fogFactor) * hdrProperties.x, texNCombine2.r * materialColor.a);		
	return float4(lerp(lightAcc*2, fogColor.rgb, fogFactor), 1);
	*/

	return float4(lightAcc*2, 1);

}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}

