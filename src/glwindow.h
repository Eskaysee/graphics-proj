#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>

#include "glm/glm.hpp"

#include "geometry.h"

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render();
    bool handleEvent(SDL_Event e);
    void cleanup();

private:
    SDL_Window* sdlWin;
    GLuint vao;
    GLuint shader;
    GLuint vertexBuffer;

    GeometryData geometry;
    bool mainMenu=true;
    char menuOption;
    void menuUI();
};

#endif
