
float4x4 matrixWV : WORLDVIEW;
float4x4 matrixP :PROJECTION;

float4 materialColor;		// (r, g, b, a)
float4 materialParallax;	// (bumpheight, parallax_multiplier, parallax_add, 1/bumpheight)
float4 materialProperties;	// (specular power, diffuse amount, specular amount, self illumination amount)

float4 lightColor[16];
float4 lightPositionRangeCS[16];

texture mapColor;
texture mapNormalHeightAmbient;
texture mapMaterialProperties;
texture lightIndexTexture;


sampler lightIndexTextureSampler = 
sampler_state
{
    Texture = <lightIndexTexture>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;
};


sampler mapColorSampler = 
sampler_state
{
    Texture = <mapColor>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


sampler mapNormalHeightAmbientSampler = 
sampler_state
{
    Texture = <mapNormalHeightAmbient>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


sampler mapMaterialPropertiesSampler = 
sampler_state
{
    Texture = <mapMaterialProperties>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


struct VS_OUTPUT
{
    float4 pos			: POSITION;
    float2 tex			: TEXCOORD0;
    float3 normal		: TEXCOORD1;
    float3 tangent		: TEXCOORD2;
    float3 binormal		: TEXCOORD3;
    float4 posVS		: TEXCOORD4;	
    float3 posScreen	: TEXCOORD5;
};


VS_OUTPUT VS(
    float3 inPos		: POSITION,
    float3 inNormal		: NORMAL,
    float3 inTangent	: TANGENT,
    float3 inBinormal	: BINORMAL,
    float2 inTex		: TEXCOORD0)
{
    VS_OUTPUT outStruct;

    outStruct.normal    = normalize(mul(inNormal, (float3x3)matrixWV));			// normal to view space
    outStruct.tangent   = normalize(mul(inTangent, (float3x3)matrixWV));		// tangent to view space
    outStruct.binormal  = normalize(mul(inBinormal, (float3x3)matrixWV));		// binormal to view space
    
    outStruct.posVS = mul(float4(inPos, 1), matrixWV);
    //outStruct.posVS = float4(mul(inPos, (float4x3)matrixWV), 1);
    outStruct.pos = mul(outStruct.posVS, matrixP);
    outStruct.posScreen = outStruct.pos.xyw;
    outStruct.posScreen.y *= -1;    
    outStruct.tex  = inTex;

    return outStruct;
}


float4 PS(VS_OUTPUT psIn) : COLOR
{
	// Sample light index buffer for this pixel
	float2 texScreen = psIn.posScreen.xy / psIn.posScreen.z;
	texScreen = (texScreen * 0.5) + 0.5 + float2(0.5/1280.0, 0.5/720.0);
	float4 lightIndices = tex2D(lightIndexTextureSampler, texScreen);

	float3 normal = normalize(psIn.normal);
	float3 tangent = normalize(psIn.tangent);
	float3 binormal = normalize(psIn.binormal);

	// Calculate pixel's parallax texture coordinate shift from pixel height and camera direction transformed to tangent space
	half3 pixelDirection = normalize(psIn.posVS.xyz);
	//half texHeight = (tex2D(mapNormalHeightAmbientSampler, psIn.tex).g - 0.5) * materialParallax.x * -0.025;
	half4 texHeight4d = tex2D(mapNormalHeightAmbientSampler, psIn.tex);
	half texHeight = (texHeight4d.g - 0.5) * materialParallax.x * -0.025;
	clip(texHeight4d.r-0.3);
	half2 parallaxShift = psIn.tex;
	parallaxShift.y -= dot(binormal, pixelDirection) * texHeight;
	parallaxShift.x += dot(tangent, pixelDirection) * texHeight;

	float4 texNCombine1 = tex2D(mapColorSampler, parallaxShift);
	float4 texNCombine2 = tex2D(mapNormalHeightAmbientSampler, parallaxShift);
	float4 texCombine = tex2D(mapMaterialPropertiesSampler, parallaxShift);

	// Calculate normal vector from 2 channel input, transform it to view space by using interpolated vertex normal/tangent/binormal
	float3 texNormal;
	texNormal.x = (texNCombine1.a * 2) - 1;
	texNormal.y = (texNCombine2.a * 2) - 1;
	texNormal.z = sqrt(1 - texNormal.x*texNormal.x - texNormal.y*texNormal.y);
	float3 texNormal2 = normalize((texNormal.x) * tangent - (texNormal.y) * binormal + texNormal.z * normal * materialParallax.w);
	
	float3 vertexPosition = psIn.posVS.xyz;
	float3 lightAcc = texNCombine1.rgb * texNCombine2.b;	// Emissive
	// TODO: Add ambient!!
	// lightAcc += ambientCube * texCombine.a;
	
	float lightIndicesArray[4] = {lightIndices.x, lightIndices.y, lightIndices.z, lightIndices.w};		// TODO: Optimize!!
	//float lightIndicesArray[4] = lightIndices;
	for (int i=0; i<4; i++)
	{
		int lightIndex = lightIndicesArray[i]*255;
		float4 lightPosRan = lightPositionRangeCS[lightIndex];
		
		float3 lightDirection = lightPosRan.xyz - vertexPosition;
		float range = length(lightDirection);
		float lrange = lightPosRan.w - range;
		lightDirection = lightDirection / range;
		float brightness = saturate(lrange / lightPosRan.w);
		
		//half3 vertexDirection = -normalize(psIn.posVS.xyz);
		half3 vertexDirection = normalize(psIn.posVS.xyz);
		half3 specHalfVector = normalize(lightDirection - vertexDirection);
		//float3 specHalfVector = normalize(SimpleLightDirection - cameraDirection);

		/*
		half diffuse = saturate(dot(texNormal, lightDirection)) * texMaterialProperties.y * texColor.a;
		half specular = pow(saturate(dot(texNormal, specHalfVector)), texMaterialProperties.x * 200.0) * texMaterialProperties.z;
		return half4((brightness * lightColor * (texColor.rgb * diffuse + specular)), 1);	
		*/

		float diffuse = saturate(dot(texNormal2, lightDirection)) * materialProperties.y * texCombine.r;
		//float specular = pow(saturate(dot(texNormal2, specHalfVector)), materialProperties.x * texCombine.g * 200.0) * materialProperties.z * texCombine.b;
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), materialProperties.x * texCombine.b * 200.0) * materialProperties.z * texCombine.g;
		lightAcc += brightness * materialColor.rgb * lightColor[lightIndex] * (texNCombine1.rgb * diffuse + specular);

		//return float4(texNCombine1.rgb * (diffuse * SimpleLightColor + texCombine.a * SimpleLightAmbient + texNCombine2.b), texNCombine2.r) * materialColor + float4(specular.xxx, 0);
		
		/*
		float diffuse = saturate(dot(texNormal2, SimpleLightDirection)) * materialProperties.y * texCombine.r;
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), materialProperties.x * texCombine.b * 150.0) * materialProperties.z * texCombine.g;
		return float4(texNCombine1.rgb * (diffuse * SimpleLightColor + texCombine.a * SimpleLightAmbient + texNCombine2.b), texNCombine2.r) * materialColor + float4(specular.xxx, 0);
		*/
	}
	return float4(lightAcc, 1);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}

