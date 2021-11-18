#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;
uniform mat3 normalMatrix;

uniform float sceneTime;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec4 tangent; //New! Note , Vec4!
in vec2 texCoord;

out Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent; //New! Note , Vec3!
    vec3 binormal; //New!
    vec3 worldPos;
} OUT;

void main(void) {
    float amplitude = 10.0f;
    float waveLength = 400.0f;
    float speed = 10.0f;

    vec3 offset = vec3(0,0,0);
    float k = 2 * 3.142 / waveLength; // last float wavelength
    float f = k * (position.x - speed * sceneTime);
    offset.y = amplitude * sin(f) ; // amplitude first float

    vec3 tangent_wave = normalize(vec3(1, k * amplitude * cos(f), 0));
    vec3 normal_wave = vec3(-tangent.y, tangent.x, 0);
    
    vec3 wNormal = normalize(normalMatrix * normalize(normal ));
    vec3 wTangent = normalize(normalMatrix * normalize(tangent_wave));

    OUT.normal = wNormal;
    OUT.tangent = wTangent;
    OUT.binormal = cross(wTangent , wNormal) * tangent.w;

    OUT.texCoord = (textureMatrix * vec4(texCoord , 0.0, 1.0)). xy;

    vec4 worldPos = (modelMatrix * vec4((position + offset),1));

    OUT.worldPos = worldPos.xyz;

    gl_Position = (projMatrix * viewMatrix) * worldPos;
}