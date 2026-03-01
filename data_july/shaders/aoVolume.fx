
float4x4 matrixWV;
float4x4 matrixP;
float4x4 matrixUser;	// Matrix to convert from view space to object volume texture space

float2 zConversion;


texture3D mapColor;
sampler3D mapColorSampler = 
sampler_state
{
    Texture = <mapColor>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
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
    float3 positionVS			: TEXCOORD0;
};


VS_OUTPUT VS(float3 inPos : POSITION)
{
    VS_OUTPUT outStruct;
	outStruct.positionVS = mul(float4(inPos, 1), (float4x3)matrixWV);
    outStruct.pos = mul(float4(outStruct.positionVS, 1), matrixP);		
    return outStruct;
}


float4 PS(VS_OUTPUT psIn, float2 tex : VPOS) : COLOR
{
	// 2x2 MSAA hack vpos adjustement
	tex *= 2;

	// Sample depth using vpos	
    float4 texDepth;					// Depth
    asm 
    {
		tfetch2D texDepth, tex,
                 gbufferDepthSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5
	};

	// Convert texDepth to view space
	float pixelDepthVS = zConversion.y / ((1.0f - texDepth.x) - zConversion.x);
	float3 pixelPositionVS = (psIn.positionVS.xyz / psIn.positionVS.z) * pixelDepthVS;

	// Transform view space coordinate to volume texture space
	float3 texCoord3D = mul(float4(pixelPositionVS, 1), (float4x3)matrixUser);

	float occlusionFactor = 1;
	if (!any(floor(texCoord3D+0.5)))
	{		
		// Sample normal using vpos
		float4 texNormalSpec;				// normal.xy, specular, glossiness
		asm 
		{
			tfetch2D texNormalSpec, tex,
					 gbufferNormalSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5
		};

		// Calculate normal vector from two channel input
		float3 texNormal2;
		texNormal2.xy = (texNormalSpec.xy * 2) - 1;
		texNormal2.z = -sqrt(1 - texNormal2.x*texNormal2.x - texNormal2.y*texNormal2.y);

		// Sample normal.xyz and occlusion amount from volume texture
		//float4 aoDirectionOcclusion = tex3D(mapColorSampler, texCoord3D);
		
		// TEST: Simple sphere occlusion
		//float3 dirLocal = 0.5-texCoord3D;
		float3 dirLocal = -texCoord3D;
		//float4 aoDirectionOcclusion = float4(dirLocal, saturate(0.4*length(dirLocal)-0.5f));
		float4 aoDirectionOcclusion = float4(dirLocal, saturate(3.0*length(dirLocal)-0.5f));

		// Transform AO map normal vector to view space
		float3 viewNormal = normalize(mul(aoDirectionOcclusion.xyz, (float3x3)matrixWV));		// normal to view space
		
		// Calculate occlusion factor
		occlusionFactor = 0.5 * (dot(texNormal2, viewNormal) + 1);
		occlusionFactor *= 1-aoDirectionOcclusion.a;
		occlusionFactor = 1-occlusionFactor;
	}
	
	// Output the occlusion amount to the pixel (multiple alpha)
	return float4(occlusionFactor.xxx, 1);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


