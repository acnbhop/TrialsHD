
float4x4 matrixWVP;
float4x4 matrixWVPSurfaceCache[2];
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
    MipFilter = LINEAR;
};


texture surfaceCachePast;
sampler surfaceCacheFutureSampler = 
sampler_state
{
    Texture = <surfaceCachePast>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


struct VS_OUT
{
	float4 ProjPos  : POSITION;
	float4 tex1	: TEXCOORD0;
	float4 tex2	: TEXCOORD1;
};


VS_OUT main(float4 ObjPos : POSITION)
{
	VS_OUT Out;
	Out.ProjPos = mul(matrixWVP, ObjPos);
	Out.tex1 = mul(matrixWVPSurfaceCache[0], ObjPos);
	Out.tex2 = mul(matrixWVPSurfaceCache[1], ObjPos);
	return Out;
}


struct PS_IN
{
	float4 tex1    : TEXCOORD0;
	float4 tex2    : TEXCOORD1;
};


float4 main(PS_IN In) : COLOR
{
	float2 texc1 = In.tex1.xyz / In.tex1.w;
	float4 texColor1 = tex2D(surfaceCacheFutureSampler, texc1);
	float color1mul = saturate(abs(texColor1.a - objectID) * 1024);

	float2 texc2 = In.tex2.xyz / In.tex2.w;
	float4 texColor2 = tex2D(surfaceCachePastSampler, texc2);
	float color2mul = saturate(abs(texColor2.a - objectID) * 1024);

	float interpolator = saturate(frameMod + color1mul - color2mul);
	return lerp(texColor1, texColor2, interpolator);
}

