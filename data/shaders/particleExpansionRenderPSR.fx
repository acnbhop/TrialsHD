
float aspectRatio = 1.0;
//float4 hdrProperties;

const float2 quadCorners[4] = { float2( -1, -1 ), float2( 1, -1 ), float2( 1, 1 ), float2( -1, 1 ) };
float4 fogColor;

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
    float4 lightDirZ			: TEXCOORD2;
};


VS_OUTPUT VS(int index : INDEX)
{
	// Vfetch data. Four adjacent vertices fetch same input data (particle quad corners), but expand it differently: [-1,-1]; [1,-1]; [1,1]; [-1,1]
	int vertexIndex = index/4;
	float4 posCenterXYZRad;
	float4 directionXYZRad;
	float4 lightAccumulatorXYZAlpha;
	float4 lightAvgDirectionXYZFrame;
	asm
	{
		vfetch posCenterXYZRad, vertexIndex, position0
		vfetch directionXYZRad, vertexIndex, position1
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
	
	posCenterXYZRad.z = 1.0 - posCenterXYZRad.z;						// 16 bit float z value precision improvement

	VS_OUTPUT outStruct;
	
	// Pass though same vertex lighting properties for each quad corner
	outStruct.lightColorAlpha = lightAccumulatorXYZAlpha;
	outStruct.lightDirZ = float4(lightAvgDirectionXYZFrame.xyz, 1.0f - pow(posCenterXYZRad.z, 48.0f));

	// Calculate quad expansion factors from vertex index
	int indexMod = index - vertexIndex*4;
	float2 expand = quadCorners[indexMod];
	//int indexD2 = index / 2;
	//int expandY = indexD2 - vertexIndex*2;
	//int expandX = index - indexD2*2;
	
	// Output vertex texture coordinate
	outStruct.tex = texCoordRectangle.xy + texCoordRectangle.zw * (expand * float2(0.5, -0.5) + 0.5);
	
	// Do quad expansion for vertex
	// Combined expander for point particles, rotated particles and stretched particles
	float2 sideVec = normalize(float2(directionXYZRad.y, -directionXYZRad.x)) * lerp(posCenterXYZRad.w, directionXYZRad.w, expand.y * 0.5 + 0.5);
	float3 expandAdd = directionXYZRad.xyz * expand.y + float3(sideVec * expand.x, 0);
	//expandAdd.y *= -aspectRatio;
	expandAdd.y *= aspectRatio;
	outStruct.pos = float4(posCenterXYZRad.xyz + expandAdd, 1);
	
    return outStruct;
}


float4 PS( VS_OUTPUT vsInput ) : COLOR0
{
	float4 tex1 = tex2D(mapColorSampler, vsInput.tex);
	float4 tex2 = tex2D(mapNormalHeightAmbientSampler, vsInput.tex);
		
	float4 lightColorAlpha = float4(vsInput.lightColorAlpha.xyz,1) * vsInput.lightColorAlpha.w;
	lightColorAlpha.xyz += tex2.z;
	lightColorAlpha.w *= tex2.x; // texAlpha * partAlpha... and for premultiplied alpha: texRGB * partAlpha
	lightColorAlpha.xyz *= tex1.xyz;// * hdrProperties.x;
	
	if (dot(vsInput.lightDirZ.xyz, vsInput.lightDirZ.xyz) != 0)
	{
		float3 normal = float3(-(tex1.w*2-1), tex2.w*2-1, 0);
		normal.z = sqrt(1 - normal.x*normal.x - normal.z*normal.z);	
		lightColorAlpha.xyz *= (dot(normal, vsInput.lightDirZ.xyz) + 1.0f) * 0.5f;
	}
	
	//return lightColorAlpha;

	//float fogFactor = pow(vsInput.lightDirZ.w, 48.0f);
	//float alphaMult = (1.0f - fogFactor);
	lightColorAlpha *= vsInput.lightDirZ.w;
	return lightColorAlpha;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}


