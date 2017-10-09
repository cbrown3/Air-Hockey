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
	float3 normal		: NORMAL;       // Normal
	float2 uv           : TEXCOORD;     // UV
	float3 worldPos		: POSITION;
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



Texture2D srv : register (t0);
SamplerState basicSampler : register (s0);

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
	
	//Adjust the variables below as necessary to work with your own code
	float4 surfaceColor = srv.Sample(basicSampler, input.uv);

	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	input.normal = normalize(input.normal);

	float dirLightAmount = saturate(dot(input.normal, -light.Direction));
	//dirLightAmount = clamp(dirLightAmount, 0.0f, 0.25f);

	
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

	
	float4 finalLight = (((pLight.AmbientColor + (pLight.DiffuseColor * pLightAmount) + pLightSpec) + (light.AmbientColor + (light.DiffuseColor * dirLightAmount)) + (pLight.AmbientColor + (pLight.DiffuseColor * pLightAmount))) * surfaceColor);

	//return pLight.AmbientColor;
	//return pLight.AmbientColor + (pLightAmount * pLight.DiffuseColor);

	return finalLight;
}