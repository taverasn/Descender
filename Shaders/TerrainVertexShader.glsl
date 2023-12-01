#version 430 // GLSL 4.30
// an ultra simple glsl vertex shader

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

// Input from VBO
layout(location = 0) in vec4 localPos;


void main()
{

	vec4 world_position = localPos * UBO_Data.worldMatrix;

	mat4 viewProj = UBO_Data.viewMatrix * UBO_Data.projectionMatrix;
	world_position *= viewProj;
	gl_Position = world_position;


}