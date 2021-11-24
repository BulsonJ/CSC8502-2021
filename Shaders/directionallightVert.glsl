#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;

uniform vec3 lightPos;
uniform vec4 lightColour;

void main(void) {
    gl_Position = (projMatrix * viewMatrix) * vec4(position , 1.0);
}