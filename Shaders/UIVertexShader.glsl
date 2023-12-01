#version 430 // GLSL 4.30
// an ultra simple glsl vertex shader



// input layout for the vertex
layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_texcoord;

// uniform information for the vertex shader
uniform vec2 uni_position;
uniform vec2 uni_scale;
uniform float uni_rotation;
uniform float uni_depth;

// output to the pixel shader for the vertex shader
out vec2 texcoord;


void main()
{

	texcoord = in_texcoord;
	vec2 r = vec2(cos(uni_rotation), sin(uni_rotation));
	mat2 rotate = mat2(r.x, -r.y, r.y, r.x);
	vec2 pos = uni_position + (rotate * (uni_scale * in_pos));
	gl_Position = vec4(pos, uni_depth, 1); 

}