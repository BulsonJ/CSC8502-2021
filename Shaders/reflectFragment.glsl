#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform samplerCube cubeTex;
uniform sampler2D depthTex;
uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D dudvMap;

uniform mat4 projMatrix;

uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform vec3 cameraPos;
uniform float sceneTime;

const float waveStrength = 0.02;
const float refractionStrength = 50;

in Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent; //New!
    vec3 binormal; //New!
    vec3 worldPos;
    vec4 clipSpace;
    vec3 toCameraVector;
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
   // Light calculations
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

    // Calculate cubeMap reflection and add to diffuse
    vec3 reflectDir = reflect(-viewDir ,normalize(IN.normal));
    vec4 reflectTex = texture(cubeTex ,reflectDir );
    //diffuse.rgb = reflectTex.rgb + (diffuse.rgb * 0.25f);

    // Calculate refract/reflext tex coords
    vec2 ndc = (IN.clipSpace.xy/IN.clipSpace.w)/2.0 + 0.5;
    vec2 refractTexCoords = vec2(ndc.x,ndc.y);
    vec2 reflectTexCoords = vec2(ndc.x,-ndc.y);

    // Calculate depth
    float far = 15000.0;
    float near = 1.0;
    
    float depth = texture(depthTex, refractTexCoords).r;
    float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
    
    depth = gl_FragCoord.z;
    float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
    float waterDepth = floorDistance - waterDistance;

    // Dudv map
    vec2 distortion1 = (texture(dudvMap, vec2(IN.texCoord.x + (sceneTime / refractionStrength), IN.texCoord.y)).rg * 2.0 - 1.0) * waveStrength;
    vec2 distortion2 = (texture(dudvMap, vec2(-IN.texCoord.x + (sceneTime / refractionStrength), (IN.texCoord.y + sceneTime / refractionStrength))).rg * 2.0 - 1.0) * waveStrength;
    vec2 totalDistortion = distortion1 + distortion2;
    
    // Move texture coords by distortion
    refractTexCoords += totalDistortion;
    refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
    reflectTexCoords += totalDistortion;
    reflectTexCoords.x = clamp(reflectTexCoords.x, 0.001, 0.999);
    reflectTexCoords.y = clamp(reflectTexCoords.y, -0.999, -0.001);

    vec4 reflectColour = texture(reflectionTex, reflectTexCoords);
    vec4 refractColour = texture(refractionTex, refractTexCoords);

    vec3 viewVector = normalize(IN.toCameraVector);
    float refractiveFactor = dot(viewVector, vec3(0.0,1.0,0.0));
    //refractiveFactor = pow(refractiveFactor, 10.0);

    diffuse.rgb = mix(reflectColour.rgb,refractColour.rgb,refractiveFactor) + (diffuse.rgb * 0.25f);

    // Calculate final colour
    vec3 surface = (diffuse.rgb * lightColour.rgb);
    fragColour.rgb = surface * lambert * attenuation;
    fragColour.rgb += (lightColour.rgb * specFactor )* attenuation *0.33;
    fragColour.rgb += surface * 0.1f;

    // add foam
    float strength = 2;
    float foamAmount = clamp((waterDepth / 25.0) * strength, 0.0,1.0);
    fragColour.rgb = mix(vec3(1.0,1.0,1.0), fragColour.rgb, foamAmount);
    fragColour.a = clamp(waterDepth/5.0, 0.0,1.0);
}