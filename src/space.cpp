#include "space.hpp"
#include "..\cparse\shunting-yard.h"
#include "..\cparse\builtin-features.inc"
#include "functionReader.hpp"

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
	for (string fun : functions) {
		vars["x"] = camera.x;
		vars["y"] = camera.y;
		vars["z"] = camera.z;
		bool cameraSign = calculator::calculate(fun.c_str(), vars).asInt() > 0;

		vars["x"] = point.x;
		vars["y"] = point.y;
		vars["z"] = point.z;
		bool pointSign = calculator::calculate(fun.c_str(), vars).asInt() > 0;
		
		res += !(cameraSign && pointSign);
	}

	return res;
}

void Surface::renderSurfaceGPU(Shader s, Camera c, const int SCR_WIDTH, const int SCR_HEIGHT, vec3 object_color, float rotx, float roty, const float camDist) const {
	return;
}

void Surface::renderSurfaceCPU(Shader s, Camera c, const int SCR_WIDTH, const int SCR_HEIGHT, vec3 objectColor, float rotx, float roty, const int threads, const float camDist) const {
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 lightPos = c.Position;
	vec3 viewPos = c.Position;

	s.use();

	Plain plain = Plain(
		Ray(c.Position, c.Front),
		c.Up,
		c.Right,
		camDist
	);
	

	vector<float> vertices;
	for (size_t i = -(SCR_WIDTH / 2); i < (SCR_WIDTH / 2); i++) {
		for (size_t j = -(SCR_HEIGHT / 2); j < (SCR_HEIGHT / 2); j++) {
			// Vetor Diretor do raio para o RayMarching.
			vec3 rayMarchDir = plain.findPoint(i, j) - c.Position;
			
			Ray ray = Ray(c.Position, rayMarchDir);
			vec3 result = ray.rayMarch(*this, 1000, .001f, .1f);
			
			vertices.push_back(result.x);
			vertices.push_back(result.y);
			vertices.push_back(result.z);
		}
	}

	unsigned int VBO, VAO;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glEnableVertexAttribArray(0);  // Vertex position
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer((GLuint) 0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	// glBindVertexArray(0);

	s.setVec3("objectColor", objectColor);
	s.setVec3("lamp.lightColor", lightColor);
	s.setVec3("lamp.lightPos", lightPos);
	s.setVec3("lamp.viewPos", viewPos);

	mat4 projection = perspective(radians(c.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	mat4 view = c.GetViewMatrix();

	s.setMat4("projection", projection);
	s.setMat4("view", view);

	mat4 model = mat4(1.0f);
	
	model = rotate(model, radians(0.f + rotx), vec3(1.f, 0.f, 0.f));
	model = rotate(model, radians(0.f + roty), vec3(0.f, 1.f, 0.f));
	model = scale(model, vec3(1.0f));
	s.setMat4("model", model);

	// glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_LINES, 0, vertices.size());

	glBindVertexArray(0);
}

string Surface::toString() {
	string out;
	for (string exp : functions)
		out.append(" ^ " + exp);
	
	return out;
}
// End Implementation


// Ray Implementation
Ray::Ray(vec3 point, vec3 dir) {
	pointPos = point;
	uVecDir = normalize(dir);
}

vec3 Ray::findPoint(float dist) {
	// Equação vetorial reta:
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
	pointPos = line.getPosition() + dist * line.getDirection();
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
