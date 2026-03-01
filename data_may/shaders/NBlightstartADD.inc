
	// Deferred lighting shader
    float4 texDepth;					// Depth
    float4 texColor;					// color.rgb, emissive
    float4 texNormalSpec;				// normal.xy, specular, glossiness
    
    asm 
    {
		tfetch2D texDepth, tex,
                 gbufferDepthSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D texColor, tex,
                 gbufferColorSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5

		tfetch2D texNormalSpec, tex,
                 gbufferNormalSampler, UnnormalizedTextureCoords = true, OffsetX = 0.5, OffsetY = 0.5
	};
	
	// Convert texDepth to view space
	float pixelDepthVS = zConversion.y / ((1.0f - texDepth.x) - zConversion.x);
	float3 pixelPositionVS = tex2 * pixelDepthVS;
	
	// Calculate normal vector from two channel input
	float3 texNormal2;
	texNormal2.xy = (texNormalSpec.xy * 2) - 1;
	texNormal2.z = -sqrt(1 - texNormal2.x*texNormal2.x - texNormal2.y*texNormal2.y);
	
	texNormalSpec.zw *= 2.0f;	// Stored specularity and glossiness in [0,2] range. Multiply back
	
	float3 pixelDirection = normalize(tex2);
	
	float3 lightAccDiff = float3(0, 0, 0);
	float3 lightAccSpec = float3(0, 0, 0);

