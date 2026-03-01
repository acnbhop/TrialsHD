
float aspectRatio = 1.0;


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
    float2 tex					: TEXCOORD0;
    float4 lightColorAlpha		: TEXCOORD1;
    float3 lightDir				: TEXCOORD2;
};


VS_OUTPUT VS(int index : INDEX)
{
	// Vfetch data. Four adjacent vertices fetch same input data (particle quad corners), but expand it differently: [-1,-1]; [1,-1]; [-1,1]; [1,1]
	float vertexIndex = floor(index/4);	       
	float4 pos1ScreenXYZRad;
	float4 pos2ScreenXYZRad;
	float4 lightAccumulatorXYZAlpha;
	float4 lightAvgDirectionXYZFrame;
	asm
	{
		vfetch pos1ScreenXYZRad, index, position0
		vfetch pos2ScreenXYZRad, index, position1
		vfetch lightAccumulatorXYZAlpha, index, position2
		vfetch lightAvgDirectionXYZFrame, index, position3
	};

	VS_OUTPUT outStruct;
	
	// Pass though same vertex lighting properties for each quad corner
	outStruct.lightColorAlpha = lightAccumulatorXYZAlpha;
	outStruct.lightDir = lightAvgDirectionXYZFrame.xyz;    

	// Calculate quad expansion factors from vertex index
	float3 centerPoint = (pos1ScreenXYZRad.xyz + pos2ScreenXYZRad.xyz) * 0.5;
	float indexD2 = floor(index/2);
	float expandX = index-indexD2*2;
	float expandY = indexD2-vertexIndex*2;
	
	// Output vertex texture coordinate
	outStruct.tex = float2(expandX, expandY);
	
	// Do quad expansion for vertex (streched particle with radius)
	// TODO: Add correct aspect ratio fixed calculation!
	float3 centerVec = pos1ScreenXYZRad.xyz - centerPoint;
	float lengthX = length(centerVec);
	float lenghtXExpanded = (pos1ScreenXYZRad.w + lengthX) / lengthX;
	float3 expandVectorX = centerVec * lenghtXExpanded;
	float3 expandVectorY = normalize(float3(-expandVectorX.y, expandVectorX.x, 0)) * pos1ScreenXYZRad.w;	// calculate perpendicular vector in xy plane
	
	// Output the corner position
	outStruct.pos = float4(centerPoint + expandVectorX*(expandX*2-1) + expandVectorY*(expandY*2-1), 1);
	
    return outStruct;
}


float4 PS( VS_OUTPUT vsInput ) : COLOR0
{
	float4 texCol = tex2D(mapColorSampler, vsInput.tex );
	return texCol * vsInput.lightColorAlpha;
}


technique Basic
{
    pass P0
    {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}


