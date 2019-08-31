// Specify shader inputs
layout (location = 0) in V3 Position;
out V4 VertexColour;

void main()
{
    // Set shader output with Position
    gl_Position = V4(Position, 1.0);
    
    // Set fragment shader colour
    VertexColour = V4(0.5, 0.0, 0.0, 1.0);
}