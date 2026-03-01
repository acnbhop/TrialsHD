
//half4x4 matrixW : WORLD;
//half4x4 matrixWVP : WORLDVIEW;

half4 materialColor;

half3 SimpleLightDirection = half3(-0.577, 0.577, -0.577);
half3 SimpleLightColor = half3(0.7, 0.7, 0.45);
half3 SimpleLightAmbient = half3(0.05, 0.05, 0.15);

float objectID;

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
float3 ambientMultiplier;

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

struct VS_OUTPUT
{
    half4 pos		: POSITION;
};


VS_OUTPUT VS(half3 inPos		: POSITION)
{
	//OBJEKTIS_VAARA_SHADERI_SIPE_KORJAA();
    VS_OUTPUT outStruct;
    outStruct.pos  = float4(0,0,-1,1);
    return outStruct;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = null;
    }
}


