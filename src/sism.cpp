#include "sism.hpp"


using namespace calcgl;
using namespace filesystem;


#ifdef _WIN32
string slash = "\\";
#else
string slash = "/";
#endif

namespace callback {
    calcgl::CalcGL* instance = nullptr;

    void bindInstance(calcgl::CalcGL* i) {
        instance = i;
    }

    calcgl::CalcGL* getInstance(void) {
        return instance;
    }

    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void mousebtn_callback(GLFWwindow* window, int button, int action, int mods) {
        bool lbutton_down = false;
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (GLFW_PRESS == action)
                lbutton_down = true;
            else if (GLFW_RELEASE == action)
                lbutton_down = false;
        }

        if (lbutton_down) {
            double x, y;
            glfwGetCursorPos(getInstance()->getWindow(), &x, &y);
        }
    }

    void mouse_callback(GLFWwindow * window, double xpos, double ypos) {
        if (getInstance()->getMouseData().first) {
            getInstance()->setMouseData((float) xpos, (float) ypos, false);
            debugs("\n");
        }

        float xoffset = xpos - getInstance()->getMouseData().lastX;
        float yoffset = getInstance()->getMouseData().lastY - ypos;     // reversed since y-coordinates go from bottom to top
        getInstance()->setMouseData(xpos, ypos, false);
        getInstance()->getCamera().ProcessMouseMovement(xoffset, yoffset);
    }

    void scroll_callback(GLFWwindow * window, double xoffset, double yoffset) {
        getInstance()->getCamera().ProcessMouseScroll(yoffset);
    }
}

void calcgl::writeBlock(TextRenderer tr, float x, float y, float scale, vector<string> block) {
    for (size_t i = 0; i < block.size(); i++)
        tr.RenderText(block[i], x, y + ((float)i) * scale * (tr.getFontSize() + 5.f), scale);
}

void calcgl::writeInstructions(TextRenderer tr, float x, float y, float scale) {
    writeBlock(tr, x, y, scale, vector<string>({
        "               [O] Open File           ",
        "[WASD,Arrows,2468] Move camera         ",
        "               [C] Change surface color",
        "               [X] Reset surface color ",
        "           [Mouse] Rotate camera       ",
        "     [Mouse wheel] Zoom                ",
        "          [ESC, Q] Exit                "
        })
    );
}

void calcgl::writeAuthors(TextRenderer tr, float x, float y, float scale) {
    writeText(tr, "  Diogo Simoes", x, y + scale * (tr.getFontSize() + 5.f), scale);
}

void calcgl::writeText(TextRenderer tr, string text, float x, float y, float scale) {
    tr.RenderText(text, x, y, scale);
}

const char* CalcGLException::what(void) const throw () { 
    return message.c_str(); 
}

// TODO: TERMINAR ISTO
bool CalcGL::initialize_glfw(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
        return false;

    glfwMakeContextCurrent(window);

    setFrameBufferSizeCallback(callback::framebuffer_size_callback);
    setMouseButtonCallback(callback::mousebtn_callback);
    setCursorPositionCallback(callback::mouse_callback);
    setScrollCallback(callback::scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    return true;
}

int CalcGL::initialize_glad(void) {
    return gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
}

action CalcGL::processInput(void) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.MovementSpeed = 6.f;
    else
        camera.MovementSpeed = 3.f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);


    /*
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
        molrotx += 1.f; // TODO: Change for camera rotation
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        molrotx -= 1.f; // TODO: Change for camera rotation
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        molroty += 1.f; // TODO: Change for camera rotation
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        molroty -= 1.f; // TODO: Change for camera rotation
    */
    
    // TODO Might need
    //if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
    //    if (!w_was_pressed)
    //        rmode = (rmode == SurfaceGPU) ? SurfaceCPU : SurfaceGPU;
    //    w_was_pressed = true;
    //} else {
    //    w_was_pressed = false;
    //}

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        return action::CAMERA_RESET;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        switchModeView(false);
        
        char* newfile = openFunctionFileDialog();
        if (newfile != NULL) {
            if (isFunction(newfile)) {
                fname = string(newfile);
                free(newfile);
                switchModeView(true);
                surfColor = SURF_DEFAULT_COLOR;
                return action::OPEN_FILE;
            } 
        
            cout << "This is not a valid '.fun' or a '.function' file." << endl;
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "This is not a valid PDB file.");
            switchModeView(true);
            free(newfile);
            return action::NO_ACTION;
        }

        switchModeView(true);
        return action::NO_ACTION;
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        //switchModeView(false);
        //osdialog_color color;
        //int ret = CalcGL::selectColorDialog(&color);

        // debugs("Color: (%d, %d, %d, %d); ret = %d\n", (int)color.r, (int)color.g, (int)color.b, (int)color.a, ret);
        //if (ret == 0) {
        //    surfColor = vec3((int) color.r / 255.f, (int) color.g / 255.f, (int) color.b / 255.f);
        //    switchModeView(true);
        //    return action::CHANGE_COLOR;
        //}

        //switchModeView(true);
        //return action::NO_ACTION;
        switchModeView(true);

        return action::CHANGE_COLOR;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        surfColor = SURF_DEFAULT_COLOR;

    return action::NO_ACTION;
}

Camera& CalcGL::getCamera(void) {
    return camera;
}

calcgl::mouse_data CalcGL::getMouseData(void) {
    return mouse;
}

void CalcGL::setMouseData(float x, float y, bool first) {
    mouse = { x, y, first };
}

bool CalcGL::getModeView(void) {
    return modeView;
}

void CalcGL::switchModeView(bool mode) {
    modeView = mode;
    // glfwSetInputMode(window, GLFW_CURSOR, mode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_HIDDEN);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

char* CalcGL::openFunctionFileDialog(void) {
    return osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
}

int CalcGL::selectColorDialog(osdialog_color* color) {
    return osdialog_color_picker(color, 1);
}

bool CalcGL::isFunction(char* fname) {
    return path(fname).extension().compare(".function") == 0;
}

void CalcGL::setFrameBufferSizeCallback(GLFWframebuffersizefun callback) {
    glfwSetFramebufferSizeCallback(window, callback);
}

void CalcGL::setMouseButtonCallback(GLFWmousebuttonfun callback) {
    glfwSetMouseButtonCallback(window, callback);
}

void CalcGL::setCursorPositionCallback(GLFWcursorposfun callback) {
    glfwSetCursorPosCallback(window, callback);
}

void CalcGL::setScrollCallback(GLFWscrollfun callback) {
    glfwSetScrollCallback(window, callback);
}

bool CalcGL::launchSuccessful(void) {
    return success;
}

string CalcGL::getAppDir(void) {
    return appDir;
}

string CalcGL::getAppName(void) {
    return appName;
}

string CalcGL::getApp(void) {
    return appDir + slash + appName;
}

string CalcGL::getShaderDir(void) {
    return shaderDir;
}

string CalcGL::getFontDir(void) {
    return fontDir;
}

string CalcGL::getFont(void) {
    return fontDir + slash + fontName;
}

GLFWwindow* CalcGL::getWindow(void) {
    return window;
}

Shader CalcGL::getRayMarchCPUShader(void) {
    return shaderRayMarchCPU;
}

Shader CalcGL::getRayMarchGPUShader(void) {
    return shaderRayMarchGPU;
}

Shader CalcGL::getSphereTracingGPUShader(void) {
    return shaderSphereTracingGPU;
}

Shader CalcGL::getFontShader(void) {
    return textRender.getShader();
}

TextRenderer CalcGL::getTextRenderer(void) {
    return textRender;
}

string CalcGL::getFPS(void) {
    return appFPS;
}

void CalcGL::setFPS(unsigned int fps) {
    appFPS = to_string(fps) + " FPS";
}

void CalcGL::refresh(void) {
    callback::bindInstance(this);

    //glDisable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);

    glDepthMask(GL_TRUE);
    glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (logo.isAvailable() && fname.empty())
        logo.render(scr_width, scr_height);


    // Process Input Handler
    switch (processInput()) {
        case action::OPEN_FILE:
            if (rmode == SurfaceGPU) {
                try {
                    surface = Surface(fname.data());
                    // debugs("\n%s\n", surface.toString().c_str());
                    surface.generate();

                    shaderRayMarchGPU = Shader((shaderDir + slash + RAYMARCH_VS).c_str(), (shaderDir + slash + RAYMARCH_FS).c_str(), surface.getExpressions());
                    if (!shaderRayMarchGPU.wasSuccessful())
                        throw CalcGLException(
                            "SphereTrace GPU: " + shaderRayMarchGPU.getReport() + "\n" + shaderRayMarchGPU.getVertexShaderPath() + "\n" + shaderRayMarchGPU.getGeometryShaderPath()
                        );

                } catch (const CalcGLException& e) {
                    debugs("[ERROR]\n");
                    cout << "Error: " << e.what() << endl;
                    cout << "Abort launch!" << endl;
                    success = false;
                }
            }

            if (rmode == SurfaceCPU)
                getRayMarchCPUShader().recompileWithFunctions(surface.getExpressions());

            renderIF = true;

            if (rmode == STraceGPU)
                renderIF = false;

            break;

        case action::CAMERA_RESET:
            camera.Position = vec3(0.f, 1.f, 0.f);
            // molrotx = 0.f; // TODO: change for camera rotation
            // molroty = 0.f; // TODO: change for camera rotation
            debugs("Camera position reseted.\n");
            break;

        case action::CHANGE_COLOR:
            try {
                sTracing = SphereTracing();
                sTracing.generate();

                renderIF = true;
            
                shaderSphereTracingGPU = Shader((shaderDir + slash + SPHERETRACING_VS).c_str(), (shaderDir + slash + SPHERETRACING_FS).c_str());
                if (!shaderSphereTracingGPU.wasSuccessful())
                    throw CalcGLException(
                        "SphereTrace GPU: " + shaderSphereTracingGPU.getReport() + "\n" + shaderSphereTracingGPU.getVertexShaderPath() + "\n" + shaderSphereTracingGPU.getGeometryShaderPath()
                    );
            } catch (const CalcGLException& e) {
                debugs("[ERROR]\n");
                cout << "Error: " << e.what() << endl;
                cout << "Abort launch!" << endl;
                success = false;
            }

            break;

        default:
            break;
    }
 
    // Switch between viewing modes
    if (renderIF) 
        switch (rmode) {
            case STraceGPU:
                sTracing.renderGPU(getSphereTracingGPUShader(), getCamera(), scr_width, scr_height, surfColor, bgColor, glfwGetTime());
                break;
            case SurfaceCPU:
                surface.renderCPU(getRayMarchCPUShader(), getCamera(), scr_width, scr_height, surfColor, threadAmnt);
                break;
            case SurfaceGPU:
            default:
                surface.renderGPU(getRayMarchGPUShader(), getCamera(), scr_width, scr_height, surfColor, bgColor, glfwGetTime(), 50.);
        }


    // TODO: FIND BUG HERE
    writeInstructions(getTextRenderer(), 10.f, 10.f, 0.6f);
    writeAuthors(getTextRenderer(), 1470.f, scr_height - 2.f * getTextRenderer().getFontSize(), 0.75f);
    writeText(getTextRenderer(), getFPS(), 10.f, scr_height - (2 * getTextRenderer().getFontSize()), 0.8f);
    writeText(getTextRenderer(), (!fname.empty() ? filesystem::path(fname).filename().string() : "No file opened"), 10.f, scr_height - getTextRenderer().getFontSize(), 0.8f);


    glfwSwapBuffers(window);
    glfwPollEvents();
}

void CalcGL::main(void) {

    double prevTime = 0.0;
    double crntTime = 0.0;
    double timeDiff;

    unsigned int counter = 0;

    while (!glfwWindowShouldClose(getWindow())) {
        crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;

        if (timeDiff >= 1.0 / 30.0) { 
            setFPS((1.0/timeDiff) * counter);

            prevTime = crntTime;
            counter = 0;
        }

        refresh();
    }
}

void CalcGL::terminate(void) {
    glfwTerminate();
}

CalcGL::CalcGL(void) { }

CalcGL::CalcGL(const unsigned int width, const unsigned int height, const char* rMode, const unsigned int threads) {
    try {
        debugs("Lauching CalcGL...\n");

        debugs("\tSetting global variables... ");
        if (strcmp(rMode, "CPU") == 0) {
            rmode = SurfaceCPU;
            threadAmnt = threads;
            debugs("{ Surface::CPU with %u threads }", threadAmnt);
        } else if (strcmp(rMode, "GPU") == 0) {
            rmode = SurfaceGPU;
            debugs("{ Surface::GPU }");
        } else if (strcmp(rMode, "SPHERE") == 0) {
            rmode = STraceGPU;
            debugs("{ SphereTracing::GPU }");
        }

        debugs("[OK]\n\tSetting relevant directories... ");
        path appPath(filesys::getAppPath());

        appDir = appPath.parent_path().string();
        appName = appPath.filename().string();
        shaderDir = appDir + slash + "shaders";
        fontDir = appDir + slash + "fonts";
        resDir = appDir + slash + "res";
        fontName = "UbuntuMono-R.ttf";
        logoName = "cgl" + logo.getLogoV() + ".bmp";

        debugs("[OK]\n\tInitializing GLFW and GLAD... ");
        scr_width = width;
        scr_height = height;
        setMouseData(scr_width / 2.0f, scr_height / 2.0f, true);
        
        if (!initialize_glfw(scr_width, scr_height, "CalcGL - Implicit Function Renderer"))
            throw CalcGLException("Failed to create the window");
        if (!initialize_glad())
            throw CalcGLException("Failed to initialize GLAD");

        debugs("[OK]\n\tAuto-checking shaders... ");
        debugs("(corrected %d shaders) ", autoCorrectShaders(shaderDir));

        debugs("[OK]\n\tLoading shaders... ");
        shaderRayMarchCPU = Shader((shaderDir + slash + SURF_VS).c_str(), (shaderDir + slash + SURF_FS).c_str());
        if (!shaderRayMarchCPU.wasSuccessful())
            throw CalcGLException(
                "RayMarch CPU: " + shaderRayMarchCPU.getReport() + "\n" + shaderRayMarchCPU.getVertexShaderPath() + "\n" + shaderRayMarchCPU.getGeometryShaderPath()
            );

        shaderRayMarchGPU = Shader((shaderDir + slash + RAYMARCH_VS).c_str(), (shaderDir + slash + RAYMARCH_FS).c_str());
        if (!shaderRayMarchGPU.wasSuccessful())
            throw CalcGLException(
                "RayMarch GPU: " + shaderRayMarchGPU.getReport() + "\n" + shaderRayMarchGPU.getVertexShaderPath() + "\n" + shaderRayMarchGPU.getGeometryShaderPath()
            );

        shaderSphereTracingGPU = Shader((shaderDir + slash + SPHERETRACING_VS).c_str(), (shaderDir + slash + SPHERETRACING_FS).c_str());
        if (!shaderSphereTracingGPU.wasSuccessful())
            throw CalcGLException(
                "SphereTrace GPU: " + shaderSphereTracingGPU.getReport() + "\n" + shaderSphereTracingGPU.getVertexShaderPath() + "\n" + shaderSphereTracingGPU.getGeometryShaderPath()
            );
        
        shaderFont = Shader((shaderDir + slash + FONT_VS).c_str(), (shaderDir + slash + FONT_FS).c_str());
        if (!shaderFont.wasSuccessful())
            throw CalcGLException("Font: " + shaderFont.getReport());
        
        shaderLogo = Shader((shaderDir + slash + LOGO_VS).c_str(), (shaderDir + slash + LOGO_FS).c_str());
        if (!shaderLogo.wasSuccessful())
            throw CalcGLException("Logo: " + shaderLogo.getReport());

        camera = Camera(vec3(0.0f, 1.0f, 0.0f));

        debugs("[OK]\n\tLoading text renderer... ");
        textRender = TextRenderer(scr_width, scr_height, shaderFont);
        textRender.Load(getFont(), 24);
        
        debugs("[OK]\n\tLoading logo... ");
        logo = Logo((resDir + slash + logoName).c_str(), shaderLogo);
        if (!logo.wasSuccessful())
            throw CalcGLException("Failed to load logo");

        debugs("[OK]\n");
        debugs("Done.\n\n");

        success = true;

    } catch (const CalcGLException& e) {
        debugs("[ERROR]\n");
        cout << "Error: " << e.what() << endl;
        cout << "Abort launch!" << endl;
        success = false;
    }
}

CalcGL::~CalcGL(void) {
    terminate();
}
