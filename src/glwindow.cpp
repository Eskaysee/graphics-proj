#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"

using namespace std;

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);


    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow(){}


void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("build/simple.vert", "build/simple.frag");
    glUseProgram(shader);

    int colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

    // Load the model that we want to use and buffer the vertex attributes
    //GeometryData geometry = loadOBJFile("tri.obj");

    geometry.loadFromOBJFile("objects/tri2.obj");
    int vertexLoc = glGetAttribLocation(shader, "position");
    GLfloat* vertices = (float *)geometry.vertexData();
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3*geometry.vertexCount()*sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    // glVertex3d camPos = glVertex3d(0.0f,0.0f,3.0f);
    // glVertex3d camFront = glVertex3d(0.0f,0.0f,-1.0f);
    // glVertex3d camUp = glVertex3d(0.0f,0.1f,0.0f);

    //gluLookAt(0.0f,0.0f,3.0f,0.0f,0.0f,-1.0f,0.0f,0.1f,0.0f);

    glPrintError("Setup complete", true);

    render();
    menuUI();
}

void OpenGLWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     

    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {
        if(e.key.keysym.sym == SDLK_ESCAPE)
        {
            return false;
        }
        else if (!mainMenu){
            if (e.key.keysym.sym == SDLK_q){
                mainMenu = true;
                menuUI();
            }
            else if (menuOption == 'c' || menuOption == 'C'){
                int colorLoc = glGetUniformLocation(shader, "objectColor");
                if (e.key.keysym.sym == SDLK_w){
                    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);    //white
                }
                else if (e.key.keysym.sym == SDLK_d){
                    glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);    //black
                }
                else if (e.key.keysym.sym == SDLK_r){
                    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);    //red
                }
                else if (e.key.keysym.sym == SDLK_g){
                    glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);    //green
                }
                else if (e.key.keysym.sym == SDLK_b){
                    glUniform3f(colorLoc, 0.0f, 0.0f, 1.0f);    //blue
                }
                else if (e.key.keysym.sym == SDLK_y){
                    glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);    //yellow
                }
                else if (e.key.keysym.sym == SDLK_m){
                    glUniform3f(colorLoc, 1.0f, 0.0f, 1.0f);    //magneta
                }
                else if (e.key.keysym.sym == SDLK_c){
                    glUniform3f(colorLoc, 0.0f, 1.0f, 1.0f);    //cyan
                }
            }
            else if (menuOption == 't' || menuOption == 'T'){
                if (e.key.keysym.sym == SDLK_s){}
                else if (e.key.keysym.sym == SDLK_t){}
                else if (e.key.keysym.sym == SDLK_r){}
                else if (e.key.keysym.sym == SDLK_d){}
            }
            else if (menuOption == 'h' || menuOption == 'H'){}
        }
    }
    return true;
}

void OpenGLWindow::menuUI(){
    if (mainMenu) {
        cout << "MAIN MENU: type in terminal\n";
        cout << "Press \"c\" for colour change\nPress \"t\" for transformation\nPress \"h\" for compound transformation" << endl;
        cin >> menuOption;
    }
    cout << endl;
    if (menuOption == 'c' || menuOption == 'C'){
        cout << "COLOUR: enter in gui\n";
        cout << "[W]hite\n[D]Black" << endl;
        cout << "[R]ed\n[G]reen\n[B]lue" << endl;
        cout << "[Y]ellow\n[M]agneta\n[C]yan" << endl;
        cout << "Press \"q\" to go back to main menu options\n" << endl;
        mainMenu = false;
    }
    else if (menuOption == 't' || menuOption == 'T'){
        cout << "TRANSFORM: enter in gui\n";
        cout << "[S]cale\n[T]ranslate\n[R]otate\n[D]Shear" << endl;
        cout << "Press \"q\" to go back to main menu options\n" << endl;
        mainMenu = false;
    }
    else if (menuOption == 'h' || menuOption == 'H'){
        cout << "COMPOUND TRANSFORMATION:\n";
        cout << "Press \"q\" to go back to main menu options\n" << endl;
        mainMenu = false;
    }
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
