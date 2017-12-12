// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD;     // UV
	float3 normal		: NORMAL;       // Normal
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float4 shadowMapPosition	:	POSITION1;
	float4 CubeShadowMapPosition	:	POSITION2;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

struct PointLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Position;
};

// Constant Buffer
// - Allows us to define a buffer of individual variables 
//    which will (eventually) hold data from our C++ code
// - All non-pipeline variables that get their values from 
//    our C++ code must be defined inside a Constant Buffer
// - The name of the cbuffer itself is unimportant
cbuffer ExternalData : register(b0)
{
	DirectionalLight light;
	PointLight pLight;

	float3 cameraPosition; //cam position for specular
};



Texture2D srv		:	register(t0);
Texture2D ShadowMap	:	register(t1);
Texture2D NormalMap :	register(t2);
TextureCube ShadowCubeMap :	register(t3);
SamplerState basicSampler : register (s0);
SamplerComparisonState ShadowSampler : register(s1);


// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//Re-normalizing interpolated normals
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	//Unpack normal from texture sample
	float3 unpackedNormal = NormalMap.Sample(basicSampler, input.uv).xyz * 2.0f - 1.0f;

	//Create the TBN matrix (Tangent, Bi-Tangent, Normal)
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Overwrite the existing normal (we've been using for lighting),
	// with the version from the normal map, AFTER we convert to
	// world space
	

	//Adjust the variables below as necessary to work with your own code
	float4 surfaceColor = srv.Sample(basicSampler, input.uv);

	
	
	//Point Light Calculations
	float3 pLightDir = normalize(pLight.Position - input.worldPos);
	float pLightDist = length(pLight.Position - input.worldPos);
	float pLightAmount = saturate(dot(input.normal, pLightDir));
	float atten = 1.0f / (1.0f + .1 * pow(pLightDist, 2.0f)); //Attenuation, so distance matters
	pLightAmount = saturate(pLightAmount * atten);

	//Point Light Specular Calculations
	float3 reflection = reflect(-pLightDir, input.normal);
	float3 dirToCamera = normalize(cameraPosition - input.worldPos);
	float pLightSpec = pow(saturate(dot(reflection, dirToCamera)), 64);
	pLightSpec = saturate(pLightSpec * atten);

	input.normal = normalize(mul(unpackedNormal, TBN));

	float dirLightAmount = saturate(dot(input.normal, -light.Direction));
	//dirLightAmount = clamp(dirLightAmount, 0.0f, 0.25f);
	

	//SHADOW STUFF

	float2 shadowUV = input.shadowMapPosition.xy / input.shadowMapPosition.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;

	float depthFromLight = input.shadowMapPosition.z / input.shadowMapPosition.w;

	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);

	//return float4(shadowAmount, 0, 0, 1);


	//CUBE MAP SHADOW STUFF
	float4 distanceVec = float4 (pLight.Position, 1) - float4(input.worldPos, 1);
	depthFromLight = saturate(length(distanceVec.xyz));
	distanceVec.xyz = normalize(distanceVec.xyz);

	float shadowDepth = ShadowMap.Sample(basicSampler, -distanceVec.xyz).x / ShadowMap.Sample(basicSampler, -distanceVec.xyz);
	//float shadowDepthLength = length(shadowDepth.xyz);
	float shadow2Amount = depthFromLight < shadowDepth;
	
	//float4 shadow2Amount = ShadowCubeMap.Sample(basicSampler, input.position.xyz);
	//return shadow2Amount;
	//return float4(shadowAmount, shadow2Amount, shadowAmount, 1);

	float4 finalLight = shadowAmount * (((pLight.AmbientColor + (pLight.DiffuseColor * pLightAmount) + pLightSpec) + (light.AmbientColor + (light.DiffuseColor * dirLightAmount)) + (pLight.AmbientColor + (pLight.DiffuseColor * pLightAmount))) * surfaceColor);

	//return pLight.AmbientColor;
	//return pLight.AmbientColor + (pLightAmount * pLight.DiffuseColor);

	return finalLight;
}

/*/
LightFuncOutput LightPointSH(float3 inObjPos, float3 inNormal, float3 inCam2Vertex) {
	lightFuncOutput output; 
	output.diffuseResult = float4(0.0f, 0.0f, 0.0f, 1.0f); 
	output.specularResult = float4(0.0f, 0.0f, 0.0f, 1.0f)
		float4 PLightDirection = 0.0f; 
		PLightDirection.xyz = lightPosition.xyz - inObjPos;
		
		//inObjPos is the pixel's position in world space 
		float distance = length(PLightDirection.xyz); 
		
		//the depth of current pixel 
		PLightDirection.xyz = PLightDirection.xyz / distance; 
		
		//compute attenuation factor 
		PLightDirection.w = max(0, 1 / (lightAttenuation.x + lightAttenuation.y * distance + lightAttenuation.z * distance * distance) );
		
		//sample the cubic shadow map using the inverse of light direction 
		float shadowMapDepth = texCUBE(cubeShadowMapSampler, float4(-(PLightDirection.xyz), 0.0f)).x; 
		
		//do the depth comparison 
		if(distance > shadowMapDepth) {
		return output;
		//the pixel is in shadow so only the ambient light is visible to eye 
		} 
		else { 
			
			//the pixel is not in shadow so the phong lighting is applied 
			float3 floatVecTmp = normalize(inCam2Vertex + PLightDirection.xyz (;
			output.diffuseResult = PLightDirection.w * lightDiffuse * max(0, dot(inNormal, PLightDirection.xyz));
			
			output.specularResult = PLightDirection.w * lightSpecular * pow(max (0, dot(inNormal, floatVecTmp) ), specPower); return output; } } //And finally the vertex and pixel shader of this step is as follows: 
VS_OUTPUT cubicShadowMapping_VS(float4 inPosition : POSITION, float3 inNormal : NORMAL) { VS_OUTPUT output; float4 positionW = mul(inPosition, worldMat); output.cam2Vert = (eyePosition - positionW).xyz; output.position = mul(inPosition, worldViewProjMat); output.worldPos = positionW.xyz; output.normalW = mul(inNormal, worldMat).xyz; return output; } float4 cubicShadowMapping_PS(VS_OUTPUT In) : COLOR0 { lightFuncOutput lightResult; float3 normal = normalize(In.normalW); float3 cam2Vert = normalize(In.cam2Vert); lightResult = LightPointSH(In.worldPos, normal, cam2Vert); float4 ambient = materialAmbient * globalAmbient; float4 diffuse = materialDiffuse * lightResult.diffuseResult; float4 specular = materialSpecular * lightResult.specularResult; float4 lightingColor = (ambient + (diffuse + specular)); return lightingColor; 


*/

