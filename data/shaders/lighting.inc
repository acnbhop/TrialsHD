	
	// Directional sun light (PSSM)
	{
		//float3 texLmap = psIn.posPSSM[0].xyz / psIn.posPSSM[0].w;
		//if (any(floor(texLmap))) texLmap = psIn.posPSSM[1].xyz / psIn.posPSSM[1].w;

		float3 texLmap1 = psIn.posPSSM[0].xyz / psIn.posPSSM[0].w;
		float pssmIndex = any(floor(texLmap1));
		float3 texLmap2 = psIn.posPSSM[1].xyz / psIn.posPSSM[1].w;
		pssmIndex += pssmIndex * any(floor(texLmap2));
		float4 posLMap = psIn.posPSSM[pssmIndex];
		//float3 texLmap = posLMap.xyz / posLMap.w;		// TODO: Replace with tex2DProj
		float3 texLmap = posLMap.xyz;
		texLmap.xy /= posLMap.w;
		texLmap.y *= 1.0f/3.0f;
		texLmap.y += (1.0f/3.0f)*pssmIndex;
		
		//return float4(pssmIndex/2, pssmIndex/2, pssmIndex/2, 1);
		
		/*
		// Sample light depth map
		float3 lightMul = tex2D(gbufferMaterialPropertiesSampler, texLmap.xy).xyz;

		float lightZ = tex2D(gbufferMotionVectorsSampler, texLmap.xy).x;		
		lightMul *= (lightZ <= (1.0f - texLmap.z));
		*/
		
		// Sample projectors and depth map simultaneously (optimized version)
		float2 shadBuffer = tex2D(gbufferMotionVectorsSampler, texLmap.xy).xy;

/*
		// 4 sample ESM		
		float2 shadBuffer;
		{
			float4 shadBuffer1;
			float4 shadBuffer2;
			float4 shadBuffer3;
			float4 shadBuffer4;
			asm {
				tfetch2D shadBuffer1, texLmap.xy, gbufferMotionVectorsSampler, OffsetX = -2.0, OffsetY = -2.0;
				tfetch2D shadBuffer2, texLmap.xy, gbufferMotionVectorsSampler, OffsetX =  2.0, OffsetY = -2.0;
				tfetch2D shadBuffer3, texLmap.xy, gbufferMotionVectorsSampler, OffsetX = -2.0, OffsetY =  2.0;
				tfetch2D shadBuffer4, texLmap.xy, gbufferMotionVectorsSampler, OffsetX =  2.0, OffsetY =  2.0;
			};
			//shadBuffer *= 0.25f;
			shadBuffer = (shadBuffer1.xy + shadBuffer2.xy + shadBuffer3.xy + shadBuffer4.xy) * 0.25f;
		}
*/
		
		//float3 lightMul = shadBuffer.y * 32 * ((0.5f + shadBuffer.x * 0.5f) > texLmap.z );
		float lZC = (0.5f + shadBuffer.x * 0.5f);
		float lZR = texLmap.z;
		float3 lightMul = shadBuffer.y * 32 * saturate(exp(120.0f * ( lZC*lZC - lZR*lZR )));		// ESM with squared depth range
		//float3 lightMul = shadBuffer.y * 32 * saturate(exp(60.0f * ( lZC*lZC*lZC - lZR*lZR*lZR )));		// ESM with squared depth range
		
		lightMul += 0.2f;	// Hack: See a little bit though to give ambient light specular highlights
	
		/*
		// PCF filter
		{
			float4 Weights;
			float4 SampledDepth;
			asm {
				tfetch2D SampledDepth.x___, texLmap.xy, gbufferMotionVectorsSampler, OffsetX = -0.5, OffsetY = -0.5;
				tfetch2D SampledDepth._x__, texLmap.xy, gbufferMotionVectorsSampler, OffsetX =  0.5, OffsetY = -0.5;
				tfetch2D SampledDepth.__x_, texLmap.xy, gbufferMotionVectorsSampler, OffsetX = -0.5, OffsetY =  0.5;
				tfetch2D SampledDepth.___x, texLmap.xy, gbufferMotionVectorsSampler, OffsetX =  0.5, OffsetY =  0.5;			
				getWeights2D Weights, texLmap.xy, gbufferMotionVectorsSampler, MagFilter=linear, MinFilter=linear;
			};
			Weights = float4( (1-Weights.x)*(1-Weights.y), Weights.x*(1-Weights.y), (1-Weights.x)*Weights.y, Weights.x*Weights.y );
			//float4 Attenuation = step( SampledDepth, 1.0f - texLmap.z );
			//lightMul *= dot( Attenuation, Weights );
			float4 Attenuation = step( 1.0f - texLmap.z, SampledDepth );
			lightMul = saturate(lightMul - dot( Attenuation, Weights ));
		}
		*/
		
		float3 specHalfVector = normalize(directionPSSM - pixelDirection);

		float diffuse = saturate(dot(texNormal2, directionPSSM)) * materialPropertiesM.y;// * texCombine.r;
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), materialPropertiesM.x * texCombine.b * 200.0) * materialPropertiesM.z * texCombine.g;
		//lightAcc += lightMul * materialColor.rgb * colorPSSM * (texNCombine1.rgb * diffuse + specular);
		lightMul *= colorPSSM;
		lightAccDiff += lightMul * diffuse;
		lightAccSpec += lightMul * specular;
	}
	
	
	// Spotlights
	for (int i=0; i<lightAmount; i++)
	{
		float4 lightPosRan = lightPositionRangeCS[i];
		float4 posLMap = psIn.posLmap[i];
		
		if (posLMap.z > 0)
		{			
			// Sample light depth map
			/*
			float3 texLmap = posLMap.xyz / posLMap.w;
			float lightZ = tex2D(lightMapTextureSampler[i], texLmap.xy).x;
			float3 lightMul = tex2D(lightMaskTextureSampler, texLmap.xy).xyz;
			lightMul *= (lightZ <= (texLmap.z));
			*/
					
			float3 lightDirection = lightPosRan.xyz - vertexPosition;
			float range = length(lightDirection);
			float lrange = lightPosRan.w - range;
			lightDirection = lightDirection / range;
			float invRange = 1.0f / lightPosRan.w;
			float brightness = saturate(lrange * invRange);
			
			// Sample projectors and depth map simultaneously (optimized version)
			float3 texLmap = posLMap.xyz;
			texLmap.xy /= posLMap.w;
			texLmap.z *= invRange;
			float3 lightMul = tex2D(lightMaskTextureSampler, texLmap.xy).xyz;
			float2 shadBuffer = tex2D(lightMapTextureSampler[i], texLmap.xy).xy;

			//lightMul *= saturate(exp(30.0f * ( shadBuffer.x - texLmap.z )));		// ESM
					 
			lightMul *= saturate(exp((5.0f + 80.0f * brightness) * ( shadBuffer.x - texLmap.z )));		// ESM with fake distance falloff soft shadows
					 
			float3 specHalfVector = normalize(lightDirection - pixelDirection);

			float diffuse = saturate(dot(texNormal2, lightDirection)) * materialPropertiesM.y;// * texCombine.r;
			float specular = pow(saturate(dot(texNormal2, specHalfVector)), materialPropertiesM.x * texCombine.b * 200.0) * materialPropertiesM.z * texCombine.g;
			//lightAcc += lightMul * brightness * materialColor.rgb * lightColor[i].rgb * (texNCombine1.rgb * diffuse + specular);
			lightMul *= lightColor[i].rgb * brightness;
			lightAccDiff += lightMul * diffuse;
			lightAccSpec += lightMul * specular;
		}
	}
	
	
	// Pointlights
	for (int i2=0; i2<lightPointAmount; i2++)
	{
		float4 lightPosRan = lightPointPositionRangeCS[i2];
		
		float3 lightDirection = lightPosRan.xyz - vertexPosition;
		float range = length(lightDirection);
		float lrange = lightPosRan.w - range;
		lightDirection = lightDirection / range;
		float brightness = saturate(lrange / lightPosRan.w);
		
		float3 specHalfVector = normalize(lightDirection - pixelDirection);

		float diffuse = saturate(dot(texNormal2, lightDirection)) * materialPropertiesM.y;// * texCombine.r;
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), materialPropertiesM.x * texCombine.b * 200.0) * materialPropertiesM.z * texCombine.g;
		float3 lightMul = lightPointColor[i2].rgb * brightness;
		lightAccDiff += lightMul * diffuse;
		lightAccSpec += lightMul * specular;
	}
	
	lightAccSpec *= 3;
	
	