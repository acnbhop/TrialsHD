
texture mapColor;
sampler mapColorSampler = 
sampler_state
{
	Texture = <mapColor>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};


texture mapNormalHeightAmbient;
sampler mapNormalHeightAmbientSampler = 
sampler_state
{
    Texture = <mapNormalHeightAmbient>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    float2 tex					: TEXCOORD0;
    float4 lightColorAlpha		: TEXCOORD1;
    float3 lightDir				: TEXCOORD2;
};


VS_OUTPUT VS( float3 position : POSITION0, float4 color : COLOR0, float2 texcoords : TEXCOORD0, float3 lightDir : POSITION1 )
{
    VS_OUTPUT outStruct;
    outStruct.pos = float4(position, 1);
    outStruct.tex  = texcoords;
    outStruct.lightColorAlpha  = color;
    outStruct.lightDir = lightDir;
    return outStruct;
}


float4 PS( VS_OUTPUT vsInput ) : COLOR0
{
	float4 tex1 = tex2D(mapColorSampler, vsInput.tex);
	float4 tex2 = tex2D(mapNormalHeightAmbientSampler, vsInput.tex);
	
	float3 normal = float3(tex1.w*2-1, tex2.w*2-1, 0);
	normal.z = sqrt(1 - normal.x*normal.x - normal.z*normal.z);
	
	float4 lightColorAlpha = float4(vsInput.lightColorAlpha.xyz,1)*vsInput.lightColorAlpha.w;
	lightColorAlpha.xyz += tex2.z;
	lightColorAlpha.w *= tex2.x; // texAlpha * partAlpha... and for premultiplied alpha: texRGB * partAlpha
	lightColorAlpha.xyz *= tex1.xyz;
	lightColorAlpha.xyz *= saturate(dot(normal, vsInput.lightDir));
	
	return lightColorAlpha;

	//float4 texCol = tex2D(mapColorSampler, vsInput.tex) * vsInput.lightColorAlpha;
	//texCol.rgb *= vsInput.lightColorAlpha.a;	// texture has premultiplied alpha, and blending is expecting it
	//return texCol;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}


