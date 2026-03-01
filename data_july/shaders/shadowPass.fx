
float4x4 matrixWVP : WORLDVIEWPROJECTION;

int lightAmount;
float4x4 lightMatrix[4];
float4x4 matrixPSSM[3];

float time;

texture lightMaskTexture;
sampler lightMaskTextureSampler = 
sampler_state
{
    Texture = <lightMaskTexture>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
};

texture gbufferMaterialProperties;
sampler gbufferMaterialPropertiesSampler = 
sampler_state
{
	Texture = <gbufferMaterialProperties>;
	AddressU = Wrap;
	AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
};

texture gbufferMotionVectors0;
texture gbufferMotionVectors1;
texture gbufferMotionVectors2;
sampler gbufferMotionVectorsSampler[3] = 
{
	sampler_state
	{
		Texture = <gbufferMotionVectors0>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = POINT;
		MagFilter = POINT;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <gbufferMotionVectors1>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = POINT;
		MagFilter = POINT;
		MipFilter = POINT;
	}	
	,
	sampler_state
	{
		Texture = <gbufferMotionVectors2>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = POINT;
		MagFilter = POINT;
		MipFilter = POINT;
	}	
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


struct VS_OUTPUT
{
    float4 pos			: POSITION;
    float4 posLmap[4]	: TEXCOORD0;
    float4 posPSSM[3]	: TEXCOORD4;
};


VS_OUTPUT VS(
    float3 inPos		: POSITION)
{
    VS_OUTPUT outStruct;

    outStruct.pos = mul(float4(inPos, 1), matrixWVP);

    outStruct.posLmap[0] = mul(float4(inPos, 1), lightMatrix[0]);
    outStruct.posLmap[1] = mul(float4(inPos, 1), lightMatrix[1]);
    outStruct.posLmap[2] = mul(float4(inPos, 1), lightMatrix[2]);

    outStruct.posPSSM[0] = mul(float4(inPos, 1), matrixPSSM[0]);
    outStruct.posPSSM[1] = mul(float4(inPos, 1), matrixPSSM[1]);
    outStruct.posPSSM[2] = mul(float4(inPos, 1), matrixPSSM[2]);

    return outStruct;
}



float4 PS(VS_OUTPUT psIn) : COLOR
{
		
	// Directional sun light (PSSM)
	{
		//float3 texLmap = psIn.posPSSM[0].xyz / psIn.posPSSM[0].w;
		//if (any(floor(texLmap))) texLmap = psIn.posPSSM[1].xyz / psIn.posPSSM[1].w;

		float3 texLmap1 = psIn.posPSSM[0].xyz / psIn.posPSSM[0].w;
		float pssmIndex = any(floor(texLmap1));
		float3 texLmap2 = psIn.posPSSM[1].xyz / psIn.posPSSM[1].w;
		pssmIndex += pssmIndex * any(floor(texLmap2));
		float4 posLMap = psIn.posPSSM[pssmIndex];
		float3 texLmap = posLMap.xyz / posLMap.w;
		
		// Sample light depth map
		float lightZ = tex2D(gbufferMotionVectorsSampler[pssmIndex], texLmap.xy).x;
		float3 lightMul = 1; //tex2D(gbufferMaterialPropertiesSampler, psIn.tex.zw * 0.0313 + time * 0.1f).xyz;
		lightMul *= (lightZ <= (1.0f - texLmap.z));
		
	}
	
	// Spots
	for (int i=0; i<lightAmount; i++)
	{
		float4 lightPosRan = lightPositionRangeCS[i];
		float4 posLMap = psIn.posLmap[i];
		
		// Sample light depth map
		float3 texLmap = posLMap.xyz / posLMap.w;
		float lightZ = tex2D(lightMapTextureSampler[i], texLmap.xy).x;
		float3 lightMul = tex2D(lightMaskTextureSampler, texLmap.xy).xyz;
		lightMul *= (lightZ <= (1.0f - texLmap.z));
		
	}

	
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}

