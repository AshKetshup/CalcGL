#include "space.hpp"
#include "shunting-yard.h"
#include "builtin-features.inc"
#include "functionReader.hpp"

#include <thread>

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

void Surface::renderSurfaceGPU(Shader s, Camera c, const int SCR_WIDTH, const int SCR_HEIGHT, vec3 object_color, float rotx, float roty, const float camDist) const {
	return;
}

void Surface::renderSurfaceCPU(Shader s, Camera c, const int SCR_WIDTH, const int SCR_HEIGHT, vec3 objectColor, float rotx, float roty, const int threads, const float camDist) const {
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 lightPos = c.Position;
	vec3 viewPos = c.Position;
	cparse_startup();


	s.use();

	Plain plain = Plain(
		Ray(c.Position, c.Front),
		c.Up,
		c.Right,
		camDist
	);

	auto f = [this](Camera &c, Plain plain, vector<float> &vertices, int jmin, int jmax, int imin, int imax, int myself) {
		for (int j = jmin; j > -jmax; j--) {
			for (int i = imin; i < imax; i++) {
				printf("(%i, %i) @ %d\n", j, i, myself);

				// Vetor Diretor do raio para o RayMarching.
				vec3 rayMarchDir = plain.findPoint(i, j) - c.Position;

				Ray ray = Ray(c.Position, rayMarchDir);
				vec3 result = ray.rayMarch(*this, 10.f, .01f, .1f);

				if (result[0] != NULL) {
					vertices.push_back(result.x);
					vertices.push_back(result.y);
					vertices.push_back(result.z);
					printf(" -> (%f, %f, %f)", result[0], result[1], result[2]);
				}
			}
			printf("\n");
		}
	};
	

	vector<float> vertices;
	cparse_startup();

	// Multi thread code
	unsigned concurrency = thread::hardware_concurrency() / 2;
	printf("Using %d threads\n", concurrency);

	vector<thread> th;
	vector<float> *vert = new vector<float>[concurrency];

	int step = SCR_WIDTH / concurrency;
	for (int i = 0; i < concurrency; i++) {
		th.push_back(thread(f, std::ref(c), plain, std::ref(vert[i]), SCR_HEIGHT / 2, SCR_HEIGHT / 2, -(SCR_WIDTH / 2) + (i * step), -(SCR_WIDTH / 2) + ((i+1) * step), i));
		printf("Thread %d started, from %4d to %4d...\n", i, -(SCR_WIDTH / 2) + (i * step), -(SCR_WIDTH / 2) + ((i + 1) * step));
	}

	for (int i = 0; i < concurrency; i++) {
		th[i].join();
		//delete th[i];
		printf("Thread %d ended.\n", i);
	}

	for (int i = 0; i < concurrency; i++) {
		vertices.insert(vertices.end(), vert[i].begin(), vert[i].end());
	}

	delete [] vert;



	/* // Single thread code
	for (int j = (SCR_HEIGHT / 2); j > -(SCR_HEIGHT / 2); j--) {
		for (int i = -(SCR_WIDTH / 2); i < (SCR_WIDTH / 2); i++) {
			printf("(%i, %i)\n", j, i);

			// Vetor Diretor do raio para o RayMarching.
			vec3 rayMarchDir = plain.findPoint(i, j) - c.Position;
			
			Ray ray = Ray(c.Position, rayMarchDir);
			vec3 result = ray.rayMarch(*this, 10.f, .01f, .1f);
			
			if (result[0] != NULL) {
				vertices.push_back(result.x);
				vertices.push_back(result.y);
				vertices.push_back(result.z);
				printf("(%f, %f, %f)", result[0], result[1], result[2]);
			}
		}
	}
	*/

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
	size_t x = 0;
	for (string exp : functions) {
		if (x == 0) {
			out.append(exp);
			x++;
		} else {
			out.append(" ^ " + exp);
		}
	}

	return out;
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
	pointPos = line.getPosition() + dist * line.getDirection();
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
