#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

uniform float time;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
} OUT;

void main(void) {
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix )));

    OUT.texCoord = (textureMatrix * vec4(texCoord , 0.0, 1.0)). xy;

    OUT.normal = normalize(normalMatrix * normalize(normal ));

    vec3 offset;
    offset.xy = vec2(0,0);
    offset.z = -sin(time) * 0.05f ;

    vec4 worldPos = (modelMatrix * vec4((position),1)) + (modelMatrix*vec4(offset,0));


    OUT.worldPos = worldPos.xyz;

    gl_Position = (projMatrix * viewMatrix) * worldPos;
}