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
		
		// Sample light depth map
		float lightZ = tex2D(gbufferMotionVectorsSampler, texLmap.xy).x;	// Only depth for semi transparent surfaces (no projectors!)
		float3 lightMul2 = 1.0f;
		//float3 lightMul = lightMul2 * (lightZ <= (1.0f - texLmap.z));

		float lZC = (0.5f + lightZ * 0.5f);
		float lZR = texLmap.z;
		float3 lightMul = saturate(exp(120.0f * ( lZC*lZC - lZR*lZR )));		// ESM with squared depth range
		
		lightMul2 = (lightMul2 * 0.6f + 0.25f) * texNCombine2.b;		
		
		float3 specHalfVector = normalize(directionPSSM - pixelDirection);

		float diffuse = saturate(dot(texNormal2, directionPSSM)) * materialPropertiesM.y;// * texCombine.r;
		float diffuse2 = saturate(-dot(texNormal2, directionPSSM)+1.6f) * materialPropertiesM.y;// * texCombine.r;	// backside light
		float specular = pow(saturate(dot(texNormal2, specHalfVector)), materialPropertiesM.x * texCombine.b * 200.0) * materialPropertiesM.z * texCombine.g;
		lightMul *= colorPSSM;
		lightMul2 *= colorPSSM;
		//lightAcc += lightMul * materialColor.rgb * colorPSSM * (texNCombine1.rgb * diffuse + specular);
		lightAccDiff += lightMul * diffuse + lightMul2 * diffuse2;
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
	