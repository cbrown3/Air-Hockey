
// Constant Buffer
// - Allows us to define a buffer of individual variables 
//    which will (eventually) hold data from our C++ code
// - All non-pipeline variables that get their values from 
//    our C++ code must be defined inside a Constant Buffer
// - The name of the cbuffer itself is unimportant
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	
	matrix shadowViewMat;
	matrix shadowProjMat;
	matrix CubeShadowProjMat;
	matrix CubeShadowViewMat1;
	matrix CubeShadowViewMat2;
	matrix CubeShadowViewMat3;
	matrix CubeShadowViewMat4;
	matrix CubeShadowViewMat5;
	matrix CubeShadowViewMat6;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{ 
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 position		: POSITION;     // XYZ position
	float2 uv           : TEXCOORD;     // UV
	float3 normal		: NORMAL;       // Normal
	float3 tangent		: TANGENT;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float2 uv           : TEXCOORD;     // UV
	float3 normal		: NORMAL;       // Normal
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float4 shadowMapPosition : POSITION1;
	float4 CubeShadowMapPosition : POSITION2;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	//positions for output
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	
	//shadow positions for output
	matrix shadowWVP = mul(mul(world, shadowViewMat), shadowProjMat);
	output.shadowMapPosition = mul(float4(input.position, 1.0f), shadowWVP);

	matrix CShadowWVP1 = mul(mul(world, CubeShadowViewMat1), shadowProjMat);
	matrix CShadowWVP2 = mul(mul(world, CubeShadowViewMat2), shadowProjMat);
	matrix CShadowWVP3 = mul(mul(world, CubeShadowViewMat3), shadowProjMat);
	matrix CShadowWVP4 = mul(mul(world, CubeShadowViewMat4), shadowProjMat);
	matrix CShadowWVP5 = mul(mul(world, CubeShadowViewMat5), shadowProjMat);
	matrix CShadowWVP6 = mul(mul(world, CubeShadowViewMat6), shadowProjMat);
	
	output.shadowMapPosition = mul(float4(input.position, 1.0f), shadowWVP);
	output.shadowMapPosition = mul(float4(input.position, 1.0f), shadowWVP);
	output.shadowMapPosition = mul(float4(input.position, 1.0f), shadowWVP);
	output.shadowMapPosition = mul(float4(input.position, 1.0f), shadowWVP);
	output.shadowMapPosition = mul(float4(input.position, 1.0f), shadowWVP);
	output.shadowMapPosition = mul(float4(input.position, 1.0f), shadowWVP);





	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal); // Make sure it's length is 1

	// Make sure the tangent is in WORLD space and a unit vector
	output.tangent = normalize(mul(input.tangent, (float3x3)world));

	output.uv = input.uv;
	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;


	return output;
}