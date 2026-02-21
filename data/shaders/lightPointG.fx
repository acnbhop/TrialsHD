
float4x4 matrixP;
float4x4 matrixWV;

float2 zConversion;

float4 lightPositionRangeCS;
float4 lightColor;



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
	// Deferred point light shader
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
	
	// Calculate pointlight diffuse and specular intensity for pixel
	{	
		float4 lightPosRan = lightPositionRangeCS;
		float3 lightDirection = lightPosRan.xyz - pixelPositionVS;
	
		float range = length(lightDirection);
		float lrange = lightPosRan.w - range;
	
		lightDirection = lightDirection / range;
		float brightness = saturate(lrange / lightPosRan.w);
		
		float3 specHalfVector = normalize(lightDirection - pixelDirection);

		float diffuse = saturate(dot(texNormal2, lightDirection));
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
		float3 lightMul = lightColor.rgb * brightness;
		
		float3 lightAccDiff = lightMul * diffuse;
		float3 lightAccSpec = lightMul * specular;
		
		//return float4(0.2, 0.1, 0.1, 1);
		return float4(lightAccDiff * texColor.rgb + lightAccSpec*3, 1);
	}
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


