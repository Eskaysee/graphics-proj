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
    GLuint shader;
    GLuint lightShader;
    GLuint texture;

    GLuint buffers[3];
    GLuint vao[2];

    GeometryData geometry;
    glm::mat4 trans;
    bool mainMenu=true;
    char menuOption;
    void menuUI();
};

#endif
