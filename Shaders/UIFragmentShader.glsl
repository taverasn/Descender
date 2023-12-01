#version 430 // GLSL 4.30
//out vec4 Pixel;
//// an ultra simple glsl fragment shader



// uniform information for the pixel shader
uniform sampler2D texture0;

// input from the vertex shader
in vec2 texcoord;

// out from the pixel shader
out vec4 fragment_color;

void main() 
{	
	//Pixel = vec4(255.0f / 255.0f, 165.0f / 255.0f, 0.0f / 255.0f, 1);

	vec4 diffuse = texture(texture0, texcoord);
	fragment_color = diffuse;
}