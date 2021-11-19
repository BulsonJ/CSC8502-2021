#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform samplerCube cubeTex;
uniform sampler2D depthTex;

uniform mat4 projMatrix;

uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform vec3 cameraPos;

in Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent; //New!
    vec3 binormal; //New!
    vec3 worldPos;
} IN;

out vec4 fragColour;

float linearDepth(float depthSample)
{
    depthSample = 2.0 * depthSample - 1.0;
    float zNear = 1.0f;
    float zFar = 15000.0f;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

void main(void) {
   
    vec3 incident = normalize(lightPos - IN.worldPos );
    vec3 viewDir = normalize(cameraPos - IN.worldPos );
    vec3 halfDir = normalize(incident + viewDir );

    mat3 TBN = mat3(normalize(IN.tangent),
    normalize(IN.binormal), normalize(IN.normal ));

    vec4 diffuse = texture(diffuseTex , IN.texCoord );
    vec3 bumpNormal = texture(bumpTex , IN.texCoord ).rgb;
    bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

    float lambert = max(dot(incident , bumpNormal), 0.0f);
    float distance = length(lightPos - IN.worldPos );
    float attenuation = 1.0f - clamp(distance / lightRadius ,0.0 ,1.0);

    float specFactor = clamp(dot(halfDir , bumpNormal ) ,0.0 ,1.0);
    specFactor = pow(specFactor , 60.0 );

    // Calculate reflection and add to diffuse
    vec3 reflectDir = reflect(-viewDir ,normalize(IN.normal));
    vec4 reflectTex = texture(cubeTex ,reflectDir );
    diffuse.rgb = reflectTex.rgb + (diffuse.rgb * 0.25f);

    vec3 surface = (diffuse.rgb * lightColour.rgb);
    fragColour.rgb = surface * lambert * attenuation;
    fragColour.rgb += (lightColour.rgb * specFactor )* attenuation *0.33;
    fragColour.rgb += surface * 0.1f;
    fragColour.a = diffuse.a;
    fragColour.a = 0.7;

    float depth = texture(depthTex, gl_FragCoord.xy).r;

    depth = linearDepth(depth);

    fragColour.rgb = vec3(depth,depth,depth);



    //fragColour = vec4(gl_FragCoord.z,gl_FragCoord.z,gl_FragCoord.z,1.0);
}