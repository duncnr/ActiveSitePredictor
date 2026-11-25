#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

layout (location = 0) out vec3 fragColor;

layout(push_constant) uniform constants {
	mat4 inv_view_proj;
	vec4 pos;
} PushConstants;

void main() {
	gl_Position = PushConstants.inv_view_proj * vec4(PushConstants.pos.xyz + vPosition, 1.0);
	fragColor = vNormal;
}
