
#include <stdio.h>
#include "types.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define DEFAULT_HZ 120

// Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#include <tchar.h>
#define ANSI(code) ""

// OpenGL
#include <glew.h>
#include <GL/gl.h>

// 3rd Party
void *win32_VirtualAlloc(size_t Size)
{
    void *Result = 0;
    Result = VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(Result == 0)
    {
        printf("win32: VirtualAlloc failed!\n");
        return 0;
    }
    memset(Result, 0, Size);
    return Result;
}

void win32_VirtualFree(void *Pointer)
{
    VirtualFree(Pointer, 0, MEM_RELEASE);
}

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(Size)       win32_VirtualAlloc(Size)
#define STBI_REALLOC(p,newsz)   realloc(p,newsz) //TODO: Create win32_Realloc
#define STBI_FREE(Pointer)      win32_VirtualFree(Pointer)
#include "stb_image.h"

// Source
#include "renderer.cpp"

//Globals
static bool                     GlobalRunning = false;
static i64                      GlobalPerformanceCounterFrequency = 0;
static GLuint                   GlobalTextureHandle = 0;
static bool                     GlobalWireframe = false;

// Structs
typedef struct WIN32_WINDOW_DIMENSIONS
{
    i16 Width;
    i16 Height;
} WIN32_WINDOW_DIMENSIONS;


LARGE_INTEGER win32_WallClock()
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

f32 win32_SecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result = ((f32) (End.QuadPart - Start.QuadPart) / (f32) GlobalPerformanceCounterFrequency);
    return Result;
}

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch(Message)
    { 
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
            break;
        }     
    }

    return Result;
}

void win32_ProcessMessages()
{
    MSG Queue;

    while(PeekMessageW(&Queue, NULL, 0, 0, PM_REMOVE)) 
    {
        switch(Queue.message)
        {
            case WM_CLOSE:
            {
                GlobalRunning = false;
                break;
            }
            case WM_DESTROY:
            {
                GlobalRunning = false;
                break;
            }     
            // Virtual Keycode parsing
            case WM_KEYUP:
            {
                // Check key state
                u32 VKCode              = Queue.wParam;
                bool WasDown            = ((Queue.lParam & (1 << 30)) != 0);
                bool IsDown             = ((Queue.lParam & (1UL << 31)) == 0);

                // Switch
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        if(GlobalWireframe)
                        {
                            GlobalWireframe = false;
                        }
                        else
                        {
                            GlobalWireframe = true;
                        }
                    }                
                }

                break;
            } 
        }

        TranslateMessage(&Queue);
        DispatchMessageW(&Queue);
    }
}

WIN32_WINDOW_DIMENSIONS win32_GetWindowDimensions(HWND Window)
{
    WIN32_WINDOW_DIMENSIONS Result = {};
    
    // Call client rect from curreny window
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width    = ClientRect.right - ClientRect.left;
    Result.Height   = ClientRect.bottom - ClientRect.top;

    return Result;
}

void win32_InitOpenGL(HWND Window)
{
    // Create device context
    HDC OpenGLDC = GetDC(Window);

    // Initialise pixel format struct
    PIXELFORMATDESCRIPTOR PixelFormat = {};
    PixelFormat.nSize       = sizeof(PixelFormat);
    PixelFormat.nVersion    = 1;                                                            // Must be set to 1
    PixelFormat.dwFlags     = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;   // Double buffered that can drawn to any OpenGL active window
    PixelFormat.cColorBits  = 32;
    PixelFormat.cAlphaBits  = 8;
    PixelFormat.iLayerType  = PFD_MAIN_PLANE; //TODO: Is this deprecated?
    PixelFormat.iPixelType  = PFD_TYPE_RGBA;

    // Select pixel format based on the one described above
    i32 PixelFormatIndex = ChoosePixelFormat(OpenGLDC, &PixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat = {};
    DescribePixelFormat(OpenGLDC, PixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(OpenGLDC, PixelFormatIndex, &SuggestedPixelFormat);

    // Set windows context
    HGLRC OpenGLRC  = wglCreateContext(OpenGLDC);
    if(wglMakeCurrent(OpenGLDC, OpenGLRC))
    {
        // glGenTextures(1, &GlobalTextureHandle);
    }
    else
	{
        printf("win32: Failed to set OpenGL window context!\n");
    }

    ReleaseDC(Window, OpenGLDC);

    // Initialise GLEW library bindings
    if(glewInit())
    {
        printf("OpenGL: Failed to initialise GLEW library!\n");
    }
}

typedef struct TEXTURE
{
    i32 Width;
    i32 Height;
    i32 BitsPerPixel;
    u8 *Data;
} TEXTURE;

void LoadTextureFromFile(const char *Path)
{
    TEXTURE Result = {};
    stbi_set_flip_vertically_on_load(1);
    Result.Data = stbi_load("texture1.tga", &Result.Width, &Result.Height, &Result.BitsPerPixel, 4);
   
    glGenTextures(1, &GlobalTextureHandle);
    glBindTexture(GL_TEXTURE_2D, GlobalTextureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Result.Width, Result.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Result.Data);
    glBindTexture(GL_TEXTURE_2D, 0);

    if(Result.Data)
    {
        stbi_image_free(Result.Data);
    }
}


void win32_DisplayBuffer(HDC DeviceContext, i16 Width, i16 Height, RENDERER *RenderInfo)
{
    // Set polygonial mode
    if(GlobalWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Create OpenGL viewport
    glViewport(0, 0, Width, Height);

    // Render
    // Set background colour
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);    

    // Vertex Shader
    glUseProgram(RenderInfo->ShaderProgram);
    glBindVertexArray(RenderInfo->VAO);
    // glDrawArrays(GL_TRIANGLES, 0, 3); // Drawing primitive, starting index of vertex array, how many vertices to draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Push rendered backbuffer
    SwapBuffers(DeviceContext);
}

int main()
{
    //Create window and it's rendering handle
    WNDCLASSEX WindowClass = {sizeof(WNDCLASSEX), 
                            CS_CLASSDC, WindowProc, 0L, 0L, 
                            GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                            _T("DARKClass"), NULL };

    RegisterClassEx(&WindowClass);

    HWND WindowHandle = CreateWindow(WindowClass.lpszClassName, 
                        _T("DARK"), WS_OVERLAPPEDWINDOW, 100, 100, 
                        DEFAULT_WIDTH, DEFAULT_HEIGHT, 
                        NULL, NULL, WindowClass.hInstance, NULL);

    if(WindowHandle)
    {
        // Initialise OpenGL
        win32_InitOpenGL(WindowHandle);

        //Display window
        ShowWindow(WindowHandle, SW_SHOWDEFAULT);
        UpdateWindow(WindowHandle);

        //Get monitor refresh rate
        HDC RefreshDC = GetDC(WindowHandle);
        i32 MonitorRefresh = GetDeviceCaps(RefreshDC, VREFRESH);
        ReleaseDC(WindowHandle, RefreshDC);
        if(MonitorRefresh < 1)
        {
            MonitorRefresh = DEFAULT_HZ;
        }

        // Assets
        // Load textures

        // Systems
        RENDERER RenderInfo = InitialiseRenderer();
        // ConstructTriangle(&RenderInfo);
        ConstructQuad(&RenderInfo);

        // Update rate
        f32 TargetSecondsPerFrame = 1.0f / (f32) (MonitorRefresh);        

        //Start timings
        LARGE_INTEGER PerformanceCounterFrequencyResult;
        QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
        GlobalPerformanceCounterFrequency = PerformanceCounterFrequencyResult.QuadPart;

        //Request 1ms period for timing functions
        UINT SchedulerPeriodInMS = 1;
        bool IsSleepGranular = (timeBeginPeriod(SchedulerPeriodInMS) == TIMERR_NOERROR);

        //Start timings
        LARGE_INTEGER LastCounter = win32_WallClock();
        LARGE_INTEGER FlipWallClock = win32_WallClock();
        u64 LastCycleCount = __rdtsc();

        //Loop
        GlobalRunning = true;
        while(GlobalRunning)
        {
            //Updates
            //Process incoming mouse/keyboard messages, check for QUIT command
            win32_ProcessMessages();
            if(!GlobalRunning) break;

            // Render
            // Get current window size and push the back buffer
            WIN32_WINDOW_DIMENSIONS CurrentDimensions = win32_GetWindowDimensions(WindowHandle);
            HDC RenderContext = GetDC(WindowHandle);
            win32_DisplayBuffer(RenderContext, CurrentDimensions.Width, CurrentDimensions.Height, &RenderInfo);
            ReleaseDC(WindowHandle, RenderContext);

            //End performance timings
            FlipWallClock = win32_WallClock();
            u64 EndCycleCount = __rdtsc();
            LastCycleCount = EndCycleCount;

            //Check rendering work elapsed and sleep if time remaining
            LARGE_INTEGER WorkCounter = win32_WallClock();
            f32 WorkSecondsElapsed = win32_SecondsElapsed(LastCounter, WorkCounter);
            f32 SecondsElapsedForFrame = WorkSecondsElapsed;

            //If the rendering finished under the target seconds, then sleep until the next update
            if(SecondsElapsedForFrame < TargetSecondsPerFrame)
            {                        
                if(IsSleepGranular)
                {
                    DWORD SleepTimeInMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));

                    if(SleepTimeInMS > 0)
                    {
                        Sleep(SleepTimeInMS);
                    }
                }

                while(SecondsElapsedForFrame < TargetSecondsPerFrame)
                {                            
                    SecondsElapsedForFrame = win32_SecondsElapsed(LastCounter, win32_WallClock());
                }
            }

            else
            {
                f32 Difference = (SecondsElapsedForFrame - TargetSecondsPerFrame);
                printf("win32: Missed frame rate!\tDifference: %f\t[Current: %f, Target: %f]\n", Difference, SecondsElapsedForFrame, TargetSecondsPerFrame);
            } 

            //Prepare timers before next loop
            LARGE_INTEGER EndCounter = win32_WallClock();
            LastCounter = EndCounter;         
        }

        // Unload textures
        // win32_VirtualFree(TextureFile.imageData);

        DestroyWindow(WindowHandle);
        UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);        
    }
    else
    {
        DestroyWindow(WindowHandle);
        UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
        printf("win32: Failed to create window!\n");
    }

}