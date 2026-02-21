
float4x4 matrixWVP : WORLDVIEWPROJECTION;
float4x4 matrixWVPSurfaceCache[2] : WORLDVIEWPROJECTION;
float objectID;
float frameMod;


texture surfaceCacheFuture;
sampler surfaceCacheFutureSampler = 
sampler_state
{
    Texture = <surfaceCacheFuture>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
};


texture surfaceCachePast;
sampler surfaceCachePastSampler = 
sampler_state
{
    Texture = <surfaceCachePast>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
};


struct VS_OUT
{
	float4 ProjPos  : POSITION;
	float4 tex1	: TEXCOORD0;
	float4 tex2	: TEXCOORD1;
};


VS_OUT VS(float3 objPos : POSITION)
{
	VS_OUT Out;
	float4 vpos = float4(objPos, 1);
	Out.ProjPos = mul(vpos, matrixWVP);
	Out.tex1 = mul(vpos, matrixWVPSurfaceCache[0]);
	Out.tex2 = mul(vpos, matrixWVPSurfaceCache[1]);
	return Out;
}


struct PS_IN
{
	float4 tex1    : TEXCOORD0;
	float4 tex2    : TEXCOORD1;
};


float4 PS(PS_IN In) : COLOR
{
	float2 texc1 = In.tex1.xyz / In.tex1.w;
	float4 texColor1 = tex2D(surfaceCacheFutureSampler, texc1);
	float color1mul = saturate(abs(texColor1.a - objectID) * 1024);

	float2 texc2 = In.tex2.xyz / In.tex2.w;
	float4 texColor2 = tex2D(surfaceCachePastSampler, texc2);
	float color2mul = saturate(abs(texColor2.a - objectID) * 1024);

	//return float4(color1mul, color2mul, 0, 1);
	//return (0, 1, 0, 1);

	float interpolator = saturate(frameMod + color1mul - color2mul);
	return lerp(texColor1, texColor2, interpolator);
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


