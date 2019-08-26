
typedef struct RENDERER
{
    i32 ShaderProgram;
    u32 VAO;
} RENDERER;


void CompilerAndLinkShaders(i32 &ShaderProgram)
{
    // Compiling
    // Vertex shader
	u32 VertexShader = 0;
    VertexShader = glCreateShader(GL_VERTEX_SHADER);

    // TODO: Lookup GLSL includes
    const char *VertexShaderSource = "#version 460 core\n"
    "layout (location = 0) in vec3 Position;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);\n"
    "}\0";

    // Reference and compile shader source
    glShaderSource(VertexShader, 1, &VertexShaderSource, 0);
    glCompileShader(VertexShader);    

    // Check compile logs
    i32 Result = 0;
    char Log[512];
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Result);
    if(!Result)
    {
        glGetShaderInfoLog(VertexShader, 512, 0, Log);
        printf("Shader: Failed to compile Vertex Shader! Log: %s", Log);
    }

    // Fragment shader
    u32 FragmentShader = 0;
    FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // TODO: Lookup GLSL includes
    const char *FragmentShaderSource = "#version 460 core\n"
    "out vec4 FragmentColour;\n"
    "void main()\n"
    "{\n"
    "   FragmentColour = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

    // Reference and compile shader source
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, 0);
    glCompileShader(FragmentShader);    

    // Check compile logs
    Result = 0;
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Result);
    if(!Result)
    {
        glGetShaderInfoLog(FragmentShader, 512, 0, Log);
        printf("Shader: Failed to compile Fragment Shader! Log: %s", Log);
    }

    // Linking
    // Attach and link compiled shaders to the program object
    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);

    // Error check
    Result = 0;
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Result);
    if(!Result) 
    {
        glGetProgramInfoLog(ShaderProgram, 512, 0, Log);
        printf("Shader: Failed to link shaders! Log: %s", Log);
    }

    // Free compiled shaders
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
}

RENDERER InitialiseRenderer()
{
    RENDERER RenderInfo = {};
    CompilerAndLinkShaders(RenderInfo.ShaderProgram);
    return RenderInfo;
}

void ConstructTriangle(RENDERER *RenderInfo)
{
    // Set Normalized Device Coordinates
    f32 Vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };  

    // Vertex arrays are used to store collections of attributes and their associated VBOs
    u32 VAO;
    glGenVertexArrays(1, &VAO);

    // Vertex buffers are large buffers used to send vertex data in batches (as is optimised for GPU memory fetchs)
    // After generating with a unique ID, bind to an array buffer
    u32 VBO;
    glGenBuffers(1, &VBO);
    
    // Bind and generate data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copy the vertex data to the bound buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW - Vertex data will rarely change every draw    

    // Specify how vertext data is interpreted:
    // 0 - matches location (0) attribute in vertex shader code
    // 3 - Size of vertex (v3)
    // GL_FLOAT, GL_FALSE - Float values without any normalisation
    // 0 - Offset in memory for each vertex in the buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(f32) * 3), (void *) 0);
    glEnableVertexAttribArray(0); // Assign to previous VBO (0)

    // Unbind VAO
    glBindVertexArray(0);   

    // Store render state
    RenderInfo->VAO = VAO;  
}

void ConstructQuad(RENDERER *RenderInfo)
{
    // Set Normalized Device Coordinates
    // Use indices to specifiy the unique vertices in a quad (overlap means the number expands from 4 to 6 otherwise)
    float Vertices[] = {
        0.5f, 0.5f, 0.0f, // Top right
        0.5f, -0.5f, 0.0f, // Bottom right
        -0.5f, -0.5f, 0.0f, // Bottom left
        -0.5f, 0.5f, 0.0f  // Top left 
    };
    u32 Indices[] = {
        0, 1, 3,            // first triangle
        1, 2, 3             // second triangle
    };

    // Generate vertex array
    u32 VAO;
    glGenVertexArrays(1, &VAO);

    // Generate vertex buffers including element buffers
    u32 VBO;
    u32 EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // Bind and generate data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    // Bind element buffer using constructed VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    // Specify how vertext data is interpreted:
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(f32) * 3), (void *) 0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);   

    // Store render state
    RenderInfo->VAO = VAO;  
}