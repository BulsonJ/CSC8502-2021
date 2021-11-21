#version 330 core

uniform sampler2D sandTex;
uniform sampler2D sandNormal;
uniform sampler2D grassTex;
uniform sampler2D grassNormal;
uniform sampler2D rockTex;
uniform sampler2D rockNormal;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent; //New!
    vec3 binormal; //New!
    vec3 worldPos;
} IN;

out vec4 fragColour;

    float invLerp(float from, float to, float value){
        return clamp((value - from)/(to-from), 0.0, 1.0);
    }

void main(void) {

    vec3 incident = normalize(lightPos - IN.worldPos );
    vec3 viewDir = normalize(cameraPos - IN.worldPos );
    vec3 halfDir = normalize(incident + viewDir );

    mat3 TBN = mat3(normalize(IN.tangent),
    normalize(IN.binormal), normalize(IN.normal ));

    // normal of texture
    
    float height = IN.worldPos.y;
    float startHeight = 100;
    float endHeight = 255;
    float heightPercent = invLerp(startHeight, endHeight, height);
    //float sandHeight = 0.3;
    float grassHeight = 0.4;
    float rockHeight = 0.6;
    
    float blend = 0.1;
    float drawStrength = invLerp(-blend/2,blend/2, heightPercent);
    vec4 diffuse = texture(sandTex , IN.texCoord ) * drawStrength;
    vec3 bumpNormal = texture(sandNormal , IN.texCoord ).rgb * drawStrength;
    
    vec3 up = vec3(0,1,0);      
    float steepness = 1 - dot(IN.normal, up);
    steepness = clamp(steepness / 0.5, 0.0,1.0);

    drawStrength = invLerp(-blend/2,blend/2, heightPercent - grassHeight);
    drawStrength *= invLerp(0, steepness, 0.2);
    diffuse = diffuse * (1-drawStrength) + texture(grassTex , IN.texCoord ) * drawStrength;
    bumpNormal = bumpNormal * (1-drawStrength) + texture(grassNormal , IN.texCoord ).rgb * drawStrength;

    drawStrength = invLerp(-blend/2,blend/2, heightPercent - rockHeight);
    diffuse = diffuse * (1-drawStrength) + texture(rockTex , IN.texCoord ) * drawStrength;
    bumpNormal = bumpNormal * (1-drawStrength) + texture(rockNormal , IN.texCoord ).rgb * drawStrength;

    diffuse.a  = 1.0;
    bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

    float lambert = max(dot(incident , bumpNormal), 0.0f);
    float distance = length(lightPos - IN.worldPos );
    float attenuation = 1.0f - clamp(distance / lightRadius ,0.0 ,1.0);

    float specFactor = clamp(dot(halfDir , bumpNormal ) ,0.0 ,1.0);
    specFactor = pow(specFactor , 60.0 );



    vec3 surface = (diffuse.rgb * lightColour.rgb);
    fragColour.rgb = surface * lambert * attenuation;
    fragColour.rgb += (lightColour.rgb * specFactor )* attenuation *0.33;
    fragColour.rgb += surface * 0.1f;
    fragColour.a = diffuse.a;
}