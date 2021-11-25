#version 330 core

uniform  mat4  modelMatrix;
uniform  mat4  viewMatrix;
uniform  mat4  projMatrix;
uniform  mat4  textureMatrix;
uniform mat3 normalMatrix;
in   vec3  position;
in   vec2  texCoord;
in vec4 colour;
in vec3 normal;

out  Vertex   {
	vec2 texCoord;
	vec3 normal;
} OUT;

void  main(void){
	mat4  mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(position , 1.0);
	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	OUT.normal = wNormal;
	OUT.texCoord = (textureMatrix * vec4(texCoord , 0.0,  1.0)). xy;
}