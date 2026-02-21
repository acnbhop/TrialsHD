
float fogStart;
float fogLengthInv;
float4 fogColor;

float2 zConversion;

//int lightAmount;
float4 lightColor[3];
float4 lightPositionRangeCS[3];
float4x4 lightMatrix[3];

//int lightPointAmount;
float4 lightPointColor[4];
float4 lightPointPositionRangeCS[4];

float3 directionPSSM;
float4x4 matrixPSSM[3];
float3 colorPSSM;
float4 ambientMultiplier;


texture lightMapTexture0;
texture lightMapTexture1;
texture lightMapTexture2;
//texture lightMapTexture3;
sampler lightMapTextureSampler[3] = 
{
	sampler_state
	{
		Texture = <lightMapTexture0>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture1>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	,
	sampler_state
	{
		Texture = <lightMapTexture2>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}
	/*,
	sampler_state
	{
		Texture = <lightMapTexture3>;
		AddressU = Clamp;
		AddressV = Clamp;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = POINT;
	}*/
};


texture gbufferMotionVectors;
sampler gbufferMotionVectorsSampler = 
sampler_state
{
	Texture = <gbufferMotionVectors>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
};


textureCUBE lightIndexTexture;
samplerCUBE lightIndexTextureSampler = 
sampler_state
{
    Texture = <lightIndexTexture>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
};


texture gbufferDepth;
sampler gbufferDepthSampler = 
sampler_state
{
	Texture = <gbufferDepth>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


texture gbufferColor;
sampler gbufferColorSampler = 
sampler_state
{
	Texture = <gbufferColor>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


texture gbufferNormal;
sampler gbufferNormalSampler = 
sampler_state
{
	Texture = <gbufferNormal>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    float3 tex2					: TEXCOORD0;
};


VS_OUTPUT VS( float4 position : POSITION0, float2 texcoords : TEXCOORD0, float3 texcoords2 : TEXCOORD1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = position;
    outStruct.tex2 = texcoords2;
    return outStruct;
}




// --------------------------------------------------------------------------------------------------------------------------
// First pass main lighting shaders
// --------------------------------------------------------------------------------------------------------------------------


float4 PS04( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 3
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS14( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 3
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS24( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 3
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS34( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 3
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS03( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS13( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS23( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS33( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS02( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS12( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS22( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS32( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS01( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS11( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS21( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS31( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightendFog.inc"
}


float4 PS00( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
	
	// Pointlights
	
#include "shaders\NBlightendFog.inc"
}


float4 PS10( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
	
#include "shaders\NBlightendFog.inc"
}


float4 PS20( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
	
#include "shaders\NBlightendFog.inc"
}


float4 PS30( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstart.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
	
#include "shaders\NBlightendFog.inc"
}




// -------------------------------------------------------------------------------------------------------


float4 APS03( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS13( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS23( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 2
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS02( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS12( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS22( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS01( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS11( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS21( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
#define LINDEX 0
#include "shaders\NBpointlight.inc"
#undef LINDEX
		
#include "shaders\NBlightend.inc"
}


float4 APS10( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
	
#include "shaders\NBlightend.inc"
}


float4 APS20( float2 tex : VPOS, float3 tex2 : TEXCOORD0 ) : COLOR0
{
#include "shaders\NBlightstartADD.inc"

	// Spotlights
#define LINDEX 0
#include "shaders\NBspotlight.inc"
#undef LINDEX
#define LINDEX 1
#include "shaders\NBspotlight.inc"
#undef LINDEX
	
	// Pointlights
	
#include "shaders\NBlightend.inc"
}





// -------------------------------------------------------------------------------------------------------



technique Basic00
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS00();
    }
}

technique Basic01
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS01();
    }
}

technique Basic02
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS02();
    }
}

technique Basic03
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS03();
    }
}

technique Basic04
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS04();
    }
}

technique Basic10
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS10();
    }
}

technique Basic11
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS11();
    }
}

technique Basic12
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS12();
    }
}

technique Basic13
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS13();
    }
}

technique Basic14
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS14();
    }
}

technique Basic20
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS20();
    }
}

technique Basic21
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS21();
    }
}

technique Basic22
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS22();
    }
}

technique Basic23
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS23();
    }
}

technique Basic24
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS24();
    }
}

technique Basic30
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS30();
    }
}

technique Basic31
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS31();
    }
}

technique Basic32
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS32();
    }
}

technique Basic33
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS33();
    }
}

technique Basic34
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS34();
    }
}



// --------------------------------------------------------------------------------------------------------------------------
// Second pass shaders (additive, no ambient, no sunlight, etc)
// --------------------------------------------------------------------------------------------------------------------------


technique Basic01A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS01();
    }
}

technique Basic02A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS02();
    }
}

technique Basic03A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS03();
    }
}

technique Basic10A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS10();
    }
}

technique Basic11A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS11();
    }
}

technique Basic12A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS12();
    }
}

technique Basic13A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS13();
    }
}

technique Basic20A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS20();
    }
}

technique Basic21A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS21();
    }
}

technique Basic22A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS22();
    }
}

technique Basic23A
{
    pass P0
    {
        VertexShader  = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 APS23();
    }
}

