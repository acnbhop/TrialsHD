
float4 userInput;

texture2D postProcessInput;
texture2D postProcessInput2;
texture2D gBufferNormal;
texture2D gBufferDepth;

sampler2D mapColorSampler = 
sampler_state
{
    Texture = <postProcessInput>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


sampler2D mapColorSamplerBlur = 
sampler_state
{
    Texture = <postProcessInput2>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};


sampler2D motionVectorSampler = 
sampler_state
{
    Texture = <gBufferNormal>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};


sampler2D depthSampler = 
sampler_state
{
    Texture = <gBufferDepth>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};

float motionMult=-1.0;

float4 PSBloomMotionBlurDOF( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	float3 texColorBlur = tex2D(mapColorSamplerBlur, tex).rgb;

	float2 texMotionVectors = (tex2D(motionVectorSampler, tex).zw - 0.5)*motionMult;
	texMotionVectors.y *= -1;
	float3 blurredColor = 0;
	blurredColor += tex2D(mapColorSampler, tex).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.015).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.030).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.045).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.060).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.075).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.090).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.105).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.120).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.135).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.150).rgb;
	blurredColor /= 11;

	float texDepth = tex2D(depthSampler, tex).r;
	float blurFactor = (texDepth - userInput.x) * userInput.y;
	if (blurFactor > 0) blurFactor *= 0.5;
	float3 dofMotionColor = lerp(blurredColor, texColorBlur, saturate(abs(blurFactor)));
	
	return float4(dofMotionColor + texColorBlur*texColorBlur*1.2, 1);
}


float4 PSBloomMotionBlur( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	float3 texColorBlur = tex2D(mapColorSamplerBlur, tex).rgb;

	float2 texMotionVectors = (tex2D(motionVectorSampler, tex).zw - 0.5)*motionMult;
	texMotionVectors.y *= -1;
	float3 blurredColor = 0;
	blurredColor += tex2D(mapColorSampler, tex).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.015).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.030).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.045).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.060).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.075).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.090).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.105).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.120).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.135).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.150).rgb;
	blurredColor /= 11;
	
	return float4(blurredColor + texColorBlur*texColorBlur*1.2, 1);
}


float4 PSBloomDOF( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	float3 texColorBlur = tex2D(mapColorSamplerBlur, tex).rgb;

	float3 realColor = tex2D(mapColorSampler, tex).rgb;

	float texDepth = tex2D(depthSampler, tex).r;
	float blurFactor = (texDepth - userInput.x) * userInput.y;
	if (blurFactor > 0) blurFactor *= 0.5;
	float3 dofMotionColor = lerp(realColor, texColorBlur, saturate(abs(blurFactor)));
	
	return float4(dofMotionColor + texColorBlur*texColorBlur*1.2, 1);
}


float4 PSBloom( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	float3 texColorBlur = tex2D(mapColorSamplerBlur, tex).rgb;

	float3 realColor = tex2D(mapColorSampler, tex).rgb;

	return float4(realColor + texColorBlur*texColorBlur*1.2, 1);
}


float4 PSMotionBlurDOF( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	float3 texColorBlur = tex2D(mapColorSamplerBlur, tex).rgb;

	float2 texMotionVectors = (tex2D(motionVectorSampler, tex).zw - 0.5)*motionMult;
	texMotionVectors.y *= -1;
	float3 blurredColor = 0;
	blurredColor += tex2D(mapColorSampler, tex).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.015).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.030).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.045).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.060).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.075).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.090).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.105).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.120).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.135).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.150).rgb;
	blurredColor /= 11;

	float texDepth = tex2D(depthSampler, tex).r;
	float blurFactor = (texDepth - userInput.x) * userInput.y;
	if (blurFactor > 0) blurFactor *= 0.5;
	float3 dofMotionColor = lerp(blurredColor, texColorBlur, saturate(abs(blurFactor)));
	
	return float4(dofMotionColor, 1);
}


float4 PSMotionBlur( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	float2 texMotionVectors = (tex2D(motionVectorSampler, tex).zw - 0.5)*motionMult;
	texMotionVectors.y *= -1;
	float3 blurredColor = 0;
	blurredColor += tex2D(mapColorSampler, tex).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.015).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.030).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.045).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.060).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.075).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.090).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.105).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.120).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.135).rgb;
	blurredColor += tex2D(mapColorSampler, tex + texMotionVectors * 0.150).rgb;
	blurredColor /= 11;

	return float4(blurredColor, 1);
}


float4 PSDOF( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
	float3 texColorBlur = tex2D(mapColorSamplerBlur, tex).rgb;

	float3 realColor = tex2D(mapColorSampler, tex).rgb;

	float texDepth = tex2D(depthSampler, tex).r;
	float blurFactor = (texDepth - userInput.x) * userInput.y;
	if (blurFactor > 0) blurFactor *= 0.5;
	float3 dofMotionColor = lerp(realColor, texColorBlur, saturate(abs(blurFactor)));
	
	return float4(dofMotionColor, 1);
}


technique Basic
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSBloomMotionBlurDOF();
    }
}


technique BasicBloomMotionBlurDOF
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSBloomMotionBlurDOF();
    }
}


technique BasicBloomMotionBlur
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSBloomMotionBlur();
    }
}


technique BasicBloomDOF
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSBloomDOF();
    }
}


technique BasicBloom
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSBloom();
    }
}


technique BasicMotionBlurDOF
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSMotionBlurDOF();
    }
}


technique BasicMotionBlur
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSMotionBlur();
    }
}


technique BasicDOF
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PSDOF();
    }
}
