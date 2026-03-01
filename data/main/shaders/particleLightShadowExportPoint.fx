
float4x4 matrixVP : VIEWPROJECTION;
float4x4 matrixV : VIEW;

bool calculateLighting;

int lightAmount;
float4 lightColor[6];
float4 lightPositionRangeCS[6];
float4 lightDirectionCS[6];
//float4x4 lightMatrix[6];

int lightPointAmount;
float4 lightPointColor[6];
float4 lightPointPositionRangeCS[6];

float3 directionPSSM;
float4x4 matrixPSSM[3];
float3 colorPSSM;
float4 ambientMultiplier;

float4 exportAddress;
static float4 const01= {0, 1, 0, 0};


texture gbufferMotionVectors;
sampler gbufferMotionVectorsSampler = 
sampler_state
{
	Texture = <gbufferMotionVectors>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


/*
texture lightMapTexture0;
texture lightMapTexture1;
texture lightMapTexture2;
sampler lightMapTextureSampler[3] = 
{
	sampler_state
	{
		Texture = <lightMapTexture0>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = POINT;
		MagFilter = POINT;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture1>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = POINT;
		MagFilter = POINT;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture2>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = POINT;
		MagFilter = POINT;
		MipFilter = POINT;
	}
};
*/


void VS(int index : INDEX)
{
	float4 positionXYZRadius;
	float4 colorXYZW;
	float4 velocityXYZFrame;
	asm
	{
		vfetch positionXYZRadius, index, position0
		vfetch velocityXYZFrame, index, position1
		vfetch colorXYZW, index, position2
	};
	

	// ---- Transform and project particle and radius to post projection screen space ----
	
	float3 positionWS = positionXYZRadius.xyz;

	float4 positionVS = mul(float4(positionWS, 1), matrixV);

	// Transform position to post projection screen space
	float4 posScreenXYZRad = mul(float4(positionWS, 1), matrixVP);
	posScreenXYZRad.xyz /= posScreenXYZRad.w;

	// Store post projection radius for particle points (in w-channel)
	posScreenXYZRad.w = positionXYZRadius.w / posScreenXYZRad.w;
	posScreenXYZRad.z = 1.0 - posScreenXYZRad.z;						// 16 bit float z value precision improvement

	float4 expandDirection = float4(0, posScreenXYZRad.w, 0, posScreenXYZRad.w);

	// ---- Calculate lighting for particle (world space) center point ----

	float4 lightAccDiff = float4(1, 1, 1, 1);
	float4 lightAvgDirectionXYZFrame = float4(0, 0, 0, velocityXYZFrame.w);		// frame in w channel

	if (calculateLighting)
	{
		//lightAccDiff = float4(0.3, 0.3, 0.4, 0.3);		// Fake ambient
		lightAccDiff = float4(ambientMultiplier.rgb*3, 0.3f);
		float3 lightAvgDirection = float3(0.1, 0.1, 0.1); 
	
		// Directional sun light (PSSM)
		{
			// Transform particle to PSSM space
			float4 posPSSM[3];
			posPSSM[0] = mul(float4(positionWS, 1), matrixPSSM[0]);
			posPSSM[1] = mul(float4(positionWS, 1), matrixPSSM[1]);
			posPSSM[2] = mul(float4(positionWS, 1), matrixPSSM[2]);
		
			// Calculate PSSM texture coordinate
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

			// Sample light depth map
			float lightZ = tex2Dlod(gbufferMotionVectorsSampler, float4(texLmap.xy, 0, 1)).x;	// Only depth for semi transparent surfaces (no projectors!)	
			//float lightMul = (lightZ > texLmap.z) * 0.4;
			float lightMul = saturate(exp(120.0f * ( (0.5f + lightZ * 0.5f) - texLmap.z )));		// ESM
					
			lightMul += 0.3f;
			lightAccDiff += float4(colorPSSM, 1) * lightMul;
			lightAvgDirection += directionPSSM * lightMul;
		}
		
		
		// Pointlights
		for (int LINDEX=0; LINDEX<lightPointAmount; LINDEX++)
		{
			float4 lightPosRan = lightPointPositionRangeCS[LINDEX];
			
			float3 lightDirection = lightPosRan.xyz - positionVS;
			float rangeSQ = dot(lightDirection, lightDirection);
			if (rangeSQ < lightPosRan.w*lightPosRan.w)
			{
				float range = sqrt(rangeSQ);
				float lrange = lightPosRan.w - range;
			
				lightDirection = lightDirection / range;
				float brightness = saturate(lrange / lightPosRan.w);

				float3 lightMul = lightPointColor[LINDEX].rgb * brightness;
				lightAccDiff += float4(lightMul, 1);
				lightAvgDirection += lightDirection * brightness;				
			}
		}
		
		// Spotlights (shadowmapped with projectors)
		/*
		for (int LINDEX2=0; LINDEX2<lightAmount; LINDEX2++)		
		{
			float4 posLMap = mul(float4(positionVS, 1), lightMatrix[LINDEX2]);
					
			//if (posLMap.z > 0)
			{			
				float3 texLmap = posLMap.xyz;
				posLMap.xyz /= posLMap.w;

				if (!any(floor(posLMap.xyz)))
				{
					float4 lightPosRan = lightPositionRangeCS[LINDEX2];
				
					float3 lightDirection = lightPosRan.xyz - positionVS;
					float range = length(lightDirection);
					float lrange = lightPosRan.w - range;
					lightDirection = lightDirection / range;
					float invRange = 1.0f / lightPosRan.w;
					float brightness = saturate(lrange * invRange);
					
					// Sample projectors and depth map simultaneously (optimized version)
					texLmap.xy = posLMap.xy;
					texLmap.z *= invRange;
					//float3 lightMul = tex2D(lightMaskTextureSampler, texLmap.xy).xyz;
					float lightZ = tex2D(lightMapTextureSampler[LINDEX2], texLmap.xy).xy;

					brightness *= saturate(exp((5.0f + 80.0f * brightness) * ( lightZ - texLmap.z )));		// ESM with fake distance falloff soft shadows
							 
					float3 lightMul = lightColor[LINDEX2].rgb * brightness;
					lightAccDiff += float4(lightMul, 1);
					lightAvgDirection += lightDirection;				
				}
			}
		}
		*/		

		// Spotlights (no shadowmap or projector. Cone calculated with math)
		for (int LINDEX2=0; LINDEX2<lightAmount; LINDEX2++)		
		{
			float4 lightPosRan = lightPositionRangeCS[LINDEX2];
		
			float3 lightDirection = lightPosRan.xyz - positionVS;
			float range = length(lightDirection);
			float lrange = lightPosRan.w - range;
			lightDirection = lightDirection / range;
			float invRange = 1.0f / lightPosRan.w;
			float brightness = saturate(lrange * invRange);
			
			//brightness *= dot(lightDirection, lightDirectionCS[LINDEX2].xyz) > 0.707f;
			brightness *= saturate((dot(lightDirection, lightDirectionCS[LINDEX2].xyz) - 0.69f) * 5.0f);

			float3 lightMul = lightColor[LINDEX2].rgb * brightness;
			lightAccDiff += float4(lightMul, 1);
			lightAvgDirection += lightDirection * brightness;				
		}
		
		lightAvgDirectionXYZFrame.xyz = -normalize(lightAvgDirection);
	}

	lightAccDiff.w = saturate(lightAccDiff.w);		// Clamp alpha value to range [0, 1]
	lightAccDiff.w = 1;
	
	float4 lightAccumulatorXYZAlpha = lightAccDiff * colorXYZW;								// alpha multiplier in w channel

	int writeIndex = index * 4;
	asm 
	{
		alloc export=2
		mad eA, writeIndex, const01, exportAddress
		mov eM0, posScreenXYZRad
		mov eM1, expandDirection
		mov eM2, lightAccumulatorXYZAlpha
		mov eM3, lightAvgDirectionXYZFrame
	};
}


float4 PSDummy() : COLOR0
{
	return float4(0,0,0,0);
};


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PSDummy();
    }
}


