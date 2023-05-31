
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>


#include "glwindow.h"
#include "geometry.h"

#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

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

OpenGLWindow::OpenGLWindow()
{
    trans = glm::mat4(1.0f);
    geometry.loadFromOBJFile("objects/cube/cube.obj");
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 720, SDL_WINDOW_OPENGL);
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
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback( MessageCallback, 0 );
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    glGenVertexArrays(2, vao);
    glBindVertexArray(vao[0]);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("build/simple.vert", "build/simple.frag");
    glUseProgram(shader);

    int colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

    // Load the model that we want to use and buffer the vertex attributes
    //GeometryData geometry = loadOBJFile("tri.obj");

    GLint vertexLoc = glGetAttribLocation(shader, "position");
    GLfloat* vertices = (float *)geometry.vertexData();
    glGenBuffers(3, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, 3*geometry.vertexCount()*sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false,  0, (void*)0);
    glEnableVertexAttribArray(vertexLoc);

    if (geometry.hasTextCoords()){
        GLint textBool = glGetUniformLocation(shader, "isText");
        glUniform1i(textBool, 1);       //SET TO ZERO TO DEACTIVATE TEXTURING
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load and generate the texture
        int width, height, nrChannels;
        unsigned char *data = stbi_load("objects/cube/crate.jpg", &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data); 
            
        GLint textLoc = glGetAttribLocation(shader, "vText");
        GLfloat* textureCoord = (float*)geometry.textureCoordData();
        glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ARRAY_BUFFER, 2*geometry.vertexCount()*sizeof(float), textureCoord, GL_STATIC_DRAW);
        glVertexAttribPointer(textLoc, 2, GL_FLOAT, true,  0, 0);
        glEnableVertexAttribArray(textLoc);

        GLint normLoc = glGetAttribLocation(shader, "norms");
        cout << normLoc << "\n" << endl;
        GLfloat* normals = (float*)geometry.normalData();
        glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
        glBufferData(GL_ARRAY_BUFFER, 3*geometry.vertexCount()*sizeof(float), normals, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, true,  0, 0);
        glEnableVertexAttribArray(2);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao[0]);
    }

    // GLint normLoc = glGetAttribLocation(shader, "normal");
    // GLfloat* normalData = (float*)geometry.normalData();
    // glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(normalData), normalData, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(2);
    // glVertexAttribPointer(normLoc, 3, GL_FLOAT, false, 0, 0);

    
    glm::vec3 camPos = glm::vec3(0.0f,0.0f,3.0f);
    glm::vec3 camFront = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 camUp =  glm::vec3(0.0f,1.0f,0.0f);

    glm::mat4 model = glm::mat4(1.0f);
    glm ::mat4 view = glm::lookAt(camPos, camFront, camUp);
    glm::mat4 projection = glm::perspective(90.0f, (4.0f/3.0f), 0.1f, 75.0f);
    GLint modLoc = glGetUniformLocation(shader, "model");
    glUniformMatrix4fv(modLoc, 1, GL_FALSE, glm::value_ptr(model));
    GLint viewLoc = glGetUniformLocation(shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    GLint projLoc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // //unsigned int lightVAO; 
    // unsigned int VBO;
    // //glGenVertexArrays(1, &lightVAO);
    // glBindVertexArray(vao[1]);
    // // we only need to bind to the VBO, the container's VBO's data already contains the data.
    // glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    // glm::mat4 model2 = glm::mat4(1.0f);
    // glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    // model2 = glm::translate(model, lightPos);
    // model2 = glm::scale(model, glm::vec3(0.2f)); 
    // // set the vertex attribute 
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    glPrintError("Setup complete", true);
    render();
    menuUI();
}

void OpenGLWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
            GLint colorLoc = glGetUniformLocation(shader, "objectColor");
            GLint transLoc = glGetUniformLocation(shader, "model");
            if (e.key.keysym.sym == SDLK_q){
                mainMenu = true;
                menuUI();
            }
            else if (menuOption == 'c' || menuOption == 'C'){
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
            else if (menuOption == 's' || menuOption == 'S'){
                if (e.key.keysym.sym == SDLK_KP_PLUS || e.key.keysym.sym == SDLK_PLUS || e.key.keysym.sym == SDLK_EQUALS){
                    trans = glm::scale(trans, glm::vec3(2.0f, 2.0f, 2.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
                else if (e.key.keysym.sym == SDLK_MINUS){
                    trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
            }
            else if (menuOption == 't' || menuOption == 'T') { 
                if (e.key.keysym.sym == SDLK_LEFT){
                    trans = glm::translate(trans, glm::vec3(-0.5f, 0.0f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
                else if (e.key.keysym.sym == SDLK_RIGHT){
                    trans = glm::translate(trans, glm::vec3(0.5f, 0.0f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
                else if (e.key.keysym.sym == SDLK_UP){
                    trans = glm::translate(trans, glm::vec3(0.0f, 0.5f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
                else if (e.key.keysym.sym == SDLK_DOWN){
                    trans = glm::translate(trans, glm::vec3(0.0f, -0.5f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
            }
            else if (menuOption == 'R' || menuOption == 'r'){
                if (e.key.keysym.sym == SDLK_LEFT){
                    trans = glm::rotate(trans, glm::radians(22.5f), glm::vec3(0.0f, 1.0f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
                else if (e.key.keysym.sym == SDLK_RIGHT){
                    trans = glm::rotate(trans, glm::radians(-22.5f), glm::vec3(0.0f, 1.0f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
                else if (e.key.keysym.sym == SDLK_UP){
                    trans = glm::rotate(trans, glm::radians(-22.5f), glm::vec3(1.0f, 0.0f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
                else if (e.key.keysym.sym == SDLK_DOWN){
                    trans = glm::rotate(trans, glm::radians(22.5f), glm::vec3(1.0f, 0.0f, 0.0f));
                    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
                }
            }
            else if (menuOption == 'h' || menuOption == 'H'){}
        }
    }
    return true;
}

void OpenGLWindow::menuUI(){
    if (mainMenu) {
        cout << "MAIN MENU: type in terminal\n";
        cout << "Press \"c\" for colour change\nPress \"t\" for transformation\
        \nPress\"h\" for compound transformation\nPress \"r\" to reset object" << endl;
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
        cout << "TRANSFORM: type in terminal\n";
        cout << "[S]cale\n[T]ranslate\n[R]otate\n[D]Shear" << endl;
        cin >> menuOption;
        cout << endl;
        if (menuOption == 'T' || menuOption == 't') {
            mainMenu = false;
            cout << "Use direction keys in GUI\n";
            cout << "Press \"q\" to go back to main menu options\n" << endl;
        }
        else if (menuOption == 'S' || menuOption == 's') {
            mainMenu = false;
            cout << "In the GUI enter:" << endl;
            cout << "\"+\" to double the object size" << endl;
            cout << "\"-\" to half the object size" << endl;
            cout << "Press \"q\" to go back to main menu options\n" << endl;
        }
        else if (menuOption == 'R' || menuOption == 'r'){
            mainMenu = false;
            cout << "In the GUI enter:" << endl;
            cout << "left or right arrow key to rotate about the y axis" << endl;
            cout << "up or down arrow key to rotate about the x axis" << endl;
            cout << "Press \"q\" to go back to main menu options\n" << endl;
        }
    }
    else if (menuOption == 'h' || menuOption == 'H'){
        cout << "COMPOUND TRANSFORMATION:\n";
        cout << "Press \"q\" to go back to main menu options\n" << endl;
        mainMenu = false;
    }
    else if (menuOption == 'r' || menuOption == 'R'){
        mainMenu = true;
        GLint transLoc = glGetUniformLocation(shader, "model");
        GLint colorLoc = glGetUniformLocation(shader, "objectColor");
        trans = glm::mat4(1.0f);
        glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    }
}

void OpenGLWindow::cleanup()
{
    glDeleteTextures(1, &texture);
    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(2, vao);
    SDL_DestroyWindow(sdlWin);
}
