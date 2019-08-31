
typedef struct RENDERER_SETTINGS
{
    V2U RenderDimensions;
    GLuint ShaderProgram;
    GLuint VertexArrayObject; // Simply stores the state of the VertexArrayObject after each gen/bind
} RENDERER;

void CheckCompilerLogs(GLuint ShaderID)
{
    // Check compile logs
    GLint Result = 0;
    GLchar Log[1024];
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
    if(!Result)
    {
        glGetShaderInfoLog(ShaderID, 1024, 0, Log);
        Fatal("Shader: Failed to compile Vertex Shader! Compiler log: %s", Log);
    }
}

// TODO: Lookup GLSL includes - reading a file and converting to a string
GLuint CreateShaderProgram(MEMORY_ARENA *Arena)
{
    // Compiling
    // General defines
    GLchar Defines[1024];
    FormatString(sizeof(Defines), Defines,
                "#version %d\n"
                "#define f32 float\n"
                "#define i32 int\n"
                "#define u32 int unsigned\n"
                "#define v2 vec2\n"
                "#define V4 vec4\n"
                "#define V3 vec3\n"
                "#define V2 vec2\n",
                OPENGL_VERSION);

    // Vertex shader
    // Load from text file
    GLchar *VertexCode = (GLchar *) FileLoadText(Arena, "data/shaders/vertex.glsl");

    // Compile
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLchar *VertexShaderCode[] =
    {
        Defines,
        VertexCode,
    };
    glShaderSource(VertexShaderID, ArrayCount(VertexShaderCode), VertexShaderCode, 0);
    glCompileShader(VertexShaderID);
    
    // Fragment shader
    // Load from text file
    GLchar *FragmentCode = (GLchar *) FileLoadText(Arena, "data/shaders/fragment.glsl");

    // Compile
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar *FragmentShaderCode[] =
    {
        Defines,
        FragmentCode,
    };
    glShaderSource(FragmentShaderID, ArrayCount(FragmentShaderCode), FragmentShaderCode, 0);
    glCompileShader(FragmentShaderID);

    // Linking
    // Attach and link compiled shaders to the program object
    GLuint ShaderProgramID = glCreateProgram();
    glAttachShader(ShaderProgramID, VertexShaderID);
    glAttachShader(ShaderProgramID, FragmentShaderID);
    glLinkProgram(ShaderProgramID);

    // Error check
    // Check shader compiles
    CheckCompilerLogs(VertexShaderID);
    CheckCompilerLogs(FragmentShaderID);

    // Check linking status
    glValidateProgram(ShaderProgramID);
    GLint Result = 0;
    glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &Result);    

    // Helper function failed - get the info logs
    if(!Result)
    {
        GLsizei Ignored;
        GLchar VertexErrors[4096];
        GLchar FragmentErrors[4096];
        GLchar ProgramErrors[4096];
        glGetShaderInfoLog(VertexShaderID, sizeof(VertexErrors), &Ignored, VertexErrors);
        glGetShaderInfoLog(FragmentShaderID, sizeof(FragmentErrors), &Ignored, FragmentErrors);
        glGetProgramInfoLog(ShaderProgramID, sizeof(ProgramErrors), &Ignored, ProgramErrors);
        
        Assert(0, "OpenGL: Shader compile/linking (validation) failed!");
    }

    // Free compiled shaders
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ShaderProgramID;
}

RENDERER InitialiseRenderer(MEMORY_ARENA *Arena, V2U Dimensions)
{
    RENDERER RenderInfo = {};
    RenderInfo.RenderDimensions = Dimensions;
    RenderInfo.ShaderProgram    = CreateShaderProgram(Arena);
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
    GLuint VertexArrayObject;
    glGenVertexArrays(1, &VertexArrayObject);

    // Vertex buffers are large buffers used to send vertex data in batches (as is optimised for GPU memory fetchs)
    // After generating with a unique ID, bind to an array buffer
    GLuint VertexBufferObject;
    glGenBuffers(1, &VertexBufferObject);
    
    // Bind and generate data
    glBindVertexArray(VertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);

    // Copy the vertex data to the bound buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); // GL_STATIC_DRAW - Vertex data will rarely change every draw    

    // Specify how vertext data is interpreted:
    // 0 - matches location (0) attribute in vertex shader code
    // 3 - Size of vertex (v3)
    // GL_FLOAT, GL_FALSE - Float values without any normalisation
    // 0 - Offset in memory for each vertex in the buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(f32) * 3), (void *) 0);
    glEnableVertexAttribArray(0); // Assign to previous VertexBufferObject (0)

    // Unbind VertexArrayObject
    glBindVertexArray(0);   

    // Store render state
    RenderInfo->VertexArrayObject = VertexArrayObject;  
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
    GLuint Indices[] = {
        0, 1, 3,            // first triangle
        1, 2, 3             // second triangle
    };

    // Generate vertex array
    GLuint VertexArrayObject;
    glGenVertexArrays(1, &VertexArrayObject);

    // Generate vertex buffers including element buffers
    GLuint VertexBufferObject;
    GLuint ElementBufferObject;
    glGenBuffers(1, &VertexBufferObject);
    glGenBuffers(1, &ElementBufferObject);
    
    // Bind and generate data
    glBindVertexArray(VertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    // Bind element buffer using constructed VertexBufferObject
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    // Specify how vertext data is interpreted:
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(f32) * 3), (void *) 0);
    glEnableVertexAttribArray(0);

    // Unbind VertexArrayObject
    glBindVertexArray(0);   

    // Store render state
    RenderInfo->VertexArrayObject = VertexArrayObject;  
}