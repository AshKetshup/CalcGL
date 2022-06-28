#include "space.hpp"
#include "shunting-yard.h"
#include "builtin-features.inc"
#include "functionReader.hpp"

#include <thread>
#include <chrono>
#include <format>

// Surface Implementation
Surface::Surface() { }

Surface::Surface(const string fileName) {
	source = fileName;
	functions = reader::readFunctionFile(fileName);
	// generate();
}

Surface::Surface(const vector<string> expressions) {
	source = "";
	functions = expressions;
}

vector<string> Surface::getExpressions() {
	return functions;
}

bool Surface::isIntercepted(vec3 camera, vec3 point) {
	TokenMap vars;
	bool res = false;
	bool cameraSign, pointSign;
	int i = 0;

	for (string fun : functions) {
		if (i == 0) {
			vars["x"] = camera.x;
			vars["y"] = camera.y;
			vars["z"] = camera.z;
			
			try {
				packToken cam = calculator::calculate(fun.c_str(), vars);
				cameraSign = cam.asInt() > 0;
			} catch (const std::exception& ex) {
				cerr << ex.what() << "\n";
			}

			i++;
		}
		
		vars["x"] = point.x;
		vars["y"] = point.y;
		vars["z"] = point.z;

		try {
			packToken p = calculator::calculate(fun.c_str(), vars);
			pointSign = p.asInt() > 0;
		} catch (const std::exception& ex) {
			cerr << ex.what() << "\n";
		}

		res += !((cameraSign && pointSign) || (!cameraSign && !pointSign));
	}

	return res;
}

/*
in vec3  lightPos;
in vec3  lightColor;
in vec3  camPos;
in vec3  camDirFront;
in vec3  camDirUp;
in vec3  camDirRight;
in float camFOV;
*/
void Surface::renderSurfaceGPU(
	Shader s, 
	Camera c,
	const float width,
	const float height,
	vec3 objectColor,
	const float renderDistance
) const {
	vec4 lightColor = vec4(1.0f);
	vec3 lightPos = c.Position;

	float camFOV  = .5f;
	// float camFOV  = c.Zoom;

	// TODO: PASS VARIABLES TO THE GPU AS UNIFORMS
	s.use();
	
	s.setVec3("lightPos", lightPos);
	s.setVec3("lightColor", lightColor);

	s.setVec3("camPos", c.Position);
	s.setVec3("camDirF", c.Front);
	s.setVec3("camDirU", c.Up);
	s.setVec3("camDirR", c.Right);

	s.setFloat("camFOV", camFOV);
	s.setFloat("renderDistance", renderDistance);

	//s.setVec4("colorAttempt", 0.f, 0.f, 0.f, 1.f);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glUseProgram(0);
}

void Surface::renderSurfaceCPU(
	Shader s,
	Camera c,
	const float width,
	const float height,
	vec3 objectColor,
	const float renderDistance,
	const int threadAmmount
) const {
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 lightPos = c.Position;
	vec3 viewPos = c.Position;
	float camFOV = .5f;

	s.use();

	Plain plain = Plain(
		Ray(c.Position, c.Front),
		c.Up,
		c.Right,
		camFOV
	);

	//glm::mat

	auto f = [this](Camera &c, Plain plain, vector<float> &vertices, int jmin, int jmax, int imin, int imax, int myself) {
		for (int j = jmin; j >= -jmax; j--) {
			for (int i = imin; i <= imax; i++) {
				// printf("(%i, %i) @ %d\n", j, i, myself);

				// Vetor Diretor do raio para o RayMarching.
				vec3 rayMarchDir = plain.findPoint(i, j) - c.Position;

				Ray ray     = Ray(c.Position, rayMarchDir);
				vec3 result = ray.rayMarch(*this, 10.f, .01f, .1f);

				if (result[0] != NULL) {
					vertices.push_back(result.x);
					vertices.push_back(result.y);
					vertices.push_back(result.z);
					// printf(" -> (%f, %f, %f)", result[0], result[1], result[2]);
				}
			}
			printf("Line %d @ %d\n",j ,myself);
		}
	};
	

	vector<float> vertices;
	cparse_startup();

	// Multi thread code
	unsigned concurrency = thread::hardware_concurrency() - 2;
	printf("Using %d threads\n", concurrency);

	vector<thread> th;
	vector<vector<float>> vert = vector<vector<float>>(concurrency);

	auto start = chrono::system_clock::now();

	int step = width / concurrency;
	for (int i = 0; i < concurrency; i++) {
		th.push_back(
			thread(f, ref(c), plain, ref(vert[i]), height / 2, height / 2, -(width / 2) + (i * step), -(width / 2) + ((i+1) * step), i)
		);
		printf("Thread %d started, from %4d to %4d...\n", i, -(width / 2) + (i * step), -(width / 2) + ((i + 1) * step));
	}

	for (int i = 0; i < concurrency; i++) {
		th[i].join();
		//delete th[i];
		printf("Thread %d ended.\n", i);
	}

	for (int i = 0; i < concurrency; i++) {
		vertices.insert(vertices.end(), vert[i].begin(), vert[i].end());
	}

	// delete [] vert;
	auto end = chrono::system_clock::now();

	chrono::duration<double> elapsed = end - start;

	cout << "1 Frame calculated in: " << elapsed.count() << " seconds\n";


	// glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
}

string Surface::toString() {
	string iFunction = "";
	for (size_t i = 0; i < this->functions.size(); i++) {
		iFunction.append("(" + this->functions[i] + ")");

		if (i != (functions.size() - 1))
			iFunction.append(" * ");
	}

	return iFunction;
}
// End Implementation

// Ray Implementation
Ray::Ray(vec3 point, vec3 dir) {
	pointPos = point;
	uVecDir = normalize(dir);
}

vec3 Ray::findPoint(float dist) {
	// Equa��o vetorial reta:
	//	(x,y,z) = (x0,y0,z0) + dist(vx,vy,vz)
	//	(Ponto) = (Pos. Ini) + dist(vetor dir. norm.)

	return (pointPos + dist * uVecDir);
}

vec3 Ray::rayMarch(Surface s, const float maxDistance, const float precision, const float stepSize) {
	float stepAux = stepSize;
	float aux = 0;
	vec3 point = pointPos;
	
	while (!s.isIntercepted(pointPos, point) && aux < maxDistance) {
		aux += stepSize;
		point = findPoint(aux);
	}

	if (aux >= maxDistance)
		return vec3(NULL);

	while (true) {
		aux -= (stepAux /= 2);
		vec3 midPoint = findPoint(aux);
		if (stepAux < precision) {
			// Proximo o suficiente.
			point = midPoint;
			break;
		}

		if (!s.isIntercepted(pointPos, midPoint))
			aux += (stepAux /= 2);
	}

	return point;
}

vec3 Ray::getPosition() {
	return pointPos;
}

vec3 Ray::getDirection() {
	return uVecDir;
}
// End Implementation


// Plain Implementation
Plain::Plain(vec3 point, vec3 vecV, vec3 vecH) {
	pointPos = point;
	uVecDirV = vecV;
	uVecDirH = vecH;
}

Plain::Plain(Ray line, vec3 cameraUUp, vec3 cameraURight, float dist) {
	pointPos = line.findPoint(dist);
	uVecDirV = cameraUUp;
	uVecDirH = cameraURight;
}

vec3 Plain::findPoint(float h, float v) {
	// Equa��o vetorial plano:
	// 	(x,y,z) = (x0,y0,z0) + h(vx,vy,vz) + v(ux,uy,uz)
	// 	(Ponto) = (Pos. Ini) + (vetor dir) + (vetor dir

	return (pointPos + h * uVecDirH + v * uVecDirV);
}

vec3 Plain::getPosition() {
	return pointPos;
}

vec3 Plain::getVDirection() {
	return uVecDirV;
}

vec3 Plain::getHDirection() {
	return uVecDirH;
}
// End Implementation
