
	{
		float4 posLMap = mul(float4(pixelPositionVS, 1), lightMatrix[LINDEX]);
				
		//if (posLMap.z > 0)
		{			
			float3 texLmap = posLMap.xyz;
			posLMap.xyz /= posLMap.w;

			if (!any(floor(posLMap.xyz)))
			{
				float4 lightPosRan = lightPositionRangeCS[LINDEX];
			
				float3 lightDirection = lightPosRan.xyz - pixelPositionVS;
				float range = length(lightDirection);
				float lrange = lightPosRan.w - range;
				lightDirection = lightDirection / range;
				float invRange = 1.0f / lightPosRan.w;
				float brightness = saturate(lrange * invRange);
				
				// Sample projectors and depth map simultaneously (optimized version)
				texLmap.xy = posLMap.xy;
				texLmap.z *= invRange;
				//float3 lightMul = tex2D(lightMaskTextureSampler, texLmap.xy).xyz;
				float2 shadBuffer = tex2D(lightMapTextureSampler[LINDEX], texLmap.xy).xy;

				brightness *= shadBuffer.y * 32 * saturate(exp((5.0f + 80.0f * brightness) * ( shadBuffer.x - texLmap.z )));		// ESM with fake distance falloff soft shadows
						 
				float3 specHalfVector = normalize(lightDirection - pixelDirection);

				float diffuse = saturate(dot(texNormal2, lightDirection));
				float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
				float3 lightMul = lightColor[LINDEX].rgb * brightness;
				lightAccDiff += lightMul * diffuse;
				lightAccSpec += lightMul * specular;
			}
		}
	}
