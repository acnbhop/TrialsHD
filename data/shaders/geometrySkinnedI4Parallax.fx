
float4x4 matrixP : PROJECTION;
float4x4 matrixWV[30] : WORLDVIEW;

float4 materialColor;		// (r, g, b, a)
float4 materialProperties;	// (specular power, diffuse amount, specular amount, self illumination amount)
float4 materialParallax;	// (bumpheight, parallax_multiplier, parallax_add, 1/bumpheight)
float3 cameraPosition;
float4 morphMultipliers;

texture mapColor;
texture mapNormalHeightAmbient;
texture mapMaterialProperties;

sampler mapColorSampler = 
sampler_state
{
    Texture = <mapColor>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = ANISOTROPIC;
    MagFilter = ANISOTROPIC;
    MipFilter = LINEAR;
    MaxAnisotropy = 2;
};


sampler mapNormalHeightAmbientSampler = 
sampler_state
{
    Texture = <mapNormalHeightAmbient>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = ANISOTROPIC;
    MagFilter = ANISOTROPIC;
    MipFilter = LINEAR;
    MaxAnisotropy = 2;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    float2 tex					: TEXCOORD0;
    float3 normal				: TEXCOORD1;
    float3 tangent				: TEXCOORD2;
    float3 binormal				: TEXCOORD3;
    float3 positionVS			: TEXCOORD4;
};


struct PS_OUTPUT
{
	float4 rtColorEmi	: COLOR0;
	float4 rtNormalSpec	: COLOR1;
};


VS_OUTPUT VS(
    float3 inPos	: POSITION0,
    float3 inNormal	: NORMAL,
    float3 inTangent	: TANGENT,
    float3 inBinormal	: BINORMAL,
    float2 inTex	: TEXCOORD0,
    float4 inWeights	: BLENDWEIGHT,
    uint4  inIndices	: BLENDINDICES)
{
    VS_OUTPUT outStruct;

	outStruct.positionVS = mul(float4(inPos, 1), (float4x3)matrixWV[inIndices.x]) * inWeights.x;
	outStruct.positionVS += mul(float4(inPos, 1), (float4x3)matrixWV[inIndices.y]) * inWeights.y;
	outStruct.positionVS += mul(float4(inPos, 1), (float4x3)matrixWV[inIndices.z]) * inWeights.z;
	outStruct.positionVS += mul(float4(inPos, 1), (float4x3)matrixWV[inIndices.w]) * inWeights.w;

    outStruct.pos = mul(float4(outStruct.positionVS, 1), matrixP);		

    outStruct.normal    = mul(inNormal, (float3x3)matrixWV[inIndices.x]) * inWeights.x;
    outStruct.normal    += mul(inNormal, (float3x3)matrixWV[inIndices.y]) * inWeights.y;
    outStruct.normal    += mul(inNormal, (float3x3)matrixWV[inIndices.z]) * inWeights.z;
    outStruct.normal    += mul(inNormal, (float3x3)matrixWV[inIndices.w]) * inWeights.w;
    outStruct.normal    = normalize(outStruct.normal);

    outStruct.tangent    = mul(inTangent, (float3x3)matrixWV[inIndices.x]) * inWeights.x;
    outStruct.tangent    += mul(inTangent, (float3x3)matrixWV[inIndices.y]) * inWeights.y;
    outStruct.tangent    += mul(inTangent, (float3x3)matrixWV[inIndices.z]) * inWeights.z;
    outStruct.tangent    += mul(inTangent, (float3x3)matrixWV[inIndices.w]) * inWeights.w;
    outStruct.tangent    = normalize(outStruct.tangent);

    outStruct.binormal    = mul(inBinormal, (float3x3)matrixWV[inIndices.x]) * inWeights.x;
    outStruct.binormal    += mul(inBinormal, (float3x3)matrixWV[inIndices.y]) * inWeights.y;
    outStruct.binormal    += mul(inBinormal, (float3x3)matrixWV[inIndices.z]) * inWeights.z;
    outStruct.binormal    += mul(inBinormal, (float3x3)matrixWV[inIndices.w]) * inWeights.w;
    outStruct.binormal    = normalize(outStruct.binormal);
    
    outStruct.tex  = inTex;	// TODO: Optimize to normal.w, tangent.w

    return outStruct;
}


PS_OUTPUT PS(VS_OUTPUT psIn) : COLOR
{
    PS_OUTPUT outStruct;
	float3 normal = normalize(psIn.normal);
	float3 tangent = normalize(psIn.tangent);
	float3 binormal = normalize(psIn.binormal);

	// Calculate pixel's parallax texture coordinate shift from pixel height and camera direction transformed to tangent space
	float3 pixelDirection = normalize(psIn.positionVS);
	float texHeight = (tex2D(mapNormalHeightAmbientSampler, psIn.tex).g - 0.5) * materialParallax.x * -0.025;
	float2 parallaxShift = psIn.tex;
	parallaxShift.y -= dot(binormal, pixelDirection) * texHeight;
	parallaxShift.x += dot(tangent, pixelDirection) * texHeight;

	float4 texNCombine1 = tex2D(mapColorSampler, parallaxShift);					// RGB, NX
	float4 texNCombine2 = tex2D(mapNormalHeightAmbientSampler, parallaxShift);		// Sp, H, Gl, NY
	float4 materialPropertiesM = materialProperties;
	
	float3 texNormal;
	texNormal.x = (texNCombine1.a * 2) - 1;
	texNormal.y = (texNCombine2.a * 2) - 1;
	texNormal.z = sqrt(1 - texNormal.x*texNormal.x - texNormal.y*texNormal.y);
	float2 texNormal2 = normalize((texNormal.x) * tangent - (texNormal.y) * binormal + texNormal.z * normal * materialParallax.w).xy;
	texNormal2 = (texNormal2 + 1) * 0.5;

	outStruct.rtColorEmi = float4(texNCombine1.rgb * materialColor * materialPropertiesM.y, (materialPropertiesM.w) * 0.5);
    outStruct.rtNormalSpec = float4(texNormal2, texNCombine2.rb * materialPropertiesM.zx);
    return outStruct;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


