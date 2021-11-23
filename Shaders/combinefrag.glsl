#version 330 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix; 

uniform sampler2D diffuseTex;
uniform sampler2D depthTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;
uniform sampler2D shadowTex;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(IN.texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverse(projMatrix) * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inverse(viewMatrix) * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main(void) {
    vec3 diffuse = texture(diffuseTex , IN.texCoord ).xyz;
    float diffuseValue = texture(diffuseTex , IN.texCoord ).a;
    vec3 light = texture(diffuseLight , IN.texCoord ).xyz;
    vec3 specular = texture(specularLight , IN.texCoord ).xyz;

    float shadow = 1.0;

    float depth = texture(depthTex, IN.texCoord).r;


    //vec3 shadowNDC = shadowProj.xyz / shadowProj.w;
    vec3 shadowNDC = WorldPosFromDepth(depth);
    if(abs(shadowNDC.x) < 1.0f &&
    abs(shadowNDC.y) < 1.0f &&
    abs(shadowNDC.z) < 1.0f) {
        vec3 biasCoord = shadowNDC *0.5f + 0.5f;
        float shadowZ = texture(shadowTex , biasCoord.xy).x;
        if(shadowZ < biasCoord.z) {
           shadow = 0.0f;
        }
    }

    fragColour.rgb = diffuse;
    if (diffuseValue != 0.00){
        fragColour.xyz = diffuse * 0.1; // ambient
        fragColour.rgb *= shadow; // shadowing factor
        fragColour.xyz += diffuse * light; // lambert
        fragColour.xyz += specular; // Specular
    }

    
    fragColour.a = 1.0;
}