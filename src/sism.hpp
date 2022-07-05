#ifndef SISM_H
#define SISM_H

// #include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>

#include "shader_m.h"
#include "text.h"

//#include "functionReader.hpp"
//#include "camera.hpp"
#include "space.hpp"
#include "logo.hpp"
#include "debug.hpp"
#include "glslCode.hpp"
#include "filesys.hpp"

//#include <string>
#include <filesystem>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "osdialog.h"
// #include "quickgl.hpp"
// #define QGL_GLAD

namespace calcgl {
	using namespace std;
	using namespace glm;
	// using namespace qgl;

	typedef enum {
		SurfaceCPU,
		SurfaceGPU,
		STraceGPU,
		STraceCPU
	} render_mode;

	typedef enum {
		NO_ACTION,
		CAMERA_RESET,
		OPEN_FILE,
		CHANGE_COLOR
	} action;

	typedef struct {
		float lastX;
		float lastY;
		bool first;
	} mouse_data;

	void writeBlock(TextRenderer tr, float x, float y, float scale, vector<string> block);
	void writeInstructions(TextRenderer, float x, float y, float);
	void writeAuthors(TextRenderer, float x, float y, float);
	void writeText(TextRenderer, string, float x, float y, float);

	class CalcGLException: public exception {
		public:
			CalcGLException(const string& msg) : message(msg) {};
			virtual const char* what() const throw();
		private:
			const string message;
	};

	class CalcGL {
		private:
			static constexpr vec3 SURF_DEFAULT_COLOR = vec3(50.f/255.f, 140.f/255.f, 235.f/ 255.f);
			static constexpr vec3 BG_DEFAULT_COLOR = vec3(46.f / 255.f, 68.f / 255.f, 64.f / 255.f);
			bool success = false;
			bool renderIF = false;

			string appFPS;

			string appDir;
			string appName;

			string shaderDir;
			Shader shaderFont;
			Shader shaderRayMarchCPU;
			Shader shaderRayMarchGPU;
			Shader shaderSphereTracingGPU;
			Shader shaderLogo;

			vec3 surfColor = SURF_DEFAULT_COLOR;
			vec3 bgColor   = BG_DEFAULT_COLOR;

			string fontDir;
			string fontName;
			TextRenderer textRender;

			string resDir;
			string logoName;
			Logo logo;

			render_mode rmode = SurfaceGPU;

			GLFWwindow* window;
			unsigned int scr_width;
			unsigned int scr_height;

			// Timing 
			float deltaTime = 0.0f;
			float lastFrame = 0.0f;
			float currentFrame;
			
			// No Rotation of the Graphic

			// Camera
			Camera camera;
			mouse_data mouse; // = {0.f, 0.f, true};
			bool modeView = false;
			bool w_was_pressed = false;

			string fname;
			Surface surface;
			SphereTracing sTracing;

			bool initialize_glfw(int, int, const char*);
			int initialize_glad(void);
			action processInput(void);

		public:
			void switchModeView(bool);

			static char* openFunctionFileDialog(void);
			static int selectColorDialog(osdialog_color*);
			static bool isFunction(char*);

			void setMouseData(float, float, bool);
			void setFrameBufferSizeCallback(GLFWframebuffersizefun);
			void setMouseButtonCallback(GLFWmousebuttonfun);
			void setCursorPositionCallback(GLFWcursorposfun);
			void setScrollCallback(GLFWscrollfun);
			void setFPS(unsigned int);

			bool launchSuccessful(void);
			
			GLFWwindow* getWindow(void);
			Camera& getCamera(void);
			mouse_data getMouseData(void);
			bool getModeView(void);
			string getAppDir(void);
			string getAppName(void);
			string getApp(void);
			string getShaderDir(void);
			string getFontDir(void);
			string getFont(void);
			Shader getFontShader(void);
			Shader getRayMarchCPUShader(void);
			Shader getRayMarchGPUShader(void);
			Shader getSphereTracingGPUShader(void);
			TextRenderer getTextRenderer(void);
			string getFPS(void);

			void refresh(void);
			void terminate(void);
			void main(void);

			CalcGL(void);
			CalcGL(const unsigned int, const unsigned int);
			~CalcGL(void);
	};
}

#endif
