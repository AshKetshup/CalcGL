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

bool Surface::isIntercepted(vec3 point, bool currentSign) {
	return (eval(point) < 0.f) != currentSign;
}

void Surface::generate() {
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);
	glVertexAttrib1f(0, 0);

	glBindVertexArray(0);
}

void Surface::renderGPU(
	Shader s, 
	Camera c,
	const float width,
	const float height,
	vec3 objectColor,
	const float deltaTime,
	const float renderDistance
) const {
	vec4 lightColor = vec4(1.0f);
	vec3 lightPos = c.Position;

	float camFOV;
	{
		// Cálculo do FOV
		float MIN = 1.f, MAX = ZOOM;
		float fov_min = 0.5f, fov_max = 2.0f;
		camFOV = (fov_min - fov_max) / (MAX - MIN) * (c.Zoom - MIN) + fov_max;
	}

	glDisable(GL_DEPTH_TEST);

	s.use();
	
	s.setVec3("lightPos",     lightPos);
	s.setVec3("lightColor", lightColor);

	s.setVec3("camPos",   c.Position);
	s.setVec3("camDirFront", c.Front);
	s.setVec3("camDirUp",    c.Up   );
	s.setVec3("camDirRight", c.Right);
	
	s.setFloat("camFOV", camFOV);
	s.setFloat("renderDistance", renderDistance);

	s.setFloat("iTime", deltaTime);
	s.setVec2("iResolution", vec2(width, height));
	s.setVec3("objectColor", objectColor);
	
	glBindVertexArray(vaoHandle);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glUseProgram(0);
}

void Surface::renderCPU(
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

	Plain plain(
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

	// Multi thread code
	unsigned concurrency = thread::hardware_concurrency() - 2;
	printf("Using %ui threads\n" + concurrency);

	auto f = [this](Camera& c, Plain& plain, vector<vec4>& tex, vec2 dim, size_t yMin, size_t yMax, int myself) {
		for (size_t y = yMin; y < yMax; y++) {
			cout << "on Row " << y + 1 << "/" << dim.y << " by Thread " << myself << "\n";
			for (size_t x = 0; x < dim.x; x++) {
				vec2 uv = (vec2(x, y) * 2.f - dim) / dim;

				vec3 sPoint      = plain.findPoint(uv.x, uv.y);
				vec3 rayMarchDir = sPoint - c.Position;
				Ray  ray(c.Position, rayMarchDir);
				vec3 result   = ray.rayMarch(*this, 5.f, .01f, .1f);
				float distRes = distance(result, c.Position);

				if (distRes < 10.f) {
					tex[x * dim.x + y] = vec4(1.f);
					//cout << "Hit\n";
				} else {
					tex[x * dim.x + y] = vec4(0.f);
				}
			}
		}
	};

	vector<thread> th;

	int step = ceil(height / concurrency);
	for (int i = 0; i < concurrency; i++) {
		th.push_back(
			i != (concurrency - 1)
			? thread(f, ref(c), ref(plain), ref(tex), vec2(width, height), i * step, i * step + step, i)
			: thread(f, ref(c), ref(plain), ref(tex), vec2(width, height), i * step, height, i)
		);
		printf("Thread %d started, from %4d to %4d...\n", i, i * step, i * step + step);
	}

	for (int i = 0; i < concurrency; i++) {
		th[i].join();
		//delete th[i];
		printf("Thread %d ended.\n", i);
	}

	vector<float> data;
	for (vec4 el : tex) {
		data.push_back(el.x);
		data.push_back(el.y);
		data.push_back(el.z);
		data.push_back(el.w);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data.data());
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

SphereTracing::SphereTracing() {
	spheres.push_back( { vec4(0., 1., 6., 1.), vec4(0., 1., 0., 1.) } );
}

void SphereTracing::generate() {
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);
	glVertexAttrib1f(0, 0);

	glBindVertexArray(0);
}

void SphereTracing::renderGPU(
	Shader s,
	Camera c,
	const float width,
	const float height,
	vec3 objectColor,
	float deltaTime,
	const float renderDistance
) const {
	vec3 lightColor = vec3(1.f, 1.f, 1.f);
	vec3 lightPos = vec3(0.f, 5.f, -6.f) + vec3(2 * sinf(deltaTime), 0, 2 * cosf(deltaTime));

	float camFOV = c.Zoom;
	{
		// Cálculo do FOV
		float MIN = 1.f, MAX = ZOOM;
		float fov_min = 0.5f, fov_max = 2.0f;
		camFOV = (fov_min - fov_max) / (MAX - MIN) * (c.Zoom - MIN) + fov_max;
	}

	glDisable(GL_DEPTH_TEST);
	
	s.use();

	s.setVec3("lightPos", lightPos);
	s.setVec3("lightColor", lightColor);
	
	s.setVec3("camPos", c.Position);
	s.setVec3("camDirFront", c.Front);
	s.setVec3("camDirUp", c.Up);
	s.setVec3("camDirRight", c.Right);
	
	s.setFloat("camFOV", camFOV);
	s.setFloat("renderDistance", renderDistance);
	
	s.setVec2("iResolution", vec2(width, height));
	s.setFloat("iTime", deltaTime);
	s.setVec4("objectColor", vec4(objectColor, 1.f));

	// s.setInt("nspheres", spheres.size());
	// unsigned int block_index = glGetUniformBlockIndex(s.getID(), "ubospheres");
	// glUniformBlockBinding(s.getID(), block_index, 0);

	glBindVertexArray(vaoHandle);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	// glBindVertexArray(0);

	glUseProgram(0);
}

// Ray Implementation
Ray::Ray(vec3 point, vec3 dir) {
	pointPos = point;
	uVecDir = normalize(dir);
}

vec3 Ray::findPoint(float dist) {
	//  Equação vetorial reta:
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

	while (t < maxDistance && !s.isIntercepted(rightPoint, currentSign)) {
		leftPoint = findPoint(t);
		t += stepSize;
		rightPoint = findPoint(t);
	}

	if (t > maxDistance)
		return findPoint(t + 1.f);

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
	// Equação vetorial plano:
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
