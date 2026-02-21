
	float3 lightAcc = lightAccDiff * texColor.rgb + lightAccSpec*3;

	// Fog already added in first pass, now add only non-fogged part
	float fogFactorInv = 1.0f - saturate((length(pixelPositionVS) - fogStart) * fogLengthInv);
	return float4(lightAcc * (2 * fogFactorInv), 1);

