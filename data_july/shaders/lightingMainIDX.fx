
float fogStart;
float fogLengthInv;
float4 fogColor;

float2 zConversion;

int lightAmount;
float4 lightColor[16];
float4 lightPositionRangeCS[16];
float4x4 lightMatrix[16];

int lightPointAmount;
float4 lightPointColor[16];
float4 lightPointPositionRangeCS[16];

float3 directionPSSM;
float4x4 matrixPSSM[3];
float3 colorPSSM;
float3 ambientMultiplier;


texture lightMapTexture0;
texture lightMapTexture1;
texture lightMapTexture2;
texture lightMapTexture3;
texture lightMapTexture4;
texture lightMapTexture5;
texture lightMapTexture6;
texture lightMapTexture7;
texture lightMapTexture8;
texture lightMapTexture9;
texture lightMapTexture10;
texture lightMapTexture11;
texture lightMapTexture12;
texture lightMapTexture13;
texture lightMapTexture14;
texture lightMapTexture15;
sampler lightMapTextureSampler[16] = 
{
	sampler_state
	{
		Texture = <lightMapTexture0>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture1>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture2>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture3>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture4>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture5>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture6>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture7>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture8>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture9>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture10>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture11>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture12>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture13>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture14>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture15>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
};


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
    float3 indices1				: TEXCOORD1;
    float3 indices2				: TEXCOORD2;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position;
    outStruct.tex2 = texcoords2;
    outStruct.indices1 = float3(0, 1, 2);
    outStruct.indices2 = float3(0, 1, 2);
    return outStruct;
}


float4 PS( float2 tex : VPOS, float3 tex2 : TEXCOORD0, float3 indices1 : TEXCOORD1, float3 indices2 : TEXCOORD2 ) : COLOR0
{
	int lightIndices[3];
	lightIndices[0] = indices1.x;
	lightIndices[1] = indices1.y;
	lightIndices[2] = indices1.z;
	int lightPointIndices[3];
	lightPointIndices[0] = indices2.x;
	lightPointIndices[1] = indices2.y;
	lightPointIndices[2] = indices2.z;

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
	
	// TODO: Convert texDepth to view space here
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
	
	float3 lightAccDiff = texColor.aaa;																		// Add emissive to diffuse (DOES NOT WORK ON MULTIPASS!!)
	lightAccDiff += texCUBE(lightIndexTextureSampler, texNormal2).rgb * ambientMultiplier;					// Add ambient (DOES NOT WORK ON MULTIPASS!!)
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
		
		lightMul += 0.2f;	// Hack: See a little bit though to give ambient light specular highlights
		
		float3 specHalfVector = normalize(directionPSSM - pixelDirection);

		float diffuse = saturate(dot(texNormal2, directionPSSM));
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
		lightMul *= colorPSSM;
		lightAccDiff += lightMul * diffuse;
		lightAccSpec += lightMul * specular;
	}


	// Spotlights
	for (int i=0; i<lightAmount; i++)
	{
		float4 lightPosRan = lightPositionRangeCS[lightIndices[i]];
		float4 posLMap = mul(float4(pixelPositionVS, 1), lightMatrix[lightIndices[i]]);
		
		if (posLMap.z > 0)
		{			
			// Sample light depth map
			float3 lightDirection = lightPosRan.xyz - pixelPositionVS;
			float range = length(lightDirection);
			float lrange = lightPosRan.w - range;
			lightDirection = lightDirection / range;
			float invRange = 1.0f / lightPosRan.w;
			float brightness = saturate(lrange * invRange);
			
			// Sample projectors and depth map simultaneously (optimized version)
			float3 texLmap = posLMap.xyz;
			texLmap.xy /= posLMap.w;
			texLmap.z *= invRange;
			//float3 lightMul = tex2D(lightMaskTextureSampler, texLmap.xy).xyz;
			float2 shadBuffer = tex2D(lightMapTextureSampler[lightIndices[i]], texLmap.xy).xy;

			brightness = shadBuffer.y * 32 * saturate(exp((5.0f + 80.0f * brightness) * ( shadBuffer.x - texLmap.z )));		// ESM with fake distance falloff soft shadows
					 
			float3 specHalfVector = normalize(lightDirection - pixelDirection);

			float diffuse = saturate(dot(texNormal2, lightDirection));
			float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
			float3 lightMul = lightColor[lightIndices[i]].rgb * brightness;
			lightAccDiff += lightMul * diffuse;
			lightAccSpec += lightMul * specular;
		}
	}
	
	
	// Pointlights
	for (int i2=0; i2<lightPointAmount; i2++)
	{
		float4 lightPosRan = lightPointPositionRangeCS[lightPointIndices[i2]];
		
		float3 lightDirection = lightPosRan.xyz - pixelPositionVS;
		float range = length(lightDirection);
		float lrange = lightPosRan.w - range;
		lightDirection = lightDirection / range;
		float brightness = saturate(lrange / lightPosRan.w);
		
		float3 specHalfVector = normalize(lightDirection - pixelDirection);

		float diffuse = saturate(dot(texNormal2, lightDirection));
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
		float3 lightMul = lightPointColor[lightPointIndices[i2]].rgb * brightness;
		lightAccDiff += lightMul * diffuse;
		lightAccSpec += lightMul * specular;
	}
		
	float3 lightAcc = lightAccDiff * texColor.rgb + lightAccSpec*3;
	
	//return float4(lightAcc*2, 0);
	
	// Fog
	float fogFactor = saturate((length(pixelPositionVS) - fogStart) * fogLengthInv);
	//return float4(lerp(lightAcc*2, fogColor.rgb, fogFactor) * hdrProperties.x, texNCombine2.r * materialColor.a);		
	return float4(lerp(lightAcc*2, fogColor.rgb, fogFactor), 1);
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


