#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;
uniform mat3 normalMatrix;

uniform float speed;
uniform float waveLength;
uniform float amplitude;

uniform vec4 plane;
uniform vec3 cameraPos;

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
    vec4 clipSpace;
    vec3 toCameraVector;
} OUT;

void main(void) {

    // Calculate water offset
    vec3 offset = vec3(0,0,0);
    float k = 2 * 3.142 / waveLength;
    float f = k * (position.x - speed * sceneTime);
    offset.y = amplitude * sin(f) ;

    // Calculate normal and tanget
    vec3 tangent_wave = normalize(vec3(1, k * amplitude * cos(f), 0));
    vec3 normal_wave = vec3(-tangent.y, tangent.x, 0);

    // Convert normal and tangent to world space
    vec3 wNormal = normalize(normalMatrix * normalize(normal_wave));
    vec3 wTangent = normalize(normalMatrix * normalize(tangent_wave));

    OUT.normal = wNormal;
    OUT.tangent = wTangent;
    OUT.binormal = cross(wTangent , wNormal) * tangent.w;
    
    vec4 worldPos = (modelMatrix * vec4((position + offset),1));

    OUT.worldPos = worldPos.xyz;
    OUT.clipSpace = projMatrix * viewMatrix * worldPos;
    OUT.toCameraVector = cameraPos - worldPos.xyz;
    OUT.texCoord = texCoord;

    // Set clipping plane
    gl_ClipDistance[0] = dot(worldPos, plane);



    gl_Position = projMatrix * viewMatrix * worldPos;
}