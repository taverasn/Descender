#version 430 // GLSL 4.30
out vec4 Pixel;
// an ultra simple glsl fragment shader

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


void main() 
{	
	Pixel = vec4(255.0f / 255.0f, 165.0f / 255.0f, 0.0f / 255.0f, 1);
}