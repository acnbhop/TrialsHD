
float4x4 matrixWV;
float4x4 matrixP;

float4 materialColor;		// (r, g, b, a)
float4 materialProperties;	// (specular power, diffuse amount, specular amount, self illumination amount)
float4 materialParallax;	// (bumpheight, parallax_multiplier, parallax_add, 1/bumpheight)
float3 cameraPosition;

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


sampler mapMaterialPropertiesSampler = 
sampler_state
{
    Texture = <mapMaterialProperties>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = ANISOTROPIC;
    MagFilter = ANISOTROPIC;
    MipFilter = LINEAR;
    MaxAnisotropy = 2;
};


texture mapReflection;
sampler mapReflectionSampler = 
sampler_state
{
    Texture = <mapReflection>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
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
    float3 inPos		: POSITION,
    float3 inNormal		: NORMAL,
    float3 inTangent	: TANGENT,
    float3 inBinormal	: BINORMAL,
    float2 inTex		: TEXCOORD0)
{
    VS_OUTPUT outStruct;

	outStruct.positionVS = mul(float4(inPos, 1), (float4x3)matrixWV);
    outStruct.pos = mul(float4(outStruct.positionVS, 1), matrixP);		

    outStruct.normal    = normalize(mul(inNormal, (float3x3)matrixWV));		// normal to view space
    outStruct.tangent   = normalize(mul(inTangent, (float3x3)matrixWV));	// tangent to view space
    outStruct.binormal  = normalize(mul(inBinormal, (float3x3)matrixWV));	// binormal to view space
    
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

	// Calculate normal vector from 2 channel input, transform it to view space by using interpolated vertex normal/tangent/binormal, and then store it.
	float4 texNCombine1 = tex2D(mapColorSampler, parallaxShift);
	float4 texNCombine2 = tex2D(mapNormalHeightAmbientSampler, parallaxShift);
	float4 texCombine = tex2D(mapMaterialPropertiesSampler, parallaxShift);
	
	float4 materialPropertiesM = materialProperties;
	materialPropertiesM -= (1.0f - materialPropertiesM.g) * texCombine.r;
	
	// Calculate normal vector from 2 channel input, transform it to view space by using interpolated vertex normal/tangent/binormal
	float3 texNormal;
	texNormal.x = (texNCombine1.a * 2) - 1;
	texNormal.y = (texNCombine2.a * 2) - 1;
	texNormal.z = sqrt(1 - texNormal.x*texNormal.x - texNormal.y*texNormal.y);
	float3 texNormal2 = normalize((texNormal.x) * tangent - (texNormal.y) * binormal + texNormal.z * normal * materialParallax.w);

	// Chrome / reflection
	float pran = dot(texNormal2, pixelDirection);
	float3 reflectionVector = pixelDirection - texNormal2*(2*pran);
	float3 reflectionColor = texCUBE(mapReflectionSampler, reflectionVector).rgb;

	texNormal2.xy = (texNormal2.xy + 1) * 0.5;

	outStruct.rtColorEmi = float4(texNCombine1.rgb * materialColor.rgb * (materialPropertiesM.y * texCombine.a) + reflectionColor * 0.3f * texNCombine2.b, (materialPropertiesM.w + texNCombine2.b * dot(reflectionColor, reflectionColor) * 0.001f) * 0.5);
    outStruct.rtNormalSpec = float4(texNormal2.xy, texCombine.gb * materialPropertiesM.zx * 0.5f);
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














