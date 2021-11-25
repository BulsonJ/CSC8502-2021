#version 330 core

uniform samplerCube cubeTex;
uniform float sceneTime;
uniform float dayAngle;

in Vertex {
    vec3 viewDir;
} IN;

out vec4 fragColour;

float invLerp(float from, float to, float value){
    return clamp((value - from)/(to-from), 0.0, 1.0);
}

void main(void) {
    vec3 day = texture(cubeTex ,normalize(IN.viewDir )).rgb;
    vec3 night = vec3(0,0,0);

    float angleLerp = invLerp(-90, 90, dayAngle);
    if (dayAngle > 90){
        angleLerp = invLerp(270, 90, dayAngle);
    }

    fragColour.rgb = angleLerp * day;
    fragColour.a = 0.0;
}