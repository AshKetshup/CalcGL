#ifndef SPACE_H
#define SPACE_H

#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>
#include <string>

#include "functionReader.hpp"
#include "shader_m.h"

using namespace std;
using namespace glm;

class Surface {
	private:
		string source = "";
		vector<string> functions;
	public:
		Surface();
		Surface(const string fileName);
		Surface(const vector<string> expressions);
		vector<string> getExpressions();
		bool isIntercepted(vec3 camera, vec3 point);
		// void generate();
		void renderSurfaceGPU(Shader, Camera, const float, const float, vec3, const float = 10.f) const;
		void renderSurfaceCPU(Shader, Camera, const int, const int, vec3, float = 0.f, float = 0.f, const int threads = 1, const float camDist = 50.f) const;
		string toString();
};

class Ray {
	private:
		vec3 pointPos;
		vec3 uVecDir;
	public:
		Ray(vec3 point, vec3 dir);
		vec3 findPoint(float dist);
		vec3 rayMarch(Surface s, const float maxDistance, const float precision, const float stepSize);
		vec3 getPosition();
		vec3 getDirection();
};

class Plain {
	private:
		vec3 pointPos;
		vec3 uVecDirV;
		vec3 uVecDirH;
	public:
		Plain(vec3 point, vec3 vecV, vec3 vecH);
		Plain(Ray line, vec3 cameraUUp, vec3 cameraURight, float dist);
		vec3 findPoint(float h, float v);
		vec3 getPosition();
		vec3 getVDirection();
		vec3 getHDirection();
};

#endif