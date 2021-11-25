#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform samplerCube cubeTex;
uniform sampler2D depthTex;
uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D dudvMap;

uniform mat4 projMatrix;

uniform vec3 cameraPos;
uniform float sceneTime;

uniform vec3 lightPos;
uniform vec4 lightColour;

#define NR_POINT_LIGHTS 1
uniform vec4 pointLights_lightColour[NR_POINT_LIGHTS];
uniform vec3 pointLights_lightPos[NR_POINT_LIGHTS];
uniform float pointLights_lightRadius[NR_POINT_LIGHTS];

#define NR_SPOT_LIGHTS 5
uniform vec3 spotLights_lightPos[NR_SPOT_LIGHTS];
uniform vec3 spotLights_lightDirection[NR_SPOT_LIGHTS];
uniform vec4 spotLights_lightColour[NR_SPOT_LIGHTS];
uniform float spotLights_lightCutoff[NR_SPOT_LIGHTS];

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

vec3 CalcDirLight(vec4 texture, vec3 light_pos, vec4 light_colour, vec3 normal){
    vec3 incident = normalize(-light_pos);
    vec3 viewDir = normalize(cameraPos - IN.worldPos );
    vec3 halfDir = normalize(incident + viewDir );

    float lambert = max(dot(incident , normal), 0.0f);
    float specFactor = clamp(dot(halfDir , normal ) ,0.0 ,1.0);
    specFactor = pow(specFactor , 60.0 );

    vec3 surface = (texture.rgb * light_colour.rgb);
    vec3 ambient = surface * 0.1f;
    vec3 diffuseLight = surface * lambert;
    vec3 specular =  (light_colour.rgb * specFactor ) * 0.33;
    return (ambient + diffuseLight + specular);
}

vec3 CalcSpotLight(vec4 texture, vec3 light_pos, vec3 light_direction,vec4 light_colour, float light_distance,  float light_cutoff, vec3 normal){

    vec3 incident = normalize(light_pos - IN.worldPos );
    vec3 viewDir = normalize(cameraPos - IN.worldPos );
    vec3 halfDir = normalize(incident + viewDir );

    float lambert = max(dot(incident , normal), 0.0f);
    float distance = length(light_pos - IN.worldPos );
    float attenuation = 1.0f - clamp(distance / light_distance ,0.0 ,1.0);
    float specFactor = clamp(dot(halfDir , normal ) ,0.0 ,1.0);
    specFactor = pow(specFactor , 60.0 );

    float theta = dot(light_pos - IN.worldPos, normalize(-light_direction));
 
    if(theta > light_cutoff) {
        attenuation = 0.0;
    }

    vec3 surface = (texture.rgb * light_colour.rgb);
    vec3 ambient = surface * 0.1f;
    vec3 diffuseLight = surface * lambert * attenuation;
    vec3 specular =  (light_colour.rgb * specFactor )* attenuation *0.33;
    ambient *= attenuation;
    diffuseLight *= attenuation;
    specular *= attenuation;    

    return (ambient + diffuseLight + specular);
}

vec3 CalcPointLight(vec4 texture, vec3 light_pos, float light_radius, vec4 light_colour, vec3 normal){
    vec3 incident = normalize(light_pos - IN.worldPos );
    vec3 viewDir = normalize(cameraPos - IN.worldPos );
    vec3 halfDir = normalize(incident + viewDir );

    float lambert = max(dot(incident , normal), 0.0f);
    float distance = length(light_pos - IN.worldPos );
    float attenuation = 1.0f - clamp(distance / light_radius ,0.0 ,1.0);
    float specFactor = clamp(dot(halfDir , normal ) ,0.0 ,1.0);
    specFactor = pow(specFactor , 60.0 );

    vec3 surface = (texture.rgb * light_colour.rgb);
    vec3 ambient = surface * 0.1f;
    vec3 diffuseLight = surface * lambert * attenuation;
    vec3 specular =  (light_colour.rgb * specFactor )* attenuation *0.33;
    ambient *= attenuation;
    diffuseLight *= attenuation;
    specular *= attenuation;    

    return (ambient + diffuseLight + specular);
}

void main(void) {

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

    vec4 diffuse = texture(diffuseTex , IN.texCoord );
    diffuse.rgb = mix(reflectColour.rgb,refractColour.rgb,refractiveFactor) + (diffuse.rgb * 0.25f);

    // add foam
    float strength = 2;
    float foamAmount = clamp((waterDepth / 25.0) * strength, 0.0,1.0);
    diffuse.rgb = mix(vec3(1.0,1.0,1.0), diffuse.rgb, foamAmount);

    mat3 TBN = mat3(normalize(IN.tangent),
    normalize(IN.binormal), normalize(IN.normal ));
    vec3 bumpNormal = texture(bumpTex , IN.texCoord ).rgb;
    bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

    vec3 output;
    output = CalcDirLight(diffuse, lightPos,lightColour, bumpNormal); 
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        output += CalcPointLight(diffuse, pointLights_lightPos[i],pointLights_lightRadius[i],pointLights_lightColour[i], bumpNormal); 
        // (specular + diffuse * shadow)
    }
    for(int i = 0; i < NR_SPOT_LIGHTS; i++){
        output += CalcSpotLight(diffuse, spotLights_lightPos[i],spotLights_lightDirection[i], spotLights_lightColour[i],500.0f, spotLights_lightCutoff[i],bumpNormal);
    }   
    fragColour.rgb = output;

    fragColour.a = clamp(waterDepth/5.0, 0.0,1.0);
}