//------------------------------------------------------------------------------
//
// quickGL - A quick and easy to use OpenGL wrapper
//
// RUNTIME LIBRARIES PACKAGE
//    camera.cpp
//
// DESCRIPTION:
// -----------
// Main library.
// Provides the Scene class and the extendable qgl_callback namespace.
// 
// AUTHORS:
// -------
//      Igor Nunes (https://github.com/thoga31)
// 
// LICENSE:
// -------
//      GNU GPL V3.0
//------------------------------------------------------------------------------

#include <iostream>
#include "quickgl.hpp"

using namespace qgl;

namespace qgl::callback {
    Scene* scene;

    void bindInstance(Scene* scn) {
        scene = scn;
    }

    Scene* getInstance(void) {
        return scene;
    }
}

const char* QGLException::what(void) const throw () { return message.c_str(); }

GLFWwindow* Scene::getWindow(void) {
    return window;
}

mouse_data Scene::getMouseData(void) {
    return mouse;
}

bool Scene::init_glfw(void) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(scr_width, scr_height, scr_title.c_str(), NULL, NULL);
    if (window == NULL)
        return false;

    glfwMakeContextCurrent(window);

    if (framebuffer_size_callback != nullptr)
        setFrameBufferSizeCallback(framebuffer_size_callback);
    if (mousebtn_callback != nullptr)
        setMouseButtonCallback(mousebtn_callback);
    if (mouse_callback != nullptr)
        setCursorPositionCallback(mouse_callback);
    if (scroll_callback != nullptr)
        setScrollCallback(scroll_callback);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    return true;
}

#ifdef QGL_GLAD
int Scene::init_glad(void) {
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}
#endif


void Scene::initialize(void) {
    initialize(DEFAULT_SCR_HEIGHT, DEFAULT_SCR_WIDTH, DEFAULT_SCR_TITLE);
}

void Scene::initialize(const unsigned width, const unsigned height) {
    initialize(width, height, DEFAULT_SCR_TITLE);
}

void Scene::initialize(const unsigned width, const unsigned height, string title) {
    try {
        scr_height = height;
        scr_width = width;
        scr_title = title;

        if (!init_glfw())
            throw QGLException("Failed to create window.");

#ifdef QGL_GLAD
        if (!init_glad())
            throw QGLException("Failed to initialize GLAD.");
#endif

        success = true;
    }
    catch (QGLException& e) {
        std::cerr << "QuickGL Error: " << e.what() << std::endl;
        std::cerr << "QuickGL is aborting launch." << std::endl;
        success = false;
    }
}

bool Scene::launchSuccessful(void) {
    return success;
}

void Scene::run(void) {
    while (!glfwWindowShouldClose(getWindow())) {
        callback::bindInstance(this);
        refresh();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Scene::finalize(void) {
    glfwTerminate();
}


void Scene::setFrameBufferSizeCallback(GLFWframebuffersizefun callback) {
    glfwSetFramebufferSizeCallback(window, callback);
}

void Scene::setMouseButtonCallback(GLFWmousebuttonfun callback) {
    glfwSetMouseButtonCallback(window, callback);
}

void Scene::setCursorPositionCallback(GLFWcursorposfun callback) {
    glfwSetCursorPosCallback(window, callback);
}

void Scene::setScrollCallback(GLFWscrollfun callback) {
    glfwSetScrollCallback(window, callback);
}