
	{
		float4 lightPosRan = lightPointPositionRangeCS[LINDEX];
		
		float3 lightDirection = lightPosRan.xyz - pixelPositionVS;
		float rangeSQ = dot(lightDirection, lightDirection);
		if (rangeSQ < lightPosRan.w*lightPosRan.w)
		{
			float range = sqrt(rangeSQ);
			float lrange = lightPosRan.w - range;
		
			lightDirection = lightDirection / range;
			float brightness = saturate(lrange / lightPosRan.w);
			
			float3 specHalfVector = normalize(lightDirection - pixelDirection);

			float diffuse = saturate(dot(texNormal2, lightDirection));
			float specular = pow(saturate(dot(texNormal2, specHalfVector)), texNormalSpec.w * 200.0) * texNormalSpec.z;
			float3 lightMul = lightPointColor[LINDEX].rgb * brightness;
			lightAccDiff += lightMul * diffuse;
			lightAccSpec += lightMul * specular;
		}
	}
