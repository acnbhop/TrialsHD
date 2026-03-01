
float aspectRatio = 1.0;

const float2 quadCorners[4] = { float2( -1, -1 ), float2( 1, -1 ), float2( 1, 1 ), float2( -1, 1 ) };


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


texture mapNormalHeightAmbient;
sampler mapNormalHeightAmbientSampler = 
sampler_state
{
    Texture = <mapNormalHeightAmbient>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
};


struct VS_OUTPUT
{
    float4 pos					: POSITION;
    float2 tex					: TEXCOORD0;
    float4 lightColorAlpha		: TEXCOORD1;
    float3 lightDir				: TEXCOORD2;
};


VS_OUTPUT VS(int index : INDEX)
{
	// Vfetch data. Four adjacent vertices fetch same input data (particle quad corners), but expand it differently: [-1,-1]; [1,-1]; [1,1]; [-1,1]
	int vertexIndex = index/4;
	float4 pos1ScreenXYZRad;
	float4 pos2ScreenXYZRad;
	float4 lightAccumulatorXYZAlpha;
	float4 lightAvgDirectionXYZFrame;
	asm
	{
		vfetch pos1ScreenXYZRad, vertexIndex, position0
		vfetch pos2ScreenXYZRad, vertexIndex, position1
		vfetch lightAccumulatorXYZAlpha, vertexIndex, position2
		vfetch lightAvgDirectionXYZFrame, vertexIndex, position3
	};

	// Vfetch texture coordinate rectangle from second vertex stream
	float rectangleIndex = lightAvgDirectionXYZFrame.w;
	float4 texCoordRectangle;
	asm
	{
		vfetch texCoordRectangle, rectangleIndex, position4
	};
	
	pos1ScreenXYZRad.z = 1.0 - pos1ScreenXYZRad.z;						// 16 bit float z value precision improvement

	VS_OUTPUT outStruct;
	
	// Pass though same vertex lighting properties for each quad corner
	outStruct.lightColorAlpha = lightAccumulatorXYZAlpha;
	outStruct.lightDir = lightAvgDirectionXYZFrame.xyz;

	// Calculate quad expansion factors from vertex index
	int indexMod = index - vertexIndex*4;
	float2 expand = quadCorners[indexMod];
	//int indexD2 = index / 2;
	//int expandY = indexD2 - vertexIndex*2;
	//int expandX = index - indexD2*2;
	
	// Output vertex texture coordinate
	outStruct.tex = texCoordRectangle.xy + texCoordRectangle.zw * (expand * 0.5 + 0.5);
	
	// Do quad expansion for vertex (simple point particle)
	outStruct.pos = float4(pos1ScreenXYZRad.xyz + float3(pos1ScreenXYZRad.w*expand.x, -pos1ScreenXYZRad.w*expand.y*aspectRatio, 0), 1);
	
    return outStruct;
}


float4 PS( VS_OUTPUT vsInput ) : COLOR0
{
	float4 tex1 = tex2D(mapColorSampler, vsInput.tex);
	float4 tex2 = tex2D(mapNormalHeightAmbientSampler, vsInput.tex);
	
	float3 normal = float3(-(tex1.w*2-1), tex2.w*2-1, 0);
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
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}


