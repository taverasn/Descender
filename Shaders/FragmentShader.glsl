#version 430 // GLSL 4.30
out vec4 Pixel;
// an ultra simple glsl fragment shader

#define MAX_INSTANCE_PER_DRAW 1000

struct OBJ_ATTRIBUTES
{
	vec3 Kd; // diffuse reflectivity
	float d; // dissolve (transparency) 
	vec3 Ks; // specular reflectivity
	float Ns; // specular exponent
	vec3 Ka; // ambient reflectivity
	float sharpness; // local reflection map sharpness
	vec3 Tf; // transmission filter
	float Ni; // optical density (index of refraction)
	vec3 Ke; // emissive reflectivity
	uint illum; // illumination model
};

layout(row_major) uniform UboData
{
	vec4 sunDirection, sunColor, sunAmbient, camPos;
	mat4 viewMatrix, projectionMatrix;
	mat4 worldMatrix;
	OBJ_ATTRIBUTES material;
} UBO_Data;


struct Vertex {
    vec3 pos;
    vec3 uvw;
    vec3 nrm;
};

in Vertex vert;

void main() 
{	
	// Set up Light Ratio
	vec3 lightDir = UBO_Data.sunDirection.xyz;
	vec3 normal = normalize(vert.nrm);
	float lightRatio = clamp(dot(normalize(-lightDir), normal), 0.0, 1.0);

	vec3 directionalLight = lightRatio * UBO_Data.sunColor.xyz * UBO_Data.material.Kd;

	vec3 ambient = UBO_Data.sunAmbient.xyz * UBO_Data.material.Kd;
	
	vec3 result;
	vec3 viewDir = normalize(UBO_Data.camPos.xyz - vert.pos);
	vec3 halfVector = normalize((-lightDir) + viewDir);
	//vec3 reflectVec = reflect(lightDir, viewDir);
	float intensity = max(dot(normal, halfVector), 0);
	intensity = clamp(pow(intensity, UBO_Data.material.Ns), 0, 1);
	vec3 reflectedLight = UBO_Data.sunColor.xyz * UBO_Data.material.Ks * intensity;
	result += directionalLight + ambient + reflectedLight;
	Pixel = vec4(result, 1);
}