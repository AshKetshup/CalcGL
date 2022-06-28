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

bool Surface::even(int n) {
	return n % 2 == 0;
}

float Surface::eval(vec3 point) {
	TokenMap vars;

	int count   = 0;
	float prod  = 1.f;
	float image = 0.f;
	
	for (string fun : functions) {
			vars["x"] = point.x;
			vars["y"] = point.y;
			vars["z"] = point.z;
			
			try {
				packToken calc = calculator::calculate(fun.c_str(), vars);
				
				image = (float) calc.asDouble();
				prod *= image;
				if (image < 0.f) 
					count++;
			} catch (const std::exception& ex) {
				cerr << ex.what() << "\n";
			}
	}

	return prod * (even(count) ? -1.f : 1.f);
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
	auto start = chrono::system_clock::now();

	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 lightPos = c.Position;
	vec3 viewPos = c.Position;
	float camFOV = .5f;

	s.use();
	cparse_startup();

	Plain plain = Plain(
		Ray(c.Position, c.Front),
		c.Up,
		c.Right,
		camFOV
	);

	// Generate a 2DTexture 
	unsigned int texture;
	glGenTextures(1, &texture);

	// Bind created texture
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	int texSize = width * height;
	vector<vec4> tex(texSize);

	//auto f = [this](Camera &c, Plain plain, vector<float> &vertices, int jmin, int jmax, int imin, int imax, int myself) {
	//	for (int j = jmin; j >= -jmax; j--) {
	//		for (int i = imin; i <= imax; i++) {
	//			// printf("(%i, %i) @ %d\n", j, i, myself);

	//			// Vetor Diretor do raio para o RayMarching.
	//			vec3 rayMarchDir = plain.findPoint(i, j) - c.Position;

	//			Ray   ray     = Ray(c.Position, rayMarchDir);
	//			vec3  result  = ray.rayMarch(*this, 10.f, .01f, .1f);
	//			float distRes = glm::distance(result, c.Position);

	//			if (distRes < 10.f) {
	//				return vec4(1.f);
	//			} else {
	//				return vec4(0.f);
	//			}
	//		}
	//		printf("Line %d @ %d\n", j, myself);
	//	}
	//};

	// Multi thread code
	unsigned concurrency = thread::hardware_concurrency() - 2;
	printf("Using 1 threads\n", concurrency);

	/* SINGLE THREAD */
	for (size_t x = 0; x < width; x++) {
		cout << "on Column " << x+1 << "/" << width << "\n";
		for (size_t y = 0; y < height; y++) {
			vec2 uv = (vec2(x, y) / vec2(width, height)) - 0.5f;

			vec3 rayMarchDir = plain.findPoint(uv.x, uv.y) - c.Position;

			Ray   ray     = Ray(c.Position, rayMarchDir);
			vec3  result  = ray.rayMarch(*this, 10.f, .01f, .1f);
			float distRes = glm::distance(result, c.Position);


			if (distRes < 10.f) {
				tex[x * width + y] = vec4(1.f);
				cout << "Hit\n";
			}
			else {
				tex[x * width + y] = vec4(0.f);
				//cout << "Void\n";
			}
		}
	}

	//vector<thread> th;

	//int step = width / concurrency;
	//for (int i = 0; i < concurrency; i++) {
	//	th.push_back(
	//		thread(f, ref(c), plain, ref(tex[i*width]), height / 2, height / 2, -(width / 2) + (i * step), -(width / 2) + ((i+1) * step), i)
	//	);
	//	printf("Thread %d started, from %4d to %4d...\n", i, -(width / 2) + (i * step), -(width / 2) + ((i + 1) * step));
	//}

	//for (int i = 0; i < concurrency; i++) {
	//	th[i].join();
	//	//delete th[i];
	//	printf("Thread %d ended.\n", i);
	//}


	//for (int i = 0; i < concurrency; i++) {
	//	vertices.insert(vertices.end(), vert[i].begin(), vert[i].end());
	//}

	// delete [] vert;

	
	vector<float> data;
	for (vec3 el : tex) {
		data.push_back(el.x);
		data.push_back(el.y);
		data.push_back(el.z);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, data.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	tex.~vector();
	data.~vector();

	glUniform1i(glGetUniformLocation(s.ID, "texture"), texture);
	s.setVec2("iResolution", vec2(width, height));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

	auto end = chrono::system_clock::now();
	chrono::duration<double> elapsed = end - start;

	cout << "1 Frame calculated in: " << elapsed.count() << " seconds\n";
	
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

vec3 Ray::bisection(Surface s, vec3 lPoint, vec3 rPoint, float precision) {
	while (true) {
		vec3 mid = (lPoint + rPoint) / 2.f;
		float signedDist = s.eval(mid);

		if (abs(signedDist) < precision)
			return mid;

		if (signedDist < 0.f)
			lPoint = mid;
		else
			rPoint = mid;
	}
}

vec3 Ray::rayMarch(Surface s, const float maxDistance, const float precision, const float stepSize) {
	float t = 0.f;
	vec3 marchingDir = normalize(getDirection());
	vec3 leftPoint   = getPosition();
	vec3 rightPoint  = vec3(0.f);
	bool intercept   = false;
	bool currentSign = (s.eval(leftPoint) < 0.f);

	while (t < maxDistance && !intercept) {
		leftPoint = findPoint(t);
		t += stepSize;
		rightPoint = findPoint(t);

		intercept = (s.eval(rightPoint) < 0.f) != currentSign;
	}

	if (t >= maxDistance)
		return findPoint(13.f);

	vec3 pInteresected = bisection(s, leftPoint, rightPoint, precision);
	return pInteresected;
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
