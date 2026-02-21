
float4x4 matrixWVP : WORLDVIEW;

float4 shadowDepthScale;

texture mapNormalHeightAmbient;
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


struct VS_OUTPUT
{
    float4 pos				: POSITION;
    float3 texLinearDepth	: TEXCOORD0;
};


VS_OUTPUT VS(
    float3 inPos		: POSITION,
    float2 inTex		: TEXCOORD0)
{
    VS_OUTPUT outStruct;
	outStruct.pos = mul(float4(inPos, 1), matrixWVP);
    //outStruct.texLinearDepth  = 32.0f * float3(inTex, outStruct.pos.z) + 0.02f;	// added depth bias
	outStruct.texLinearDepth = float3(inTex, shadowDepthScale.x * outStruct.pos.z + shadowDepthScale.y);		// multiplied to [0,32] range and added depth bias    
    return outStruct;
}


float4 PS(VS_OUTPUT psIn) : COLOR
{
	float4 texHeight4d = tex2D(mapNormalHeightAmbientSampler, psIn.texLinearDepth.xy);
	clip(texHeight4d.r-0.3);
	return float4(psIn.texLinearDepth.z, 1, 0, 0);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}

