
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
	
	float3 positionWS1 = positionXYZRadius.xyz - (0.5 * velocityXYZFrame.xyz);
	float3 positionWS2 = positionXYZRadius.xyz + (0.5 * velocityXYZFrame.xyz);
	float3 positionWSC = (positionWS1 + positionWS2) * 0.5;

	float4 positionVS = mul(float4(positionWSC, 1), matrixV);

	// Transform positions to post projection screen space
	float4 posScreenXYZRad1 = mul(float4(positionWS1, 1), matrixVP);
	float4 posScreenXYZRad2 = mul(float4(positionWS2, 1), matrixVP);
	posScreenXYZRad1.xyz /= posScreenXYZRad1.w;
	posScreenXYZRad2.xyz /= posScreenXYZRad2.w;

/*
	// Transform position to post projection screen space
	float4 posScreenXYZRad1 = mul(float4(positionWS1, 1), matrixVP);
	posScreenXYZRad1.xyz /= posScreenXYZRad1.w;

	// Transform second position to post projection screen space
	float4 posScreenXYZRad2 = mul(float4(positionWS2, 1), matrixVP);
	posScreenXYZRad2.xyz /= posScreenXYZRad2.w;
*/

	// Store post projection radius for particle points (in w-channel)
	posScreenXYZRad1.w = positionXYZRadius.w / posScreenXYZRad1.w;
	posScreenXYZRad2.w = positionXYZRadius.w / posScreenXYZRad2.w;

	// Calculate post projection radius expanded points
	float3 posScreenVel = posScreenXYZRad1.xyz - posScreenXYZRad2.xyz;
	posScreenVel.y *= (9.0/16.0);
	float velLenXY = length(posScreenVel.xy);
	posScreenVel /= velLenXY;		// Normalize only on XY plane
	posScreenXYZRad1.xyz += posScreenVel * posScreenXYZRad1.w;
	posScreenXYZRad2.xyz -= posScreenVel * posScreenXYZRad2.w;

	// Calculate center point and velocity vector in post projection screen space
	float3 posScreenCenter = (posScreenXYZRad1.xyz + posScreenXYZRad2.xyz) * 0.5;
	
	// Store center and radius
	float4 posScreenXYZRadC = float4(posScreenCenter, posScreenXYZRad1.w);
	posScreenXYZRadC.z = 1.0 - posScreenXYZRadC.z;  // 16 bit float z value precision improvement (more precision near zero point)

	// Calculate expand direction and store radius 2
	float4 expandDirection = float4((posScreenXYZRad1.xyz - posScreenCenter), posScreenXYZRad2.w);

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
			// Transform particle to PSSM space (TODO: Optimize)
			float4 posPSSM[3];
			posPSSM[0] = mul(float4(positionWSC, 1), matrixPSSM[0]);
			posPSSM[1] = mul(float4(positionWSC, 1), matrixPSSM[1]);
			posPSSM[2] = mul(float4(positionWSC, 1), matrixPSSM[2]);
		
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
			float lightZ = tex2Dlod(gbufferMotionVectorsSampler, float4(texLmap.xy, 0, 1)).x;		
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
				
		// Transform average light direction to particle space
		float3 lightTrans = -normalize(lightAvgDirection);
		float2 expandNorm = normalize(expandDirection.xy);
		expandNorm.x = -expandNorm.x;
		//lightTrans.x = dot(expandNorm, -lightAvgDirection.yx);
		lightTrans.x = dot(expandNorm, lightTrans.yx);
		lightTrans.y = dot(float2(expandNorm.y, -expandNorm.x), -lightAvgDirection.yx);
		lightAvgDirectionXYZFrame.xyz = lightTrans;
	}

	//lightAccDiff.w = saturate(lightAccDiff.w);		// Clamp alpha value to range [0, 1]	
	lightAccDiff.w = 1;
	float4 lightAccumulatorXYZAlpha = lightAccDiff * colorXYZW;							// alpha multiplier in w channel

	if (posScreenXYZRad1.z <= 0.5f || posScreenXYZRad2.z <= 0.5f)
	{
		expandDirection = float4(0.00001f, 0.0f, 0.0f, 0.0f);
		posScreenXYZRadC = float4(9999, 9999, 9999, 0);
		//posScreenXYZRad1 = 99999;
		//posScreenXYZRad2 = 99999;
	}

	int writeIndex = index * 4;
	asm 
	{
		alloc export=2
		mad eA, writeIndex, const01, exportAddress
		mov eM0, posScreenXYZRadC
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


