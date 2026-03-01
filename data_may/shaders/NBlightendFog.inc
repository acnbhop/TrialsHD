
	float3 lightAcc = lightAccDiff * texColor.rgb + lightAccSpec*3;

	// Fog
	float fogFactor = saturate((length(pixelPositionVS) - fogStart) * fogLengthInv);
	return float4(lerp(lightAcc*2, fogColor.rgb, fogFactor), 1);

