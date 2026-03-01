
//float4 userInput;
float2 gbufferSize;

texture gbufferMotionVectors;
sampler gbufferMotionVectorsSampler = 
sampler_state
{
	Texture = <gbufferMotionVectors>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    float2 tex					: TEXCOORD0;
    float3 tex2					: TEXCOORD1;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position / float4(gbufferSize.x/2, -gbufferSize.y/2, 1, 1) + float4(-1,1,0,0);
    outStruct.tex = texcoords;
    outStruct.tex2 = texcoords2;
    return outStruct;
}


float4 PS( float2 tex : TEXCOORD0, float3 tex2 : TEXCOORD1 ) : COLOR0
{
    float4 SampledDepth;
    float4 SampledDepth2;
    float4 SampledDepthC;
    asm {
        tfetch2D SampledDepth.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = -7.0
        tfetch2D SampledDepth._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX =  -7.0, OffsetY = 0.0
        tfetch2D SampledDepth.__x_, tex,        
                 gbufferMotionVectorsSampler, OffsetX =	 7.0, OffsetY = 0.0
        tfetch2D SampledDepth.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 7.0

        tfetch2D SampledDepth2.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  1.0, OffsetY = -4.0
        tfetch2D SampledDepth2._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX =  -4.0, OffsetY = -1.0
        tfetch2D SampledDepth2.__x_, tex,        
                 gbufferMotionVectorsSampler, OffsetX =	 4.0, OffsetY = 1.0
        tfetch2D SampledDepth2.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX =  -1.0, OffsetY = 4.0
                 
        tfetch2D SampledDepthC.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 0.0
	};

	SampledDepth -= SampledDepthC.x;
	SampledDepth2 -= SampledDepthC.x;

	float occlusion = 6.0f;
	occlusion -= saturate(SampledDepth.x * 200);
	occlusion -= saturate(SampledDepth.y * 200);
	occlusion -= saturate(SampledDepth.z * 200);
	occlusion -= saturate(SampledDepth.w * 200);
	occlusion -= saturate(SampledDepth2.x * 200) * 1.5f;
	occlusion -= saturate(SampledDepth2.y * 200) * 1.5f;
	occlusion -= saturate(SampledDepth2.z * 200) * 1.5f;
	occlusion -= saturate(SampledDepth2.w * 200) * 1.5f;

	float darkening = occlusion/3.0f;
	return float4(darkening, darkening, darkening, 1);
	//return float4(darkening, 1, 1, 1);

/*
    float4 SampledDepthC;
    asm {
        tfetch2D SampledDepthC.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 0.0
	};
    return float4(SampledDepthC.x*32, 32, 0, 0);
    */


/*
    float4 SampledDepth;
    float4 SampledDepth2;
    float4 SampledDepth3;
    float4 SampledDepth4;
    float4 SampledDepthC;
    asm {
        tfetch2D SampledDepth.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX = -2.0, OffsetY = -2.0
        tfetch2D SampledDepth._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = -1.0
        tfetch2D SampledDepth.__x_, tex,        
                 gbufferMotionVectorsSampler, OffsetX =	 2.0, OffsetY = -2.0

        tfetch2D SampledDepth.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX = -1.0, OffsetY = 0.0
        tfetch2D SampledDepthC.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 0.0
        tfetch2D SampledDepth2.x___, tex,        
                 gbufferMotionVectorsSampler, OffsetX =  1.0, OffsetY = 0.0

        tfetch2D SampledDepth2._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX = -2.0, OffsetY = 2.0
        tfetch2D SampledDepth2.__x_, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 1.0
        tfetch2D SampledDepth2.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX =  2.0, OffsetY = 2.0

        tfetch2D SampledDepth3.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX = -1.0, OffsetY = -3.0
        tfetch2D SampledDepth3._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX =  1.0, OffsetY = -3.0
        tfetch2D SampledDepth3.__x_, tex,
                 gbufferMotionVectorsSampler, OffsetX = -3.0, OffsetY = -1.0
        tfetch2D SampledDepth3.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX = -3.0, OffsetY = 1.0
        tfetch2D SampledDepth4.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  3.0, OffsetY = -1.0
        tfetch2D SampledDepth4._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX =  3.0, OffsetY = 1.0
        tfetch2D SampledDepth4.__x_, tex,
                 gbufferMotionVectorsSampler, OffsetX = -1.0, OffsetY = 3.0
        tfetch2D SampledDepth4.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX =  1.0, OffsetY = 3.0
    */
    /*
        tfetch2D SampledDepth.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX = -1.0, OffsetY = -1.0
        tfetch2D SampledDepth._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = -1.0
        tfetch2D SampledDepth.__x_, tex,        
                 gbufferMotionVectorsSampler, OffsetX =	 1.0, OffsetY = -1.0

        tfetch2D SampledDepth.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX = -1.0, OffsetY = 0.0
        tfetch2D SampledDepthC.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 0.0
        tfetch2D SampledDepth2.x___, tex,        
                 gbufferMotionVectorsSampler, OffsetX =  1.0, OffsetY = 0.0

        tfetch2D SampledDepth2._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX = -1.0, OffsetY = 1.0
        tfetch2D SampledDepth2.__x_, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 1.0
        tfetch2D SampledDepth2.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX =  1.0, OffsetY = 1.0

        tfetch2D SampledDepth3.x___, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = -2.0
        tfetch2D SampledDepth3._x__, tex,
                 gbufferMotionVectorsSampler, OffsetX = -2.0, OffsetY = 0.0
        tfetch2D SampledDepth3.__x_, tex,
                 gbufferMotionVectorsSampler, OffsetX =  2.0, OffsetY = 0.0
        tfetch2D SampledDepth3.___x, tex,
                 gbufferMotionVectorsSampler, OffsetX =  0.0, OffsetY = 2.0
    */
    /*};

	SampledDepth -= SampledDepthC.x;
	SampledDepth2 -= SampledDepthC.x;
	SampledDepth3 -= SampledDepthC.x;
	SampledDepth4 -= SampledDepthC.x;

	//float occlusion = 3.0f;
	float occlusion = 7.0f;
	occlusion -= saturate(SampledDepth.x * 200);
	occlusion -= saturate(SampledDepth.y * 200)*7;
	occlusion -= saturate(SampledDepth.z * 200);
	occlusion -= saturate(SampledDepth.w * 200)*7;
	occlusion -= saturate(SampledDepth2.x * 200)*7;
	occlusion -= saturate(SampledDepth2.y * 200);
	occlusion -= saturate(SampledDepth2.z * 200)*7;
	occlusion -= saturate(SampledDepth2.w * 200);
	occlusion -= saturate(SampledDepth3.x * 200);
	occlusion -= saturate(SampledDepth3.y * 200);
	occlusion -= saturate(SampledDepth3.z * 200);
	occlusion -= saturate(SampledDepth3.w * 200);
	occlusion -= saturate(SampledDepth4.x * 200);
	occlusion -= saturate(SampledDepth4.y * 200);
	occlusion -= saturate(SampledDepth4.z * 200);
	occlusion -= saturate(SampledDepth4.w * 200);
	//occlusion = 0.0f;

    //return float4(SampledDepthC.x*32, occlusion*4, 0, 0);
    //return float4(SampledDepthC.x*32, saturate(occlusion)*10.666, 0, 0);
    return float4(SampledDepthC.x*32, saturate(occlusion/7.0f)*32, 0, 0);*/
    
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


