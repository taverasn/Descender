#version 430 // GLSL 4.30
// an ultra simple glsl vertex shader

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

layout (binding = 0, std430, row_major) buffer SSBO
{
	mat4 worldInstance[MAX_INSTANCE_PER_DRAW];
	int instanceIdx;
};


out struct Vertex {
    vec3 pos;
    vec3 uvw;
    vec3 nrm;
};

out Vertex vert;


// Input from VBO
layout(location = 0) in vec3 localPos;
layout(location = 1) in vec3 localUVW;
layout(location = 2) in vec3 localNorm;

int instance_id;


void main()
{

	instance_id  = gl_InstanceID + instanceIdx;

    vert.uvw = localUVW;

	// DONE: Part 1h
	//vert.pos.y -= 0.75;
	// DONE: Part 2h
	vec4 world_position = vec4(localPos, 1) * worldInstance[instanceIdx + gl_InstanceID];

	vert.pos = world_position.xyz;
	mat4 viewProj = UBO_Data.viewMatrix * UBO_Data.projectionMatrix;
	world_position *= viewProj;
	gl_Position = world_position;

	vec4 norm_world = vec4(localNorm, 0);
	norm_world *= UBO_Data.worldMatrix;
	vert.nrm = normalize(norm_world.xyz);
}