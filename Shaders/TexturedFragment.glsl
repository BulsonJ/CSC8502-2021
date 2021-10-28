#version  330  core
uniform  sampler2D  diffuseTex;
uniform  sampler2D  diffuseTex2;
in  Vertex    {
	vec2  texCoord;
} IN;

out  vec4  fragColour;
void  main(void)    {
	if (texture(diffuseTex, IN.texCoord).a >= 1) {
		discard;
	}
	fragColour   = texture(diffuseTex , IN.texCoord );
}