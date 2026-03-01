
float hdrProperties;

texture mapColor;
sampler mapColorSampler = 
sampler_state
{
	Texture = <mapColor>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    //float2 tex					: TEXCOORD0;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position;
    //outStruct.tex = texcoords;
    return outStruct;
}


//float4 PS( float2 tex : TEXCOORD0 ) : COLOR0
float4 PS( float2 tex : VPOS ) : COLOR0
{
	// 8x8 gaussian blur filter
    float4 sampledColor;
    float4 sampledColor1;
    float4 sampledColor2;
    float4 sampledColor3;
    float4 sampledColor4;
    float4 sampledColor5;
    float4 sampledColor6;
    float4 sampledColor7;
    float4 sampledColor8;
    float4 sampledColor9;
    float4 sampledColor10;
    float4 sampledColor11;
    float4 sampledColor12;
    float4 sampledColor13;
    float4 sampledColor14;
    float4 sampledColor15;
    
    asm 
    {
		tfetch2D sampledColor, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.0, OffsetY = -3.0
		tfetch2D sampledColor1, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.0, OffsetY = -3.0
		tfetch2D sampledColor2, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  1.0, OffsetY = -3.0
		tfetch2D sampledColor3, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  3.0, OffsetY = -3.0

		tfetch2D sampledColor4, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.0, OffsetY = -1.0
		tfetch2D sampledColor5, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.0, OffsetY = -1.0
		tfetch2D sampledColor6, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  1.0, OffsetY = -1.0
		tfetch2D sampledColor7, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  3.0, OffsetY = -1.0

		tfetch2D sampledColor8, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.0, OffsetY =  1.0
		tfetch2D sampledColor9, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.0, OffsetY =  1.0
		tfetch2D sampledColor10, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  1.0, OffsetY =  1.0
		tfetch2D sampledColor11, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  3.0, OffsetY =  1.0

		tfetch2D sampledColor12, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.0, OffsetY =  3.0
		tfetch2D sampledColor13, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.0, OffsetY =  3.0
		tfetch2D sampledColor14, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  1.0, OffsetY =  3.0
		tfetch2D sampledColor15, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  3.0, OffsetY =  3.0
	};
	
	/*
    asm 
    {
		tfetch2D sampledColor, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.5, OffsetY = -3.5
		tfetch2D sampledColor1, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.5, OffsetY = -3.5
		tfetch2D sampledColor2, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  0.5, OffsetY = -3.5
		tfetch2D sampledColor3, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  2.5, OffsetY = -3.5

		tfetch2D sampledColor4, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.5, OffsetY = -1.5
		tfetch2D sampledColor5, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.5, OffsetY = -1.5
		tfetch2D sampledColor6, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  0.5, OffsetY = -1.5
		tfetch2D sampledColor7, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  2.5, OffsetY = -1.5

		tfetch2D sampledColor8, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.5, OffsetY =  0.5
		tfetch2D sampledColor9, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.5, OffsetY =  0.5
		tfetch2D sampledColor10, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  0.5, OffsetY =  0.5
		tfetch2D sampledColor11, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  2.5, OffsetY =  0.5

		tfetch2D sampledColor12, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -3.5, OffsetY =  2.5
		tfetch2D sampledColor13, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX = -1.5, OffsetY =  2.5
		tfetch2D sampledColor14, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  0.5, OffsetY =  2.5
		tfetch2D sampledColor15, tex,
                 mapColorSampler, UnnormalizedTextureCoords = true, OffsetX =  2.5, OffsetY =  2.5
	};	*/
	
	/*
	float4 result = sampledColor*sampledColor;
	result += sampledColor1*sampledColor1;
	result += sampledColor2*sampledColor2;
	result += sampledColor3*sampledColor3;
	result += sampledColor4*sampledColor4;
	result += sampledColor5*sampledColor5;
	result += sampledColor6*sampledColor6;
	result += sampledColor7*sampledColor7;
	result += sampledColor8*sampledColor8;
	result += sampledColor9*sampledColor9;
	result += sampledColor10*sampledColor10;
	result += sampledColor11*sampledColor11;
	result += sampledColor12*sampledColor12;
	result += sampledColor13*sampledColor13;
	result += sampledColor14*sampledColor14;
	result += sampledColor15*sampledColor15;*/


	float4 result = sampledColor*0.6;
	result += sampledColor1*0.8;
	result += sampledColor2*0.8;
	result += sampledColor3*0.6;
	
	result += sampledColor4*0.8;	
	result += sampledColor5;
	result += sampledColor6;
	result += sampledColor7*0.8;
	
	result += sampledColor8*0.8;	
	result += sampledColor9;
	result += sampledColor10;
	result += sampledColor11*0.8;
	
	result += sampledColor12*0.8;	
	result += sampledColor13*0.6;
	result += sampledColor14*0.6;
	result += sampledColor15*0.8;

	//result *= hdrProperties;
	//return result*result*0.15625f;						// Premultiplied to keep better quality
	
	//return result*(hdrProperties*(0.3125f*0.5f));			// Premultiplied to keep better quality
	return result*(hdrProperties*0.3125f*0.25f);					// Premultiplied to keep better quality
}


technique Basic
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


