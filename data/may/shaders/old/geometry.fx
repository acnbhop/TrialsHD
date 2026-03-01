
half4x4 matrixW : WORLD;
half4x4 matrixWVP : WORLDVIEW;

half4 materialColor;

half3 SimpleLightDirection = half3(-0.577, 0.577, -0.577);
half3 SimpleLightColor = half3(0.7, 0.7, 0.45);
half3 SimpleLightAmbient = half3(0.05, 0.05, 0.15);

float4 lightColor[4];
float4 lightPositionRangeCS[4];

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


struct VS_OUTPUT
{
    half4 pos		: POSITION;
    half2 tex		: TEXCOORD0;
    half3 color	: COLOR0;
};


VS_OUTPUT VS(
    half3 inPos		: POSITION,
    half3 inNormal		: NORMAL,
    half2 inTex		: TEXCOORD0)
{
    VS_OUTPUT outStruct;

	half3 normal = normalize(mul(inNormal, (half3x3)matrixW));		// normal to world space
	outStruct.color = saturate(dot(SimpleLightDirection, normal)) * SimpleLightColor + SimpleLightAmbient;
    outStruct.pos  = mul(half4(inPos, 1), matrixWVP);			// position to view space
    outStruct.tex  = inTex;

    return outStruct;
}


half4 PS(VS_OUTPUT psIn) : COLOR
{
	half4 color = tex2D(mapColorSampler, psIn.tex.xy) * materialColor;	
    return color * half4(psIn.color, 1);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


