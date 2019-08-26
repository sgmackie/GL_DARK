
// Specify OpenGL version
#version 460 core

// Specify shader inputs
layout (location = 0) in vec3 Position;

void main()
{
    // Set shader output with Position
    gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);
}