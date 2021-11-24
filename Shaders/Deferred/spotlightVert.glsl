#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;

void main(void) {
    gl_Position = (projMatrix * viewMatrix *  modelMatrix) * vec4(position * 1000, 1.0);
}