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
    GLuint vao[2];
    GLuint shader;
    GLuint texture;

    GLuint buffers[3];//, normalBuffer};
    

    GeometryData geometry;
    glm::mat4 trans;
    bool mainMenu=true;
    char menuOption;
    void menuUI();
};

#endif
