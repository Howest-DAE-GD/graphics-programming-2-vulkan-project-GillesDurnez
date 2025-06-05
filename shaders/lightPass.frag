#version 450

layout(location = 0) in vec4 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gDiffuse;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gMR; // R = metallic, G = roughness
layout(binding = 3) uniform sampler2D gDepth;

layout(set= 1, binding = 0) uniform UniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 cameraPosition;
} ubo;

vec3 gLightDirection = normalize(vec3(0.577, -0.577, 0.577));
const float LIGHT_INTENSITY = 7.0;
const vec3 LIGHT_COLOR = vec3(1.0);

const float PI = 3.14159265359;

vec3 Lambert(float kd, vec3 cd)
{
	return vec3( cd * kd / PI );
}

vec3 Lambert(vec3 kd, vec3 cd)
{
	return vec3( cd * kd / PI );
}

vec3 FresnelSchlick(vec3 h, vec3 v, vec3 f0)
{
	vec3 f =  f0 + (vec3( 1, 1, 1 ) - f0) * pow((1 - dot(h, v)), 5.f);
	return f;
}

float NormalDistributionGGX(vec3 n, vec3 h, float roughness)
{
	float alpha = roughness * roughness;
	float cosf = dot(n,h);
	float tempBottom = cosf * cosf * (alpha * alpha - 1) + 1;
	float d = (alpha * alpha) / (PI * (tempBottom * tempBottom));

	return d;
}

float GeometrySchlickGGX(vec3 n, vec3 v, float roughness)
{
	float alpha = roughness * roughness;
	float k = (alpha + 1) * (alpha + 1) / 8;
	float cosf = max(dot(n,v), 0.f);
	float g = cosf / (cosf * (1 - k) + k);
	return g;
}

float GeometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
	float g = GeometrySchlickGGX(n, v, roughness) * GeometrySchlickGGX(n, l, roughness);
	return g;
}

vec3 GetWorldPositionFromDepth(float depth, ivec2 fragCoords, mat4 invProj, mat4 invView, ivec2 resolution) 
{
    vec2 ndc;
    ndc.x = (float(fragCoords.x) / float(resolution.x)) * 2.0 - 1.0;
    ndc.y = (float(fragCoords.y) / float(resolution.y)) * 2.0 - 1.0;

    vec4 clipPos = vec4(ndc, depth, 1.0);
    vec4 viewPos = invProj * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = invView * viewPos;
    return worldPos.xyz;
}

void main()
{
	vec3 l = gLightDirection;
	float depth = texture(gDepth, fragTexCoord).r;
	mat4 invProj = inverse(ubo.proj);
	mat4 invView = inverse(ubo.view);
	vec3 v = normalize(ubo.cameraPosition - GetWorldPositionFromDepth(depth, ivec2(fragTexCoord), invProj, invView, textureSize(gDepth, 0) ));

	vec4 albedo = texture(gDiffuse, fragTexCoord);
	vec3 normal = texture(gNormal, fragTexCoord).rgb * 2.0 - 1.0;
	float roughness = texture(gMR, fragTexCoord).g;
	float metallic = texture(gMR, fragTexCoord).r;

	vec3 h = normalize(l - v);
	vec3 f;
	vec3 diffuse;

	float d = NormalDistributionGGX(normal, h, roughness);

	if (metallic == 1.0f)
	{
		f = FresnelSchlick(h, -v, albedo.rgb);
		diffuse = Lambert(vec3( 0,0,0 ), albedo.rgb);
	}
	else
	{
		f = FresnelSchlick(h, -v, vec3( 0.04f, 0.04f, 0.04f ));
		vec3 kd = vec3(1,1,1) - f;
		diffuse = Lambert(kd, albedo.rgb);
	}
	float g = GeometrySmith(normal, -v, l, roughness);
	vec3 specular = d * f * g / (4 * dot(-v, normal) * dot(l, normal));

	outColor = vec4(specular + diffuse, 1);
}



//vec3 fresnelSchlick(float cosTheta, vec3 F0) 
//{
//    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}
//
//float distributionGGX(vec3 N, vec3 H, float roughness) 
//{
//    float a = roughness * roughness;
//    float a2 = a * a;
//    float NdotH = max(dot(N, H), 0.0);
//    float NdotH2 = NdotH * NdotH;
//
//    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//    denom = PI * denom * denom;
//
//    return a2 / max(denom, 0.001);
//}
//
//float geometrySchlickGGX(float NdotV, float roughness) 
//{
//    float r = roughness + 1.0;
//    float k = (r * r) / 8.0;
//
//    return NdotV / (NdotV * (1.0 - k) + k);
//}
//
//float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) 
//{
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
//    float ggx1 = geometrySchlickGGX(NdotV, roughness);
//    float ggx2 = geometrySchlickGGX(NdotL, roughness);
//    return ggx1 * ggx2;
//}
//
//void main() 
//{
//    vec3 albedo = pow(texture(gDiffuse, fragTexCoord).rgb, vec3(2.2));
//    vec3 mrSample = texture(gMR, fragTexCoord).rgb;
//    float metallic = mrSample.r;
//    float roughness = clamp(mrSample.g, 0.05, 1.0);
//
//    // Tangent space basis
//    vec3 N = normalize(fragNormal);
//    vec3 T = normalize(fragTangent);
//    vec3 B = normalize(cross(N, T));
//    mat3 TBN = mat3(T, B, N);
//
//    // Sample and transform normal from normal map
//    vec3 normalMap = texture(gNormal, fragTexCoord).rgb;
//    vec3 Nmap = normalize(normalMap * 2.0 - 1.0);
//    vec3 Nworld = normalize(TBN * Nmap);
//
//    vec3 fragPos = fragPosition.xyz;
//    vec3 camPos = inverse(ubo.view)[3].xyz;
//    vec3 V = normalize(camPos - fragPos);
//    vec3 L = normalize(-gLightDirection);
//    vec3 H = normalize(V + L);
//
//    // Dot products
//    float NdotL = max(dot(Nworld, L), 0.0);
//    float NdotV = max(dot(Nworld, V), 0.0);
//    float NdotH = max(dot(Nworld, H), 0.0);
//    float VdotH = max(dot(V, H), 0.0);
//
//    // Fresnel reflectance at normal incidence
//    vec3 F0 = mix(vec3(0.04), albedo, metallic);
//    vec3 F = fresnelSchlick(VdotH, F0);
//
//    // Compute BRDF terms
//    float D = distributionGGX(Nworld, H, roughness);
//    float G = geometrySmith(Nworld, V, L, roughness);
//
//    // Cook–Torrance specular
//    vec3 numerator = D * G * F;
//    float denominator = 4.0 * NdotV * NdotL + 0.001;
//    vec3 specular = numerator / denominator;
//
//    // Diffuse
//    vec3 kd = (1.0 - F) * (1.0 - metallic);
//    vec3 diffuse = kd * albedo / PI;
//
//    vec3 radiance = LIGHT_COLOR * LIGHT_INTENSITY;
//    vec3 Lo = (diffuse + specular) * radiance * NdotL;
//
//    // Final output (gamma correct)
//    vec3 color = pow(Lo, vec3(1.0 / 2.2));
//    outColor = vec4(color, 1.0);
//}


//#version 450
//layout(location = 0) in vec4 fragPosition;
//layout(location = 1) in vec3 fragNormal;
//layout(location = 2) in vec3 fragTangent;
//layout(location = 3) in vec2 fragTexCoord;
//
//layout(location = 0) out vec4 outColor;
//
//layout(binding = 0) uniform sampler2D gDiffuse;
//layout(binding = 1) uniform sampler2D gNormal;
//layout(binding = 2) uniform sampler2D gMR;   // metal+rough
//
//
//layout(binding = 1) uniform UniformBufferObject 
//{
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//} ubo;
//
//
//vec3 gLightDirection = { 0.577f, -0.577f, 0.577f };
//
//const float PI                  = 3.14159265358979323846f;
//const float LIGHT_INTENSITY     = 7.0f;
//
//void main() 
//{
//    vec4 albedo = texture(gDiffuse, fragTexCoord);
//    vec3 normal_s = texture(gNormal, fragTexCoord).rgb * 2.0 - 1.0;
//    vec3 N = normalize(normal_s);
//
//    vec3 diffuseColor = albedo.rgb;
//    
//    vec3 binormal = cross(fragNormal, fragTangent);
//    mat4x4 tangentSpaceAxis = 
//    {
//        vec4(fragTangent.x, fragTangent.y, fragTangent.z, 0), 
//        vec4(binormal.x, binormal.y, binormal.z, 0), 
//        vec4(fragNormal.x, fragNormal.y, fragNormal.z, 0), 
//        vec4(0, 0, 0, 1)
//    };
//    
//    // Sampled Normals
//    vec3 normalMap = texture(gNormal, fragTexCoord).rgb;
//    vec3 normalMapnormal = 2 * normalMap - vec3(1, 1, 1);
//    normalMapnormal = normalize(normalMapnormal * mat3x3(tangentSpaceAxis));
//
//    // Observed Area
//    float observedArea = dot(-normalize(gLightDirection), normalMapnormal);
//    observedArea = clamp(observedArea, 0, 1);
//    
//
//
//    outColor = vec4(albedo);
//}
//