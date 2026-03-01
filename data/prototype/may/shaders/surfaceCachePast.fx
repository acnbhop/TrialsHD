
float4x4 matrixWVP : WORLDVIEWPROJECTION;
float4x4 matrixWVPSurfaceCache : WORLDVIEWPROJECTION;
float objectID;


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
};


VS_OUT VS(float3 objPos : POSITION)
{
	VS_OUT Out;
	float4 vpos = float4(objPos, 1);
	Out.ProjPos = mul(vpos, matrixWVP);
	Out.tex1 = mul(vpos, matrixWVPSurfaceCache);
	return Out;
}


struct PS_IN
{
	float4 tex1    : TEXCOORD0;
};


float4 PS(PS_IN In) : COLOR
{
	float2 texc1 = In.tex1.xyz / In.tex1.w;
	float4 texColor1 = tex2D(surfaceCachePastSampler, texc1);
	return texColor1;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


