
float4x4 matrixVP : VIEWPROJECTION;
float4x4 matrixV : VIEW;


int lightAmount;
float4 lightColor[3];
float4 lightPositionRangeCS[3];
float4x4 lightMatrix[3];

int lightPointAmount;
float4 lightPointColor[3];
float4 lightPointPositionRangeCS[3];

float3 directionPSSM;
float4x4 matrixPSSM[3];
float3 colorPSSM;
//float3 ambientMultiplier;


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

/*
	float rectangleIndex = velocityXYZFrame.w;// * 255;
	float4 texCoordRectangle;
	asm
	{
		vfetch texCoordRectangle, rectangleIndex, position3
	};
*/

	// ---- Transform and project particle and radius to post projection screen space ----
	
	float3 positionWS = positionXYZRadius.xyz;

	// Transform position to post projection screen space
	float4 pos1ScreenXYZRad = mul(float4(positionWS, 1), matrixVP);
	pos1ScreenXYZRad.xyz /= pos1ScreenXYZRad.w;

	// Store post projection radius for particle points (in w-channel)
	pos1ScreenXYZRad.w = positionXYZRadius.w / pos1ScreenXYZRad.w;
	pos1ScreenXYZRad.z = 1.0 - pos1ScreenXYZRad.z;						// 16 bit float z value precision improvement


	// ---- Calculate lighting for particle (world space) center point ----

	// Directional sun light (PSSM)
	float4 lightAccDiff = float4(0.4, 0.4, 0.4, 0.4);		// Fake ambient
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
		float3 texLmap = posLMap.xyz / posLMap.w;
		texLmap.y *= 1.0f/3.0f;
		texLmap.y += (1.0f/3.0f)*pssmIndex;
		
		// Sample light depth map
		float lightZ = tex2Dlod(gbufferMotionVectorsSampler, float4(texLmap.xy, 0, 1)).x;		
		float lightMul = (lightZ <= (1.0f - texLmap.z)) * 0.2;
		
		lightAccDiff += float4(colorPSSM, 1) * lightMul;
	}

	lightAccDiff.w = saturate(lightAccDiff.w);		// Clamp alpha value to range [0, 1]
	
	float4 lightAccumulatorXYZAlpha =lightAccDiff * colorXYZW;					// alpha multiplier in w channel
	float4 lightAvgDirectionXYZFrame = float4(-directionPSSM, velocityXYZFrame.w);								// frame in w channel

	int writeIndex = index * 4;
	asm 
	{
		alloc export=2
		mad eA, writeIndex, const01, exportAddress
		mov eM0, pos1ScreenXYZRad
		mov eM1, pos1ScreenXYZRad
		mov eM2, lightAccumulatorXYZAlpha
		mov eM3, lightAvgDirectionXYZFrame
		//mov eM3, texCoordRectangle
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


