#version  330  core
uniform  sampler2D  diffuseTex;
uniform  sampler2D  diffuseTex2;
in  Vertex    {
	vec2  texCoord;
	vec4 color;
} IN;

out  vec4  fragColour;
void  main(void)    {
	//fragColour   = texture(diffuseTex , IN.texCoord ) * IN.color;
	fragColour   = texture(diffuseTex , IN.texCoord ) * texture(diffuseTex2 , IN.texCoord );
}