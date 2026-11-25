#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

layout (location = 0) out vec3 fragColor;

layout(push_constant) uniform constants {
	mat4 inv_view_proj;
	vec4 pos;
} PushConstants;

const vec3 ambient = vec3(0.1, 0.15, 0.2);

void main() {
	gl_Position = PushConstants.inv_view_proj * vec4(PushConstants.pos.xyz + vPosition, 1.0);

	vec3 direct = vec3(1, 1.25, 1) * max(dot(vNormal, normalize(vec3(0, 0, 1))), 0);
	vec3 albedo = vec3(PushConstants.pos.w, 0.1, 0.1);
	fragColor = albedo * (ambient + direct);
}
